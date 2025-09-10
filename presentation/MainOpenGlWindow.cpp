/*
 * 文件名：MainOpenGlWindow.cpp
 * 职责：主窗口类实现，继承自基础Window类。
 */

#include "MainOpenGlWindow.hpp"

#include "data/sources/local/AppConfig.h"
#include "apps/fangjia/CurrentPageHost.h"
#include "presentation/pages/DataPage.h"
#include "presentation/pages/ExplorePage.h"
#include "presentation/pages/FavoritesPage.h"
#include "presentation/pages/HomePage.h"
#include "presentation/pages/SettingsPage.h"
#include "presentation/viewmodels/ThemeManager.h"
#include "data/sources/local/DatabaseBootstrapper.h"

#ifdef Q_OS_WIN
#include "infrastructure/platform/windows/WinWindowChrome.h"
#endif
#include <qsystemdetection.h>
#include <qtimer.h>
#include <qcolor.h>
#include <infrastructure/gfx/RenderData.hpp>
#include <presentation/ui/widgets/UiNav.h>
#include <presentation/ui/widgets/UiTopBar.h>
#include <GL/gl.h>
#include <memory>
#include <qcontainerfwd.h>
#include <qsize.h>
#include <qstring.h>

#include <qbytearray.h>
#include <qevent.h>
#include <qnamespace.h>
#include <qrect.h>
#include <qwindow.h>
#include <exception>
#include <qlogging.h>
#include <utility>
#include <presentation/ui/declarative/Binding.h>
#include <presentation/ui/declarative/ComponentWrapper.h>
#include <presentation/ui/declarative/RebuildHost.h>
#include <presentation/ui/declarative/UI.h>
#include <presentation/ui/declarative/Widget.h>
#include <qpoint.h>
#include <presentation/viewmodels/NavViewModel.h>

namespace
{
	MainOpenGlWindow::Theme schemeToTheme(const Qt::ColorScheme s)
	{
		return s == Qt::ColorScheme::Dark ? MainOpenGlWindow::Theme::Dark : MainOpenGlWindow::Theme::Light;
	}
}

MainOpenGlWindow::MainOpenGlWindow(
	std::shared_ptr<AppConfig> config,
	std::shared_ptr<ThemeManager> themeManager,
	UpdateBehavior updateBehavior)
	: fj::presentation::ui::base::Window(updateBehavior)
	, m_themeMgr(std::move(themeManager))
	, m_config(std::move(config))
	, m_pageRouter(5)
{
	// 从配置恢复导航状态
	if (m_config) {
		m_navVm.setSelectedIndex(m_config->navSelectedIndex());
		m_navVm.setExpanded(m_config->navExpanded());
	}
}

