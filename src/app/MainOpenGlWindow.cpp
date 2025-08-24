#include "MainOpenGlWindow.h"

#include "AppConfig.h"
#include "NavViewModel.h"
#include "RenderData.hpp"
#include "ServiceLocator.h"
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
#include <qlogging.h>
#include "TabViewModel.h"
#include "UiTabView.h"
#include <memory>
#include "UiFormulaView.h"
#include <utility>
#include <qbytearray.h>
#include <qwindow.h>

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

	// 辅助函数：保存窗口几何信息
	QByteArray saveWindowGeometry(QWindow* window) {
		QByteArray data;
		data.resize(sizeof(int) * 4);
		int* ptr = reinterpret_cast<int*>(data.data());
		ptr[0] = window->x();
		ptr[1] = window->y();
		ptr[2] = window->width();
		ptr[3] = window->height();
		return data;
	}
}

MainOpenGlWindow::MainOpenGlWindow(const UpdateBehavior updateBehavior)
	: QOpenGLWindow(updateBehavior)
{
	// 从 DI 获取服务
	m_config = DI.get<AppConfig>();
	m_themeMgr = DI.get<ThemeManager>();

	// 如果没有从 DI 获取到，创建本地实例（回退方案）
	if (!m_themeMgr) {
		m_localThemeMgr = std::make_shared<ThemeManager>();
		m_themeMgr = m_localThemeMgr;
	}

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
	// 保存窗口状态到配置
	if (m_config) {
		// 保存窗口几何信息
		m_config->setWindowGeometry(saveWindowGeometry(this));

		// 保存导航状态
		m_config->setNavSelectedIndex(m_navVm.selectedIndex());
		m_config->setNavExpanded(m_navVm.expanded());

		// 保存最近的 Tab
		if (!m_dataTabsVm.selectedId().isEmpty()) {
			m_config->setRecentTab(m_dataTabsVm.selectedId());
		}

		// 显式保存
		m_config->save();
	}

#ifdef Q_OS_WIN
	// 先卸载原生事件过滤器，避免窗口销毁期间回调到已失效对象
	if (m_winChrome) {
		m_winChrome->detach();
		m_winChrome = nullptr;
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

	// Windows: 安装自定义 WindowChrome
#ifdef Q_OS_WIN
	if (!m_winChrome) {
		constexpr int dragHeight = 56;
		m_winChrome = WinWindowChrome::attach(this, dragHeight, [this]() {
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

	// 初始化导航 VM 与 View
	m_navVm.setItems(QVector<NavViewModel::Item>{
		{.id = "home", .svgLight = ":/icons/home_light.svg", .svgDark = ":/icons/home_dark.svg", .label = "首页" },
		{ .id = "data",    .svgLight = ":/icons/data_light.svg",  .svgDark = ":/icons/data_dark.svg",  .label = "数据" },
		{ .id = "explore", .svgLight = ":/icons/explore_light.svg",.svgDark = ":/icons/explore_dark.svg",.label = "探索" },
		{ .id = "favorites", .svgLight = ":/icons/fav_light.svg", .svgDark = ":/icons/fav_dark.svg",   .label = "收藏" },
		{ .id = "settings",  .svgLight = ":/icons/settings_light.svg", .svgDark = ":/icons/settings_dark.svg", .label = "设置" },
	});

	// 从配置恢复导航状态
	if (m_config) {
		int savedNavIndex = m_config->navSelectedIndex();
		if (savedNavIndex >= 0 && savedNavIndex < m_navVm.count()) {
			m_navVm.setSelectedIndex(savedNavIndex);
		}
		else {
			m_navVm.setSelectedIndex(0);
		}
		m_navVm.setExpanded(m_config->navExpanded());

		// 连接导航变化到配置保存
		connect(&m_navVm, &NavViewModel::selectedIndexChanged, this, [this](int idx) {
			if (m_config) {
				m_config->setNavSelectedIndex(idx);
			}
			});

		connect(&m_navVm, &NavViewModel::expandedChanged, this, [this](bool expanded) {
			if (m_config) {
				m_config->setNavExpanded(expanded);
			}
			});
	}
	else {
		m_navVm.setSelectedIndex(0);
		m_navVm.setExpanded(false);
	}

	m_formulaView = std::make_unique<UiFormulaView>();
	m_formulaView->setDarkTheme(m_theme == Theme::Dark);

	// 修正：UiRoot 添加组件，不是 UiPage
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

	// 从配置恢复最近的 Tab
	if (m_config && !m_config->recentTab().isEmpty()) {
		int tabIdx = m_dataTabsVm.findById(m_config->recentTab());
		if (tabIdx >= 0) {
			m_dataTabsVm.setSelectedIndex(tabIdx);
		}
		else {
			m_dataTabsVm.setSelectedIndex(0);
		}
	}
	else {
		m_dataTabsVm.setSelectedIndex(0);
	}

	// 连接 Tab 变化到配置保存
	connect(&m_dataTabsVm, &TabViewModel::selectedIndexChanged, this, [this](int) {
		if (m_config) {
			m_config->setRecentTab(m_dataTabsVm.selectedId());
		}
		});

	// 设置 TabView
	m_dataTabView.setViewModel(&m_dataTabsVm);
	m_dataTabView.setIndicatorStyle(UiTabView::IndicatorStyle::Bottom);
	m_dataTabView.setTabHeight(43);
	m_dataTabView.setAnimationDuration(220);
	m_dataTabView.setContent(0, m_formulaView.get());

	// 应用调色板
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
	m_topBar.setSystemButtonSvgPaths(":/icons/sys_min.svg", ":/icons/sys_max.svg", ":/icons/sys_close.svg");

	m_uiRoot.add(&m_nav);
	m_uiRoot.add(&m_topBar);

	// 订阅 ThemeManager
	if (m_themeMgr) {
		connect(m_themeMgr.get(), &ThemeManager::effectiveColorSchemeChanged, this,
			[this](const Qt::ColorScheme s) { setTheme(schemeToTheme(s)); });

		connect(m_themeMgr.get(), &ThemeManager::modeChanged, this,
			[this](const ThemeManager::ThemeMode mode) {
				const bool follow = (mode == ThemeManager::ThemeMode::FollowSystem);

				m_topBar.setFollowSystem(follow, /*animate=*/!m_booting);

				if (!m_booting) {
					if (!m_animTimer.isActive()) {
						m_animClock.start();
						m_animTimer.start();
					}
				}
				m_uiRoot.updateResourceContext(m_iconLoader, this, static_cast<float>(devicePixelRatio()));
				updateTitle();
				update();
#ifdef Q_OS_WIN
				if (m_winChrome) m_winChrome->notifyLayoutChanged();
#endif
			});

		// 加载/同步主题（但不需要再次 load，ServiceRegistry 已经加载过了）
		// m_themeMgr->load();  // 注释掉，避免重复加载
	}

	// 监听导航选中项变化
	connect(&m_navVm, &NavViewModel::selectedIndexChanged, this, &MainOpenGlWindow::updatePageFromSelection);

	// 监听 Tab 切换
	connect(&m_dataTabsVm, &TabViewModel::selectedIndexChanged, this, [this](int idx) {
		const QString selectedId = m_dataTabsVm.selectedId();
		qDebug() << "Tab selected:" << selectedId << "at index" << idx << "\n";
		update();
		});

	// 兜底同步
	setTheme(schemeToTheme(m_themeMgr ? m_themeMgr->effectiveColorScheme() : Qt::ColorScheme::Light));
	m_topBar.setFollowSystem(m_themeMgr && m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem, /*animate=*/false);

	// 在设置主题后应用所有调色板
	applyThemeColors();
	applyNavPalette();
	applyPagePalette();
	applyTabViewPalette();

	// 初始页面标题与布局
	updatePageFromSelection(m_navVm.selectedIndex());
	updateLayout();
	updateTitle();

	// 启动完成
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
	applyTabViewPalette();

	m_nav.setDarkTheme(m_theme == Theme::Dark);
	m_topBar.setDarkTheme(m_theme == Theme::Dark);
	m_formulaView->setDarkTheme(m_theme == Theme::Dark);

	m_uiRoot.updateResourceContext(m_iconLoader, this, static_cast<float>(devicePixelRatio()));
	updateTitle();
	update();
}

void MainOpenGlWindow::setFollowSystem(const bool on)
{
	if (!m_themeMgr) return;

	if (on) m_themeMgr->setMode(ThemeManager::ThemeMode::FollowSystem);
	else {
		const Theme cur = schemeToTheme(m_themeMgr->effectiveColorScheme());
		m_themeMgr->setMode(cur == Theme::Dark ? ThemeManager::ThemeMode::Dark : ThemeManager::ThemeMode::Light);
	}
}

void MainOpenGlWindow::toggleTheme()
{
	if (!m_themeMgr) return;

	const Theme cur = schemeToTheme(m_themeMgr->effectiveColorScheme());
	const Theme next = (cur == Theme::Dark ? Theme::Light : Theme::Dark);
	m_themeMgr->setMode(next == Theme::Dark ? ThemeManager::ThemeMode::Dark : ThemeManager::ThemeMode::Light);
}

void MainOpenGlWindow::toggleFollowSystem()
{
	if (!m_themeMgr) return;
	setFollowSystem(m_themeMgr->mode() != ThemeManager::ThemeMode::FollowSystem);
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
	const int left = m_nav.currentWidth();
	const QSize winSz = size();
	const QRect vp(left, 0, std::max(0, winSz.width() - left), winSz.height());
	m_page.setViewportRect(vp);

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
	const bool follow = m_themeMgr && (m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem);
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
		m_page.setTitle(items[idx].label);
	}

	if (isDataPageIndex(idx)) {
		const QString tabId = m_dataTabsVm.selectedId();
		m_page.setContent(&m_dataTabView);
	}
	else {
		m_page.setContent(nullptr);
	}

	update();
}