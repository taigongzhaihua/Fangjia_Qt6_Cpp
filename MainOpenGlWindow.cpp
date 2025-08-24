#include "MainOpenGlWindow.h"

#include "NavViewModel.h"
#include "RenderData.hpp"
#include "ThemeManager.h"
#include "UiNav.h"
#include "UiPage.h"
#include "UiTopBar.h"
#ifdef Q_OS_WIN
#include "WinWindowChrome.h"
#endif
#include <qsystemdetection.h>

#include <algorithm>
#include <gl/GL.h>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qevent.h>
#include <qnamespace.h>
#include <qobjectdefs.h>
#include <qopenglwindow.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringliteral.h>
#include <qtimer.h>
#include <utility>
#include <qlogging.h>
#include "TabViewModel.h"
#include "UiTabView.h"
namespace {
	struct PaletteBtn { QColor btnBg, btnBgHover, btnBgPressed, iconColor; };
	PaletteBtn paletteBtnForTheme(const MainOpenGlWindow::Theme t) {
		using T = MainOpenGlWindow::Theme;
		if (t == T::Dark) {
			return { .btnBg = QColor(52,63,76,120), .btnBgHover = QColor(66, 78,92,200), .btnBgPressed = QColor(58, 70, 84,220), .iconColor =
				QColor(255, 255,255,255) };
		}
		return { .btnBg = QColor(240,243,247,200), .btnBgHover = QColor(232, 237,242,220), .btnBgPressed = QColor(225, 230,236,230), .iconColor =
			QColor(60, 64,72,255) };
	}

	Ui::NavPalette paletteNavForTheme(const MainOpenGlWindow::Theme t) {
		if (t == MainOpenGlWindow::Theme::Dark) {
			return Ui::NavPalette{
				.railBg = QColor(21, 28, 36, 0),
				.itemHover = QColor(255,255,255,18),
				.itemPressed = QColor(255,255,255,30),
				.itemSelected = QColor(255,255,255,36),
				.iconColor = QColor(242,245,255,198),
				.labelColor = QColor(255,255,255,255),
				.indicator = QColor(0,122,255,200)
			};
		}
		return Ui::NavPalette{
			.railBg = QColor(246,248,250,0),
			.itemHover = QColor(0,0,0,14),
			.itemPressed = QColor(0,0,0,26),
			.itemSelected = QColor(0,0,0,32),
			.iconColor = QColor(70,76,84,255),
			.labelColor = QColor(70,76,84,255),
			.indicator = QColor(0,102,204,220)
		};
	}

	inline MainOpenGlWindow::Theme schemeToTheme(const Qt::ColorScheme s) {
		return s == Qt::ColorScheme::Dark ? MainOpenGlWindow::Theme::Dark : MainOpenGlWindow::Theme::Light;
	}
}
MainOpenGlWindow::MainOpenGlWindow(const UpdateBehavior updateBehavior)
	: QOpenGLWindow(updateBehavior)
{
	connect(&m_animTimer, &QTimer::timeout, this, &MainOpenGlWindow::onAnimTick);
	m_animTimer.setTimerType(Qt::PreciseTimer);
	m_animTimer.setInterval(16);
	m_animClock.start();

	m_navCollapsedW = 48;
	m_navExpandedW = 200;

	m_topBar.setCornerRadius(8.0f);
}

MainOpenGlWindow::~MainOpenGlWindow()
{
#ifdef Q_OS_WIN
	// 先卸载原生事件过滤器，避免窗口销毁期间回调到已失效对象
	if (m_winChrome) {
		m_winChrome->detach();
		m_winChrome = nullptr; // attach 时也会在窗口 destroyed 中 delete
	}
#endif

	makeCurrent();
	m_iconLoader.releaseAll(this);
	m_renderer.releaseGL();
	doneCurrent();
}