MainOpenGlWindow::~MainOpenGlWindow()
{
	try {
		qDebug() << "MainOpenGlWindow destructor";

		// 保存窗口状态到配置
		if (m_config) {
			saveWindowGeometry();
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
		m_iconCache.releaseAll(this);
		m_renderer.releaseGL();
		doneCurrent();
	}
	catch (const std::exception& e) {
		qCritical() << "Exception in MainOpenGlWindow destructor:" << e.what();
	}
}

void MainOpenGlWindow::saveWindowGeometry()
{
	if (m_config) {
		const QByteArray geo = QByteArray::fromRawData(
			reinterpret_cast<const char*>(&x()), sizeof(int) * 4);
		m_config->setWindowGeometry(geo);
		qDebug() << "MainOpenGlWindow: Window geometry saved";
	}
}

void MainOpenGlWindow::restoreWindowGeometry()
{
	if (m_config) {
		QByteArray geo = m_config->windowGeometry();
		if (!geo.isEmpty() && geo.size() == sizeof(int) * 4) {
			const auto data = reinterpret_cast<const int*>(geo.data());
			setPosition(data[0], data[1]);
			resize(data[2], data[3]);
			qDebug() << "MainOpenGlWindow: Window geometry restored";
		}
	}
}

void MainOpenGlWindow::initializeWindowGL()
{
	try {
		qDebug() << "MainOpenGlWindow::initializeWindowGL start";

		m_renderer.initializeGL(this);

#ifdef Q_OS_WIN
		if (!m_winChrome) {
			qDebug() << "Attaching WinWindowChrome...";
			m_winChrome = WinWindowChrome::attach(this, 56, [this] {
				QVector<QRect> excludeRects;
				excludeRects.push_back(navBounds());
				excludeRects.push_back(topBarBounds());
				return excludeRects;
			});
		}
#endif

		// 先确定初始主题
		if (m_themeMgr) {
			setTheme(schemeToTheme(m_themeMgr->effectiveColorScheme()));
		} else {
			setTheme(Theme::Light); // 默认浅色
		}

		qDebug() << "Initializing navigation...";
		initializeNavigation();

		qDebug() << "Initializing pages...";
		initializePages();

		qDebug() << "Initializing declarative shell...";
		initializeDeclarativeShell();

		// 在所有组件添加后，应用初始主题
		const bool isDark = (theme() == Theme::Dark);
		m_uiRoot.propagateThemeChange(isDark);

		// 设置主题监听
		setupThemeListeners();

		qDebug() << "MainOpenGlWindow::initializeWindowGL end";
	}
	catch (const std::exception& e) {
		qCritical() << "Exception in initializeWindowGL:" << e.what();
		throw;
	}
}

void MainOpenGlWindow::updateWindowLayout(int w, int h)
{
	m_renderer.resize(w, h);
	
	// 更新UI布局
	const QSize windowSize(w, h);
	m_uiRoot.updateLayout(windowSize);
	m_nav.updateLayout(windowSize);

#ifdef Q_OS_WIN
	if (m_winChrome) m_winChrome->notifyLayoutChanged();
#endif
}

void MainOpenGlWindow::renderWindow()
{
	Render::FrameData frameData;
	m_uiRoot.append(frameData);
	m_renderer.drawFrame(frameData, m_iconCache, static_cast<float>(devicePixelRatio()));
}

void MainOpenGlWindow::onThemeChanged(Theme newTheme)
{
	// 应用主题到UI组件
	const bool isDark = (newTheme == Theme::Dark);
	m_uiRoot.propagateThemeChange(isDark);
	applyTheme();
}

bool MainOpenGlWindow::onAnimationTick()
{
	// 更新UI组件动画
	bool needsMoreAnimation = false;
	
	if (m_uiRoot.tick()) {
		needsMoreAnimation = true;
	}
	
	if (m_nav.tick()) {
		needsMoreAnimation = true;
	}
	
	// 更新资源上下文
	const QSize windowSize(width(), height());
	m_uiRoot.updateResourceContext(m_iconCache, this, static_cast<float>(devicePixelRatio()));
	m_nav.updateResourceContext(m_iconCache, this, static_cast<float>(devicePixelRatio()));
	
	return needsMoreAnimation;
}

void MainOpenGlWindow::mousePressEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		if (m_uiRoot.onMousePress(e->pos())) {
			update();
			e->accept();
			return;
		}
		// 手动处理TopBar空白区域拖拽：不在系统按钮区域内时开始系统移动
		const QPoint p = e->pos();
		const QRect tb = topBarBounds();
		if (tb.contains(p) && !topBarSystemButtonsRect().contains(p)) {
			// 开始系统移动（Qt提供的API）
			startSystemMove();
			e->accept();
			return;
		}
	}
	
	// 调用基类处理
	fj::presentation::ui::base::Window::mousePressEvent(e);
}

void MainOpenGlWindow::mouseMoveEvent(QMouseEvent* e)
{
	const bool handled = m_uiRoot.onMouseMove(e->pos());
	setCursor(handled ? Qt::PointingHandCursor : Qt::ArrowCursor);
	if (handled) update();
	
	// 调用基类处理
	fj::presentation::ui::base::Window::mouseMoveEvent(e);
}

void MainOpenGlWindow::mouseReleaseEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		const bool handled = m_uiRoot.onMouseRelease(e->pos());

		if (handled) {
			// Always schedule a redraw on left-button release to ensure VM-driven rebuilds are rendered
			update();
			e->accept();
			return;
		}
	}
	
	// 调用基类处理
	fj::presentation::ui::base::Window::mouseReleaseEvent(e);
}

void MainOpenGlWindow::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton) {
		if (m_nav.bounds().contains(e->pos())) {
			m_navVm.toggleExpanded();
			updateWindowLayout(width(), height());
			e->accept();
			return;
		}
	}
	
	// 调用基类处理
	fj::presentation::ui::base::Window::mouseDoubleClickEvent(e);
}

