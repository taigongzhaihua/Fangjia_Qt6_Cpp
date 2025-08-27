#include "MainOpenGlWindow.h"

#include "AppConfig.h"
#include "DataPage.h"
#include "ExplorePage.h"
#include "FavoritesPage.h"
#include "HomePage.h"
#include "ServiceLocator.h"
#include "SettingsPage.h"
#include "ThemeManager.h"

#ifdef Q_OS_WIN
#include "WinWindowChrome.h"
#endif
#include <qsystemdetection.h>
#include <qtimer.h>
#include <qcolor.h>
#include <qopenglwindow.h>
#include <RenderData.hpp>
#include <UiPage.h>
#include <UiNav.h>
#include <UiTopBar.h>
#include <NavViewModel.h>
#include <gl/GL.h>
#include <memory>
#include <qcontainerfwd.h>
#include <qsize.h>
#include <qstring.h>

#include <algorithm>
#include <qbytearray.h>
#include <qevent.h>
#include <qnamespace.h>
#include <qrect.h>
#include <qwindow.h>
#include <exception>
#include <qlogging.h>

namespace {
	inline MainOpenGlWindow::Theme schemeToTheme(const Qt::ColorScheme s) {
		return s == Qt::ColorScheme::Dark ? MainOpenGlWindow::Theme::Dark : MainOpenGlWindow::Theme::Light;
	}