void MainOpenGlWindow::initializeGL()
{
	initializeOpenGLFunctions();
	glEnable(GL_BLEND);
	glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

	m_renderer.initializeGL(this);

	// Windows: 安装自定义 WindowChrome，使顶部成为可拖拽区域、保留系统边框/阴影
#ifdef Q_OS_WIN
	if (!m_winChrome) {
		// 顶部认为 56 逻辑像素作为“标题栏拖动”高度（与 UiTopBar 的 margin + 按钮大小接近）
		constexpr int dragHeight = 56;
		m_winChrome = WinWindowChrome::attach(this, dragHeight, [this]() {
			// 返回不允许拖拽的区域（例如导航栏、右上角按钮），均为逻辑像素坐标
			QVector<QRect> out;
			out.push_back(this->navBounds());
			out.push_back(this->topBarThemeRect());
			out.push_back(this->topBarFollowRect());
			out.push_back(this->topBarSysMinRect());
			out.push_back(this->topBarSysMaxRect());
			out.push_back(this->topBarSysCloseRect());
			return out;
			});
	}
#endif

	// 初始化导航 VM 与 View：新增“数据”项
	m_navVm.setItems(QVector<NavViewModel::Item>{
		{.id = "home", .svgLight = ":/icons/home_light.svg", .svgDark = ":/icons/home_dark.svg", .label = "首页" },
		{ .id = "data",    .svgLight = ":/icons/data_light.svg",  .svgDark = ":/icons/data_dark.svg",  .label = "数据" },
		{ .id = "explore", .svgLight = ":/icons/explore_light.svg",.svgDark = ":/icons/explore_dark.svg",.label = "探索" },
		{ .id = "favorites", .svgLight = ":/icons/fav_light.svg", .svgDark = ":/icons/fav_dark.svg",   .label = "收藏" },
		{ .id = "settings",  .svgLight = ":/icons/settings_light.svg", .svgDark = ":/icons/settings_dark.svg", .label = "设置" },
	});
	m_navVm.setSelectedIndex(0);
	m_navVm.setExpanded(false);

	// 组件加入顺序决定绘制层级：先 Page（底层），后 Nav，再 TopBar（最上层）
	m_uiRoot.add(&m_page);

	// 初始化数据页 TabViewModel
	m_dataTabsVm.setItems(QVector<TabViewModel::TabItem>{
		{.id = "formula", .label = "方剂", .tooltip = "中医方剂数据库"},
		{ .id = "herb",       .label = "中药", .tooltip = "中药材信息" },
		{ .id = "classic",    .label = "经典", .tooltip = "经典医籍" },
		{ .id = "case",       .label = "医案", .tooltip = "临床医案记录" },
		{ .id = "internal",   .label = "内科", .tooltip = "内科诊疗" },
		{ .id = "diagnosis",  .label = "诊断", .tooltip = "诊断方法" }
	});
	m_dataTabsVm.setSelectedIndex(0);

	// 设置 TabView
	m_dataTabView.setViewModel(&m_dataTabsVm);
	m_dataTabView.setIndicatorStyle(UiTabView::IndicatorStyle::Bottom);
	m_dataTabView.setTabHeight(43);
	m_dataTabView.setAnimationDuration(220);

	// 在 applyPagePalette 中
	if (m_theme == Theme::Dark) {
		m_dataTabView.setPalette(UiTabView::Palette{
			.barBg = QColor(255,255,255,10),
			.tabHover = QColor(255,255,255,20),
			.tabSelectedBg = QColor(255,255,255,24),
			.indicator = QColor(0,122,255,220),
			.label = QColor(220,230,240,230),
			.labelSelected = QColor(255,255,255,255)
			});
	}
	else {
		m_dataTabView.setPalette(UiTabView::Palette{
			.barBg = QColor(0,0,0,6),
			.tabHover = QColor(0,0,0,10),
			.tabSelectedBg = QColor(0,0,0,14),
			.indicator = QColor(0,102,204,220),
			.label = QColor(70,76,84,255),
			.labelSelected = QColor(40,46,54,255)
			});
	}

	m_nav.setViewModel(&m_navVm);
	m_nav.setDarkTheme(m_theme == Theme::Dark);
	m_nav.setPalette(paletteNavForTheme(m_theme));
	m_nav.setIconLogicalSize(22);
	m_nav.setItemHeight(48);
	m_nav.setLabelFontPx(13);
	m_nav.setWidths(m_navCollapsedW, m_navExpandedW);

	applyTopBarPalette();
	m_topBar.setSvgPaths(m_svgThemeWhenDark, m_svgThemeWhenLight, m_svgFollowOn, m_svgFollowOff);
	// 新增：为系统三大键指定 SVG（也可不调用，UiTopBar 有默认值）
	m_topBar.setSystemButtonSvgPaths(":/icons/sys_min.svg", ":/icons/sys_max.svg", ":/icons/sys_close.svg");

	m_uiRoot.add(&m_nav);
	m_uiRoot.add(&m_topBar);

	// 订阅 ThemeManager
	connect(&m_themeMgr, &ThemeManager::effectiveColorSchemeChanged, this,
		[this](const Qt::ColorScheme s) { setTheme(schemeToTheme(s)); });

	connect(&m_themeMgr, &ThemeManager::modeChanged, this,
		[this](const ThemeManager::ThemeMode mode) {
			const bool follow = (mode == ThemeManager::ThemeMode::FollowSystem);

			// 启动阶段：无动画就位；运行阶段：开启动画
			m_topBar.setFollowSystem(follow, /*animate=*/!m_booting);

			if (!m_booting) {
				if (!m_animTimer.isActive()) {
					m_animClock.start();
					m_animTimer.start();
				}
			}
			// 跟随图标切换需要刷新资源上下文
			m_uiRoot.updateResourceContext(m_iconLoader, this, static_cast<float>(devicePixelRatio()));
			updateTitle();
			update();
#ifdef Q_OS_WIN
			if (m_winChrome) m_winChrome->notifyLayoutChanged();
#endif
		});

	// 监听导航选中项变化 -> 更新页面标题/内容
	connect(&m_navVm, &NavViewModel::selectedIndexChanged, this, &MainOpenGlWindow::updatePageFromSelection);

	// 监听 Tab 切换（可选）
	connect(&m_dataTabsVm, &TabViewModel::selectedIndexChanged, this, [this](int idx) {
		// 可以根据选中的 Tab 更新内容区域
		const QString selectedId = m_dataTabsVm.selectedId();
		qDebug() << "Tab selected:" << selectedId << "at index" << idx;

		// 这里可以切换不同的内容组件
		// 例如：updateDataContent(selectedId);

		update();
		});
	// 加载/同步主题（可能触发上述两个信号）
	m_themeMgr.load();

	// 兜底同步（若未触发，或确保首帧无动画）
	setTheme(schemeToTheme(m_themeMgr.effectiveColorScheme()));
	m_topBar.setFollowSystem(m_themeMgr.mode() == ThemeManager::ThemeMode::FollowSystem, /*animate=*/false);

	// 在设置主题后应用所有调色板
	applyThemeColors();
	applyNavPalette();
	applyPagePalette();
	applyTabViewPalette();

	// 初始页面标题与布局
	updatePageFromSelection(m_navVm.selectedIndex());
	updateLayout();
	updateTitle();

	// 启动完成，后续切换再走动画
	m_booting = false;
}

