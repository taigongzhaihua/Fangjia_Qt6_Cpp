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

namespace {
	struct PaletteBtn { QColor btnBg, btnBgHover, btnBgPressed, iconColor; };
	PaletteBtn paletteBtnForTheme(const MainOpenGlWindow::Theme t) {
		using T = MainOpenGlWindow::Theme;
		if (t == T::Dark) {
			return { .btnBg = QColor(52,63,76,120), .btnBgHover = QColor(66, 78,92,200), .btnBgPressed = QColor(58, 70, 84,220), .iconColor =
				QColor(255, 255,255,255) };
		}
		return { .btnBg = QColor(240,243,247,200), .btnBgHover = QColor(232, 237,242,220), .btnBgPressed = QColor(225, 230,236,230), .iconColor =
			QColor(60, 64,72,220) };
	}

	Ui::NavPalette paletteNavForTheme(const MainOpenGlWindow::Theme t) {
		if (t == MainOpenGlWindow::Theme::Dark) {
			return Ui::NavPalette{
				.railBg = QColor(21, 28, 36, 166),
				.itemHover = QColor(255,255,255,18),
				.itemPressed = QColor(255,255,255,30),
				.itemSelected = QColor(255,255,255,36),
				.iconColor = QColor(230,236,242,255),
				.labelColor = QColor(230,236,242,230),
				.indicator = QColor(0,122,255,200)
			};
		}
		return Ui::NavPalette{
			.railBg = QColor(246,248,250,127),
			.itemHover = QColor(0,0,0,14),
			.itemPressed = QColor(0,0,0,26),
			.itemSelected = QColor(0,0,0,32),
			.iconColor = QColor(70,76,84,255),
			.labelColor = QColor(70,76,84,220),
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
	m_navExpandedW = 220;

	m_topBar.setCornerRadius(8.0f);
}

MainOpenGlWindow::~MainOpenGlWindow()
{
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
		const int dragHeight = 56;
		m_winChrome = WinWindowChrome::attach(this, dragHeight, [this]() {
			// 返回不允许拖拽的区域（例如导航栏、右上角按钮），均为逻辑像素坐标
			QVector<QRect> out;
			out.push_back(this->navBounds());
			out.push_back(this->topBarThemeRect());
			out.push_back(this->topBarFollowRect());
			return out;
			});
	}
#endif

	// 初始化导航 VM 与 View
	m_navVm.setItems(QVector<NavViewModel::Item>{
		{.id = "home", .svgLight = ":/icons/home_light.svg", .svgDark = ":/icons/home_dark.svg", .label = "首页"},
		{ .id = "explore",   .svgLight = ":/icons/explore_light.svg",  .svgDark = ":/icons/explore_dark.svg",  .label = "探索" },
		{ .id = "favorites", .svgLight = ":/icons/fav_light.svg",      .svgDark = ":/icons/fav_dark.svg",      .label = "收藏" },
		{ .id = "settings",  .svgLight = ":/icons/settings_light.svg", .svgDark = ":/icons/settings_dark.svg", .label = "设置" },
	});
	m_navVm.setSelectedIndex(0);
	m_navVm.setExpanded(false);

	// 组件加入顺序决定绘制层级：先 Page（底层），后 Nav，再 TopBar（最上层）
	m_uiRoot.add(&m_page);

	m_nav.setViewModel(&m_navVm);
	m_nav.setDarkTheme(m_theme == Theme::Dark);
	m_nav.setPalette(paletteNavForTheme(m_theme));
	m_nav.setIconLogicalSize(22);
	m_nav.setItemHeight(48);
	m_nav.setLabelFontPx(13);
	m_nav.setWidths(m_navCollapsedW, m_navExpandedW);

	applyTopBarPalette();
	m_topBar.setSvgPaths(m_svgThemeWhenDark, m_svgThemeWhenLight, m_svgFollowOn, m_svgFollowOff);

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

	// 监听导航选中项变化 -> 更新页面标题
	connect(&m_navVm, &NavViewModel::selectedIndexChanged, this, &MainOpenGlWindow::updatePageFromSelection);

	// 加载/同步主题（可能触发上述两个信号）
	m_themeMgr.load();

	// 兜底同步（若未触发，或确保首帧无动画）
	setTheme(schemeToTheme(m_themeMgr.effectiveColorScheme()));
	m_topBar.setFollowSystem(m_themeMgr.mode() == ThemeManager::ThemeMode::FollowSystem, /*animate=*/false);

	applyThemeColors();
	applyNavPalette();
	applyPagePalette();

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
			// 读取顶栏动作（单向数据流：View 发意图 -> Window 调 VM）
			if (bool clickedTheme = false, clickedFollow = false; m_topBar.takeActions(clickedTheme, clickedFollow)) {
				if (clickedTheme) toggleTheme();
				if (clickedFollow) toggleFollowSystem();
			}

			if (m_nav.hasActiveAnimation() && !m_animTimer.isActive()) {
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
	// 先计算页面 viewport，使其不与导航重叠
	const int left = m_nav.currentWidth();
	const QSize winSz = size();
	m_page.setViewportRect(QRect(left, 0, std::max(0, winSz.width() - left), winSz.height()));

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

void MainOpenGlWindow::updatePageFromSelection(const int idx)
{
	const auto& items = m_navVm.items();
	if (idx >= 0 && idx < items.size()) {
		m_page.setTitle(items[idx].label);
		update();
	}
}