void MainOpenGlWindow::wheelEvent(QWheelEvent* e)
{
	if (m_uiRoot.onWheel(e->position().toPoint(), e->angleDelta())) {
		update();
		e->accept();
		return;
	}
	
	// 调用基类处理
	fj::presentation::ui::base::Window::wheelEvent(e);
}

void MainOpenGlWindow::keyPressEvent(QKeyEvent* e)
{
	// 基类处理
	fj::presentation::ui::base::Window::keyPressEvent(e);
}

void MainOpenGlWindow::keyReleaseEvent(QKeyEvent* e)
{
	// 基类处理
	fj::presentation::ui::base::Window::keyReleaseEvent(e);
}

QRect MainOpenGlWindow::topBarSystemButtonsRect() const
{
	const QRect topBar = topBarBounds();
	if (topBar.isEmpty()) return QRect();
	
	// 系统按钮区域：右侧约150像素宽度（5个按钮）
	const int buttonAreaWidth = 150;
	return QRect(topBar.right() - buttonAreaWidth, topBar.top(),
		buttonAreaWidth, topBar.height());
}

void MainOpenGlWindow::setFollowSystem(bool on) const
{
	if (m_themeMgr) {
		m_themeMgr->setFollowSystem(on);
	}
}

bool MainOpenGlWindow::followSystem() const noexcept
{
	return m_themeMgr ? m_themeMgr->followSystem() : false;
}

void MainOpenGlWindow::initializeNavigation()
{
	// 初始化导航组件
	m_nav.setViewModel(&m_navVm);
	
	// 连接导航选择变化信号
	connect(&m_navVm, &NavViewModel::selectedIndexChanged, this, &MainOpenGlWindow::onNavSelectionChanged);
	
	// 将导航组件添加到根容器
	m_uiRoot.addChild(&m_nav);
}

void MainOpenGlWindow::initializePages()
{
	// 初始化页面路由器
	m_pageRouter.addPage(0, std::make_unique<HomePage>());
	m_pageRouter.addPage(1, std::make_unique<DataPage>());
	m_pageRouter.addPage(2, std::make_unique<FavoritesPage>());
	m_pageRouter.addPage(3, std::make_unique<ExplorePage>());
	m_pageRouter.addPage(4, std::make_unique<SettingsPage>());
	
	// 设置当前页面
	m_pageRouter.setCurrentPageIndex(m_navVm.selectedIndex());
	
	// 将页面路由器添加到根容器
	m_uiRoot.addChild(&m_pageRouter);
}

void MainOpenGlWindow::initializeDeclarativeShell()
{
	// 使用声明式Shell进行初始化
	// 这里需要根据实际的声明式Shell实现来调整
	// 暂时保留基本的初始化逻辑
}

void MainOpenGlWindow::setupThemeListeners()
{
	if (m_themeMgr) {
		connect(m_themeMgr.get(), &ThemeManager::effectiveSchemeChanged,
			this, [this](Qt::ColorScheme scheme) {
				setTheme(schemeToTheme(scheme));
			});
		
		connect(m_themeMgr.get(), &ThemeManager::modeChanged,
			this, [this](ThemeManager::ThemeMode mode) {
				// 处理主题模式变化
				update(); // 触发重绘
			});
	}
}

void MainOpenGlWindow::applyTheme()
{
	// 应用主题到各个组件
	const bool isDark = (theme() == Theme::Dark);
	m_nav.onThemeChanged(isDark);
}

void MainOpenGlWindow::onNavSelectionChanged(int index)
{
	// 处理导航选择变化
	m_pageRouter.setCurrentPageIndex(index);
	update();
}

void MainOpenGlWindow::onThemeToggle() const
{
	if (m_themeMgr) {
		const auto currentMode = m_themeMgr->mode();
		const auto newMode = (currentMode == ThemeManager::ThemeMode::Dark) ?
			ThemeManager::ThemeMode::Light : ThemeManager::ThemeMode::Dark;
		m_themeMgr->setMode(newMode);
	}
}

void MainOpenGlWindow::onFollowSystemToggle() const
{
	if (m_themeMgr) {
		m_themeMgr->setFollowSystem(!m_themeMgr->followSystem());
	}
}

#include "MainOpenGlWindow.moc"