void MainOpenGlWindow::resizeGL(const int w, const int h)
{
	m_fbWpx = w; m_fbHpx = h;
	m_renderer.resize(m_fbWpx, m_fbHpx);
	updateLayout();
#ifdef Q_OS_WIN
	if (m_winChrome) m_winChrome->notifyLayoutChanged();
#endif
}

void MainOpenGlWindow::paintGL()
{
	if (Render::FrameData latest; m_renderBus.consume(latest)) {
		m_baseFrameData = std::move(latest);
	}

	glClearColor(m_clearColor.redF(), m_clearColor.greenF(), m_clearColor.blueF(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Render::FrameData toDraw = m_baseFrameData;
	appendUiOverlay(toDraw);
	m_renderer.drawFrame(toDraw, m_iconLoader, static_cast<float>(devicePixelRatio()));
}

void MainOpenGlWindow::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		if (m_uiRoot.onMousePress(e->pos())) { update(); e->accept(); return; }
	}
	QOpenGLWindow::mousePressEvent(e);
}

void MainOpenGlWindow::mouseMoveEvent(QMouseEvent* e)
{
	const bool any = m_uiRoot.onMouseMove(e->pos());
	setCursor(any ? Qt::PointingHandCursor : Qt::ArrowCursor);
	if (any) update();
	QOpenGLWindow::mouseMoveEvent(e);
}