	QByteArray saveWindowGeometry(const QWindow* window) {
		QByteArray data;
		data.resize(sizeof(int) * 4);
		const auto ptr = reinterpret_cast<int*>(data.data());
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
	try {
		qDebug() << "MainOpenGlWindow constructor start";
		initializeServices();

		// 设置动画定时器
		connect(&m_animTimer, &QTimer::timeout, this, &MainOpenGlWindow::onAnimationTick);
		m_animTimer.setTimerType(Qt::PreciseTimer);
		m_animTimer.setInterval(16);
		m_animClock.start();

		qDebug() << "MainOpenGlWindow constructor end";
	}
	catch (const std::exception& e) {
		qCritical() << "Exception in MainOpenGlWindow constructor:" << e.what();
		throw;
	}
}

MainOpenGlWindow::~MainOpenGlWindow()
{
	try {
		// 保存窗口状态
		if (m_config) {
			m_config->setWindowGeometry(saveWindowGeometry(this));
			m_config->setNavSelectedIndex(m_navVm.selectedIndex());
			m_config->setNavExpanded(m_navVm.expanded());
			m_config->save();
		}

#ifdef Q_OS_WIN
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
	catch (const std::exception& e) {
		qCritical() << "Exception in MainOpenGlWindow destructor:" << e.what();
	}
}

void MainOpenGlWindow::initializeGL()
{
	try {
		qDebug() << "MainOpenGlWindow::initializeGL start";

		initializeOpenGLFunctions();
		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		m_renderer.initializeGL(this);

#ifdef Q_OS_WIN
		if (!m_winChrome) {
			qDebug() << "Attaching WinWindowChrome...";
			m_winChrome = WinWindowChrome::attach(this, 56, [this]() {
				QVector<QRect> excludeRects;
				excludeRects.push_back(navBounds());
				excludeRects.push_back(topBarBounds());
				return excludeRects;
				});
		}
#endif

		// 先确定初始主题
		if (m_themeMgr) {
			m_theme = schemeToTheme(m_themeMgr->effectiveColorScheme());
		}
		else {
			m_theme = Theme::Light;  // 默认浅色
		}

		// 设置清屏颜色
		if (m_theme == Theme::Dark) {
			m_clearColor = QColor::fromRgbF(0.05f, 0.10f, 0.15f);
		}
		else {
			m_clearColor = QColor::fromRgbF(0.91f, 0.92f, 0.94f);
		}

		qDebug() << "Initializing navigation...";
		initializeNavigation();

		qDebug() << "Initializing pages...";
		initializePages();

		qDebug() << "Initializing top bar...";
		initializeTopBar();

		// 添加UI组件到根容器
		m_uiRoot.add(&m_nav);
		m_uiRoot.add(&m_topBar);
		if (auto* currentPage = m_pageManager.currentPage()) {
			m_uiRoot.add(currentPage);
		}

		// 在所有组件添加后，应用初始主题
		const bool isDark = (m_theme == Theme::Dark);
		m_uiRoot.propagateThemeChange(isDark);

		updateLayout();

		// 设置主题监听
		setupThemeListeners();

		qDebug() << "MainOpenGlWindow::initializeGL end";
	}
	catch (const std::exception& e) {
		qCritical() << "Exception in initializeGL:" << e.what();
		throw;
	}
}


void MainOpenGlWindow::resizeGL(const int w, const int h)
{
	m_fbWpx = w;
	m_fbHpx = h;
	m_renderer.resize(w, h);
	updateLayout();

#ifdef Q_OS_WIN
	if (m_winChrome) m_winChrome->notifyLayoutChanged();
#endif
}

void MainOpenGlWindow::paintGL()
{
	glClearColor(m_clearColor.redF(), m_clearColor.greenF(), m_clearColor.blueF(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Render::FrameData frameData;
	m_uiRoot.append(frameData);
	m_renderer.drawFrame(frameData, m_iconLoader, static_cast<float>(devicePixelRatio()));
}

void MainOpenGlWindow::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		if (m_uiRoot.onMousePress(e->pos())) {
			update();
			e->accept();
			return;
		}
	}
	QOpenGLWindow::mousePressEvent(e);
}

void MainOpenGlWindow::mouseMoveEvent(QMouseEvent* e)
{
	const bool handled = m_uiRoot.onMouseMove(e->pos());
	setCursor(handled ? Qt::PointingHandCursor : Qt::ArrowCursor);
	if (handled) update();
	QOpenGLWindow::mouseMoveEvent(e);
}

void MainOpenGlWindow::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		if (m_uiRoot.onMouseRelease(e->pos())) {
			// 处理顶栏按钮点击
			if (bool theme = false, follow = false; m_topBar.takeActions(theme, follow)) {
				if (theme) onThemeToggle();
				if (follow) onFollowSystemToggle();
			}

			if (bool min = false, max = false, close = false; m_topBar.takeSystemActions(min, max, close)) {
				if (close) this->close();
				if (min) showMinimized();
				if (max) {
					if (visibility() == Maximized) showNormal();
					else showMaximized();
				}
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

void MainOpenGlWindow::initializeServices()
{
	// 从DI获取服务
	m_config = DI.get<AppConfig>();
	m_themeMgr = DI.get<ThemeManager>();

	if (!m_themeMgr) {
		m_themeMgr = std::make_shared<ThemeManager>();
	}
}

void MainOpenGlWindow::initializeNavigation()
{
	// 设置导航项
	m_navVm.setItems(QVector<NavViewModel::Item>{
		{.id = "home", .svgLight = ":/icons/home_light.svg", .svgDark = ":/icons/home_dark.svg", .label = "首页"},
		{ .id = "data", .svgLight = ":/icons/data_light.svg", .svgDark = ":/icons/data_dark.svg", .label = "数据" },
		{ .id = "explore", .svgLight = ":/icons/explore_light.svg", .svgDark = ":/icons/explore_dark.svg", .label = "探索" },
		{ .id = "favorites", .svgLight = ":/icons/fav_light.svg", .svgDark = ":/icons/fav_dark.svg", .label = "收藏" },
		{ .id = "settings", .svgLight = ":/icons/settings_light.svg", .svgDark = ":/icons/settings_dark.svg", .label = "设置" }
	});

	// 从配置恢复状态
	if (m_config) {
		if (const int savedIndex = m_config->navSelectedIndex(); savedIndex >= 0 && savedIndex < m_navVm.count()) {
			m_navVm.setSelectedIndex(savedIndex);
		}
		else {
			m_navVm.setSelectedIndex(0);
		}
		m_navVm.setExpanded(m_config->navExpanded());
	}

	// 设置导航视图
	m_nav.setViewModel(&m_navVm);
	m_nav.setIconLogicalSize(22);
	m_nav.setItemHeight(48);
	m_nav.setLabelFontPx(13);
	m_nav.setWidths(48, 200);

	// 连接导航选择变化
	connect(&m_navVm, &NavViewModel::selectedIndexChanged, this, &MainOpenGlWindow::onNavSelectionChanged);
}

void MainOpenGlWindow::initializePages()
{
	try
	{
		// 创建并注册所有页面
		m_pageManager.registerPage("home", std::make_unique<HomePage>());
		m_pageManager.registerPage("data", std::make_unique<DataPage>());
		m_pageManager.registerPage("explore", std::make_unique<ExplorePage>());
		m_pageManager.registerPage("favorites", std::make_unique<FavoritesPage>());
		m_pageManager.registerPage("settings", std::make_unique<SettingsPage>());

		// 切换到初始页面
		const auto& items = m_navVm.items();
		if (m_navVm.selectedIndex() >= 0 && m_navVm.selectedIndex() < items.size()) {
			m_pageManager.switchToPage(items[m_navVm.selectedIndex()].id);
		}
	}
	catch (const std::exception& e) {
		qCritical() << "Exception in initializePages:" << e.what();
		throw;
	}

}
void MainOpenGlWindow::initializeTopBar()
{
	// 设置顶栏
	m_topBar.setCornerRadius(8.0f);
	m_topBar.setSvgPaths(":/icons/sun.svg", ":/icons/moon.svg",
		":/icons/follow_on.svg", ":/icons/follow_off.svg");
	m_topBar.setSystemButtonSvgPaths(":/icons/sys_min.svg",
		":/icons/sys_max.svg",
		":/icons/sys_close.svg");

	// 设置初始主题状态
	const bool isDark = (m_theme == Theme::Dark);
	m_topBar.setDarkTheme(isDark);

	// 设置跟随系统状态
	const bool followSystem = m_themeMgr && m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem;
	m_topBar.setFollowSystem(followSystem, false);  // false = 无动画
}

void MainOpenGlWindow::initializeTheme()
{
	// 设置顶栏
	m_topBar.setCornerRadius(8.0f);
	m_topBar.setSvgPaths(":/icons/sun.svg", ":/icons/moon.svg",
		":/icons/follow_on.svg", ":/icons/follow_off.svg");
	m_topBar.setSystemButtonSvgPaths(":/icons/sys_min.svg",
		":/icons/sys_max.svg",
		":/icons/sys_close.svg");

	// 监听主题变化
	if (m_themeMgr) {
		connect(m_themeMgr.get(), &ThemeManager::effectiveColorSchemeChanged, this,
			[this](const Qt::ColorScheme s) {
				setTheme(schemeToTheme(s));
			});

		connect(m_themeMgr.get(), &ThemeManager::modeChanged, this,
			[this](const ThemeManager::ThemeMode mode) {
				const bool follow = (mode == ThemeManager::ThemeMode::FollowSystem);
				m_topBar.setFollowSystem(follow, true);
				updateLayout();
				update();
			});
	}

	// 应用初始主题
	setTheme(schemeToTheme(m_themeMgr ? m_themeMgr->effectiveColorScheme() : Qt::ColorScheme::Light));
	m_topBar.setFollowSystem(m_themeMgr && m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem, false);
}

void MainOpenGlWindow::setupThemeListeners()
{
	// 监听主题变化
	if (m_themeMgr) {
		connect(m_themeMgr.get(), &ThemeManager::effectiveColorSchemeChanged, this,
			[this](const Qt::ColorScheme s) {
				setTheme(schemeToTheme(s));
			});

		connect(m_themeMgr.get(), &ThemeManager::modeChanged, this,
			[this](const ThemeManager::ThemeMode mode) {
				const bool follow = (mode == ThemeManager::ThemeMode::FollowSystem);
				m_topBar.setFollowSystem(follow, true);
				updateLayout();
				update();
			});
	}
}

void MainOpenGlWindow::updateLayout()
{
	const QSize winSize = size();
	const int navWidth = m_nav.currentWidth();

	// 设置页面视口（避开导航栏）
	const QRect pageViewport(navWidth, 0, std::max(0, winSize.width() - navWidth), winSize.height());

	if (auto* currentPage = m_pageManager.currentPage()) {
		currentPage->setViewportRect(pageViewport);
	}

	// 更新所有组件布局
	m_uiRoot.updateLayout(winSize);
	m_uiRoot.updateResourceContext(m_iconLoader, this, static_cast<float>(devicePixelRatio()));

#ifdef Q_OS_WIN
	if (m_winChrome) m_winChrome->notifyLayoutChanged();
#endif
}

void MainOpenGlWindow::setTheme(const Theme t)
{
	if (m_theme == t) return;
	m_theme = t;
	applyTheme();
}

void MainOpenGlWindow::applyTheme()
{
	const bool isDark = (m_theme == Theme::Dark);

	// 设置清屏颜色
	if (isDark) {
		m_clearColor = QColor::fromRgbF(0.05f, 0.10f, 0.15f);
	}
	else {
		m_clearColor = QColor::fromRgbF(0.91f, 0.92f, 0.94f);
	}

	// 通过UiRoot传播主题变化到所有组件
	m_uiRoot.propagateThemeChange(isDark);

	// 更新资源上下文（图标可能需要重新加载）
	m_uiRoot.updateResourceContext(m_iconLoader, this, static_cast<float>(devicePixelRatio()));

	update();
}

bool MainOpenGlWindow::followSystem() const noexcept
{
	return m_themeMgr && m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem;
}

void MainOpenGlWindow::setFollowSystem(const bool on) const
{
	if (!m_themeMgr) return;

	if (on) {
		m_themeMgr->setMode(ThemeManager::ThemeMode::FollowSystem);
	}
	else {
		const Theme cur = schemeToTheme(m_themeMgr->effectiveColorScheme());
		m_themeMgr->setMode(cur == Theme::Dark ? ThemeManager::ThemeMode::Dark : ThemeManager::ThemeMode::Light);
	}
}

void MainOpenGlWindow::onNavSelectionChanged(const int index)
{
	const auto& items = m_navVm.items();
	if (index >= 0 && index < items.size()) {
		const QString pageId = items[index].id;

		// 从UiRoot中移除旧页面
		if (auto* oldPage = m_pageManager.currentPage()) {
			m_uiRoot.remove(oldPage);
		}

		// 切换到新页面
		if (m_pageManager.switchToPage(pageId)) {
			if (auto* newPage = m_pageManager.currentPage()) {
				// 设置页面视口
				const int navWidth = m_nav.currentWidth();
				const QSize winSize = size();
				const QRect pageViewport(navWidth, 0, std::max(0, winSize.width() - navWidth), winSize.height());
				newPage->setViewportRect(pageViewport);

				// 添加到UiRoot
				m_uiRoot.add(newPage);

				m_uiRoot.propagateThemeChange(m_theme == Theme::Dark);
				// 更新资源上下文
				newPage->updateResourceContext(m_iconLoader, this, static_cast<float>(devicePixelRatio()));

			}
		}

		update();
	}
}

void MainOpenGlWindow::onThemeToggle() const
{
	if (!m_themeMgr) return;

	const Theme cur = schemeToTheme(m_themeMgr->effectiveColorScheme());
	const Theme next = (cur == Theme::Dark ? Theme::Light : Theme::Dark);
	m_themeMgr->setMode(next == Theme::Dark ? ThemeManager::ThemeMode::Dark : ThemeManager::ThemeMode::Light);
}

void MainOpenGlWindow::onFollowSystemToggle() const
{
	if (!m_themeMgr) return;
	setFollowSystem(m_themeMgr->mode() != ThemeManager::ThemeMode::FollowSystem);
}

void MainOpenGlWindow::onAnimationTick()
{
	const bool hasAnimation = m_uiRoot.tick();

	if (m_nav.hasActiveAnimation()) {
		updateLayout();
	}

	if (!hasAnimation) {
		m_animTimer.stop();
	}

	update();
}