void MainOpenGlWindow::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		if (m_uiRoot.onMouseRelease(e->pos())) {
			// 读取顶栏动作
			if (bool clickedTheme = false, clickedFollow = false; m_topBar.takeActions(clickedTheme, clickedFollow)) {
				if (clickedTheme) toggleTheme();
				if (clickedFollow) toggleFollowSystem();
			}
			if (bool cMin = false, cMax = false, cClose = false; m_topBar.takeSystemActions(cMin, cMax, cClose)) {
				if (cClose) close();
				if (cMin) showMinimized();
				if (cMax) { if (visibility() == Maximized) showNormal(); else showMaximized(); }
			}

			// 关键修改：任一组件消费点击后，若计时器未启动，则启动动画计时器
			if (!m_animTimer.isActive()) {
				m_animClock.start();
				m_animTimer.start();
			}

			update();
			e->accept();
			return;
		}
	}
	QOpenGLWindow::mouseReleaseEvent(e);
}

void MainOpenGlWindow::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		if (m_nav.bounds().contains(e->pos())) {
			m_navVm.toggleExpanded();
			updateLayout();
			if (!m_animTimer.isActive()) {
				m_animClock.start();
				m_animTimer.start();
			}
			e->accept();
			return;
		}
	}
	QOpenGLWindow::mouseDoubleClickEvent(e);
}

void MainOpenGlWindow::setTheme(const Theme t)
{
	if (m_theme == t) return;
	m_theme = t;

	applyThemeColors();
	applyTopBarPalette();
	applyNavPalette();
	applyPagePalette();
	applyTabViewPalette();  // 添加这一行

	m_nav.setDarkTheme(m_theme == Theme::Dark);
	m_topBar.setDarkTheme(m_theme == Theme::Dark);

	m_uiRoot.updateResourceContext(m_iconLoader, this, static_cast<float>(devicePixelRatio()));
	updateTitle();
	update();
}

void MainOpenGlWindow::setFollowSystem(const bool on)
{
	if (on) m_themeMgr.setMode(ThemeManager::ThemeMode::FollowSystem);
	else {
		const Theme cur = schemeToTheme(m_themeMgr.effectiveColorScheme());
		m_themeMgr.setMode(cur == Theme::Dark ? ThemeManager::ThemeMode::Dark : ThemeManager::ThemeMode::Light);
	}
}

void MainOpenGlWindow::toggleTheme()
{
	const Theme cur = schemeToTheme(m_themeMgr.effectiveColorScheme());
	const Theme next = (cur == Theme::Dark ? Theme::Light : Theme::Dark);
	m_themeMgr.setMode(next == Theme::Dark ? ThemeManager::ThemeMode::Dark : ThemeManager::ThemeMode::Light);
}

void MainOpenGlWindow::toggleFollowSystem()
{
	setFollowSystem(m_themeMgr.mode() != ThemeManager::ThemeMode::FollowSystem);
}

void MainOpenGlWindow::toggleNavExpanded()
{
	m_navVm.toggleExpanded();
	updateLayout();
	if (!m_animTimer.isActive()) {
		m_animClock.start();
		m_animTimer.start();
	}
	update();
}

void MainOpenGlWindow::applyThemeColors()
{
	if (m_theme == Theme::Dark) m_clearColor = QColor::fromRgbF(0.05f, 0.10f, 0.15f);
	else                        m_clearColor = QColor::fromRgbF(0.91f, 0.92f, 0.94f);
}

void MainOpenGlWindow::updateLayout()
{
	// 计算页面 viewport，使其不与导航重叠
	const int left = m_nav.currentWidth();
	const QSize winSz = size();
	const QRect vp(left, 0, std::max(0, winSz.width() - left), winSz.height());
	m_page.setViewportRect(vp);

	// 同步各组件布局与资源上下文
	m_uiRoot.updateLayout(winSz);
	const float dpr = static_cast<float>(devicePixelRatio());
	m_uiRoot.updateResourceContext(m_iconLoader, this, dpr);

#ifdef Q_OS_WIN
	if (m_winChrome) m_winChrome->notifyLayoutChanged();
#endif
}

void MainOpenGlWindow::updateTitle()
{
	const QString themeText = m_theme == Theme::Dark ? QStringLiteral("暗色") : QStringLiteral("浅色");
	const bool follow = (m_themeMgr.mode() == ThemeManager::ThemeMode::FollowSystem);
	const QString followText = follow ? QStringLiteral("（跟随系统）") : QStringLiteral("（自定义）");
	setTitle(QStringLiteral("Qt6 QOpenGLWindow 示例 - %1 %2").arg(themeText, followText));
}

void MainOpenGlWindow::appendUiOverlay(Render::FrameData& fd) const
{
	m_uiRoot.append(fd);
}

void MainOpenGlWindow::submitFrame(const Render::FrameData& fd, const bool scheduleUpdate)
{
	m_renderBus.submit(fd);
	if (scheduleUpdate) {
		QMetaObject::invokeMethod(this, [this] { this->update(); }, Qt::QueuedConnection);
	}
}

void MainOpenGlWindow::onAnimTick()
{
	const bool any = m_uiRoot.tick();
	// 导航展开/指示条动画进行时，实时更新布局（会推动 Page viewport 左侧随之变化）
	if (m_nav.hasActiveAnimation()) updateLayout();
	if (!any) m_animTimer.stop();
	update();
}

void MainOpenGlWindow::applyTopBarPalette()
{
	const auto [btnBg, btnBgHover, btnBgPressed, iconColor] = paletteBtnForTheme(m_theme);
	UiTopBar::Palette p{ .bg = btnBg, .bgHover = btnBgHover, .bgPressed = btnBgPressed, .icon = iconColor };
	m_topBar.setPalette(p);
}

void MainOpenGlWindow::applyNavPalette()
{
	m_nav.setPalette(paletteNavForTheme(m_theme));
}

void MainOpenGlWindow::applyPagePalette()
{
	if (m_theme == Theme::Dark) {
		m_page.setPalette(UiPage::Palette{
			.cardBg = QColor(28, 38, 50, 200),
			.headingColor = QColor(235, 240, 245, 255),
			.bodyColor = QColor(210, 220, 230, 220)
			});

	}
	else {
		m_page.setPalette(UiPage::Palette{
			.cardBg = QColor(255, 255, 255, 245),
			.headingColor = QColor(40, 46, 54, 255),
			.bodyColor = QColor(70, 76, 84, 220)
			});

	}
}

void MainOpenGlWindow::applyTabViewPalette()
{
	if (m_theme == Theme::Dark) {
		m_dataTabView.setPalette(UiTabView::Palette{
			.barBg = QColor(255,255,255,10),
			.tabHover = QColor(255,255,255,20),
			.tabSelectedBg = QColor(100,100,100,128),
			.indicator = QColor(0,122,255,220),
			.label = QColor(230,240,250,255),
			.labelSelected = QColor(255,255,255,255)
			});
	}
	else {
		m_dataTabView.setPalette(UiTabView::Palette{
			.barBg = QColor(0,0,0,6),
			.tabHover = QColor(0,0,0,10),
			.tabSelectedBg = QColor(0,0,0,14),
			.indicator = QColor(0,102,204,220),
			.label = QColor(70,76,84,255),
			.labelSelected = QColor(40,46,54,255)
			});
	}
}

bool MainOpenGlWindow::isDataPageIndex(const int idx) const
{
	const auto& items = m_navVm.items();
	if (idx < 0 || idx >= items.size()) return false;
	return items[idx].id == QStringLiteral("data");
}

void MainOpenGlWindow::updatePageFromSelection(const int idx)
{
	if (const auto& items = m_navVm.items(); idx >= 0 && idx < items.size()) {
		// 标题
		m_page.setTitle(items[idx].label);
	}

	// 内容：仅“数据”页使用 UiDataTabs
	if (isDataPageIndex(idx)) {
		m_page.setContent(&m_dataTabView);
	}
	else {
		m_page.setContent(nullptr);
	}

	update(); // 触发重绘
}