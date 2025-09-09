#include "MainOpenGlWindow.h"

#include "AppConfig.h"
#include "CurrentPageHost.h"
#include "DataPage.h"
#include "ExplorePage.h"
#include "FavoritesPage.h"
#include "HomePage.h"
#include "SettingsPage.h"
#include "ThemeManager.h"
#include "DatabaseBootstrapper.h"

#ifdef Q_OS_WIN
#include "WinWindowChrome.h"
#endif
#include <qsystemdetection.h>
#include <qtimer.h>
#include <qcolor.h>
#include <qopenglwindow.h>
#include <RenderData.hpp>
#include <UiNav.h>
#include <UiTopBar.h>
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
#include <Binding.h>
#include <ComponentWrapper.h>
#include <RebuildHost.h>
#include <UI.h>
#include <Widget.h>
#include <qpoint.h>
#include <NavViewModel.h>

namespace
{
	MainOpenGlWindow::Theme schemeToTheme(const Qt::ColorScheme s)
	{
		return s == Qt::ColorScheme::Dark ? MainOpenGlWindow::Theme::Dark : MainOpenGlWindow::Theme::Light;
	}

	QByteArray saveWindowGeometry(const QWindow* window)
	{
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

MainOpenGlWindow::MainOpenGlWindow(
	std::shared_ptr<AppConfig> config,
	std::shared_ptr<ThemeManager> themeManager,
	const UpdateBehavior updateBehavior)
	: QOpenGLWindow(updateBehavior), m_themeMgr(std::move(themeManager)), m_config(std::move(config))
{
	try
	{
		qDebug() << "MainOpenGlWindow constructor start";

		// Bootstrap the database during app initialization
		Data::DatabaseBootstrapper::initialize();

		// 设置动画定时器
		connect(&m_animTimer, &QTimer::timeout, this, &MainOpenGlWindow::onAnimationTick);
		m_animTimer.setTimerType(Qt::PreciseTimer);
		m_animTimer.setInterval(16);
		m_animClock.start();

		qDebug() << "MainOpenGlWindow constructor end";
	}
	catch (const std::exception& e)
	{
		qCritical() << "Exception in MainOpenGlWindow constructor:" << e.what();
		throw;
	}
}

MainOpenGlWindow::~MainOpenGlWindow()
{
	try
	{
		// 保存窗口状态
		if (m_config)
		{
			m_config->setWindowGeometry(saveWindowGeometry(this));
			m_config->setNavSelectedIndex(m_navVm.selectedIndex());
			m_config->setNavExpanded(m_navVm.expanded());
			m_config->save();
		}

#ifdef Q_OS_WIN
		if (m_winChrome)
		{
			m_winChrome->detach();
			m_winChrome = nullptr;
		}
#endif

		// Critical fix: Add context validity check before OpenGL cleanup
		if (context() && context()->isValid())
		{
			makeCurrent();
			
			// Additional safety check before OpenGL operations
			if (QOpenGLContext::currentContext())
			{
				m_iconCache.releaseAll(this);
				m_renderer.releaseGL();
			}
			
			doneCurrent();
		}
	}
	catch (const std::exception& e)
	{
		qCritical() << "Exception in MainOpenGlWindow destructor:" << e.what();
	}
}

void MainOpenGlWindow::initializeGL()
{
	try
	{
		qDebug() << "MainOpenGlWindow::initializeGL start";

		// Critical fix: Add context validity check before OpenGL operations
		if (!context() || !context()->isValid())
		{
			qCritical() << "OpenGL context is invalid in initializeGL";
			return;
		}

		// Initialize OpenGL functions with error checking
		if (!initializeOpenGLFunctions())
		{
			qCritical() << "Failed to initialize OpenGL functions";
			return;
		}

		// Check for OpenGL errors after function initialization
		GLenum glError = glGetError();
		if (glError != GL_NO_ERROR)
		{
			qWarning() << "OpenGL error after initializeOpenGLFunctions:" << glError;
		}

		glEnable(GL_BLEND);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// Check for errors after basic OpenGL setup
		glError = glGetError();
		if (glError != GL_NO_ERROR)
		{
			qWarning() << "OpenGL error after blend setup:" << glError;
		}

		// Critical fix: Enable OpenGL debug output in debug builds for NVIDIA driver issue diagnosis
		#ifdef _DEBUG
		if (context()->hasExtension("GL_KHR_debug"))
		{
			qDebug() << "OpenGL debug context available, enabling debug output";
			// Note: Full debug callback setup would require additional OpenGL function loading
			// For now, we rely on manual error checking throughout the code
		}
		#endif

		m_renderer.initializeGL(this);

#ifdef Q_OS_WIN
		if (!m_winChrome)
		{
			qDebug() << "Attaching WinWindowChrome...";
			m_winChrome = WinWindowChrome::attach(this, 56, [this]
				{
					QVector<QRect> excludeRects;
					excludeRects.push_back(navBounds());
					excludeRects.push_back(topBarBounds());
					return excludeRects;
				});
		}
#endif

		// 先确定初始主题
		if (m_themeMgr)
		{
			m_theme = schemeToTheme(m_themeMgr->effectiveColorScheme());
		}
		else
		{
			m_theme = Theme::Light; // 默认浅色
		}

		// 设置清屏颜色
		if (m_theme == Theme::Dark)
		{
			m_clearColor = QColor::fromRgbF(0.05f, 0.10f, 0.15f);
		}
		else
		{
			m_clearColor = QColor::fromRgbF(0.91f, 0.92f, 0.94f);
		}

		qDebug() << "Initializing navigation...";
		initializeNavigation();

		qDebug() << "Initializing pages...";
		initializePages();

		// qDebug() << "Initializing top bar...";
		// initializeTopBar(); // 不再需要：现在使用声明式TopBar

		qDebug() << "Initializing declarative shell...";
		initializeDeclarativeShell();

		// 在所有组件添加后，应用初始主题
		const bool isDark = (m_theme == Theme::Dark);
		m_uiRoot.propagateThemeChange(isDark);

		updateLayout();

		// 设置主题监听
		setupThemeListeners();

		qDebug() << "MainOpenGlWindow::initializeGL end";
	}
	catch (const std::exception& e)
	{
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
	// Critical fix: Add context validity check before OpenGL operations
	if (!context() || !context()->isValid() || !QOpenGLContext::currentContext())
	{
		qWarning() << "Invalid OpenGL context in paintGL";
		return;
	}

	glClearColor(m_clearColor.redF(), m_clearColor.greenF(), m_clearColor.blueF(), 1.0f);
	glClear(GL_COLOR_BUFFER_BIT);

	Render::FrameData frameData;
	m_uiRoot.append(frameData);
	m_renderer.drawFrame(frameData, m_iconCache, static_cast<float>(devicePixelRatio()));
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
	if (e->button() == Qt::LeftButton)
	{
		const bool handled = m_uiRoot.onMouseRelease(e->pos());

		if (handled)
		{
			// 声明式TopBar现在通过回调处理系统按钮，无需手动检查
			// 旧的 m_topBar.takeActions() 和 m_topBar.takeSystemActions() 调用已移除

			if (!m_animTimer.isActive())
			{
				m_animClock.start();
				m_animTimer.start();
			}
		}

		// Always schedule a redraw on left-button release to ensure VM-driven rebuilds are rendered
		update();

		if (handled)
		{
			e->accept();
			return;
		}
	}
	QOpenGLWindow::mouseReleaseEvent(e);
}

void MainOpenGlWindow::mouseDoubleClickEvent(QMouseEvent* e)
{
	if (e->button() == Qt::LeftButton)
	{
		if (m_nav.bounds().contains(e->pos()))
		{
			m_navVm.toggleExpanded();
			updateLayout();
			if (!m_animTimer.isActive())
			{
				m_animClock.start();
				m_animTimer.start();
			}
			e->accept();
			return;
		}
	}
	QOpenGLWindow::mouseDoubleClickEvent(e);
}

void MainOpenGlWindow::wheelEvent(QWheelEvent* e)
{
	// 将 QWheelEvent 的位置与 angleDelta 传给 UiRoot

	if (m_uiRoot.onWheel(e->position().toPoint(), e->angleDelta()))
	{
		// 如有消费则启动动画计时器并重绘
		if (!m_animTimer.isActive())
		{
			m_animClock.start();
			m_animTimer.start();
		}
		update();
		e->accept();
	}
	else
	{
		QOpenGLWindow::wheelEvent(e);
	}
}

void MainOpenGlWindow::keyPressEvent(QKeyEvent* e)
{
	// 将键盘按下事件转发到UI组件层次结构

	if (m_uiRoot.onKeyPress(e->key(), e->modifiers()))
	{
		// 如有消费则启动动画计时器并重绘
		if (!m_animTimer.isActive())
		{
			m_animClock.start();
			m_animTimer.start();
		}
		update();
		e->accept();
	}
	else
	{
		QOpenGLWindow::keyPressEvent(e);
	}
}

void MainOpenGlWindow::keyReleaseEvent(QKeyEvent* e)
{
	// 将键盘释放事件转发到UI组件层次结构

	if (m_uiRoot.onKeyRelease(e->key(), e->modifiers()))
	{
		// 如有消费则启动动画计时器并重绘
		if (!m_animTimer.isActive())
		{
			m_animClock.start();
			m_animTimer.start();
		}
		update();
		e->accept();
	}
	else
	{
		QOpenGLWindow::keyReleaseEvent(e);
	}
}

void MainOpenGlWindow::initializeNavigation()
{
	// 设置导航视图（仅配置UI视觉属性）
	if (m_config)
	{
		if (const int savedIndex = m_config->navSelectedIndex(); savedIndex >= 0 && savedIndex < m_navVm.count())
		{
			m_navVm.setSelectedIndex(savedIndex);
		}
		else
		{
			m_navVm.setSelectedIndex(0);
		}
		m_navVm.setExpanded(m_config->navExpanded());
	}

	// 设置导航视图
	m_nav.setDataProvider(&m_navVm);
	m_nav.setIconLogicalSize(22);
	m_nav.setItemHeight(48);
	m_nav.setLabelFontPx(13);
	m_nav.setWidths(48, 200);

	// 如果已有数据提供者，则配置连接


		// 连接导航选择变化
	connect(&m_navVm, &NavViewModel::selectedIndexChanged, this, &MainOpenGlWindow::onNavSelectionChanged);

	// 连接导航状态变化到配置保存
	if (m_config)
	{
		connect(&m_navVm, &NavViewModel::expandedChanged, m_config.get(), [this](const bool expanded)
			{
				m_config->setNavExpanded(expanded);
				m_config->save();
			});

		connect(&m_navVm, &NavViewModel::selectedIndexChanged, m_config.get(), [this](const int index)
			{
				m_config->setNavSelectedIndex(index);
				m_config->save();
			});
	}
}

void MainOpenGlWindow::initializePages()
{
	try
	{
		// 注册页面工厂（支持懒加载）
		m_pageRouter.registerPage("home", [this] { 
			HomePage::setWindowContext(this);
			return std::make_unique<HomePage>(); 
		});
		m_pageRouter.registerPage("data", [this] { return std::make_unique<DataPage>(m_config.get()); });
		m_pageRouter.registerPage("explore", [] { return std::make_unique<ExplorePage>(); });
		m_pageRouter.registerPage("favorites", [] { return std::make_unique<FavoritesPage>(); });
		m_pageRouter.registerPage("settings", [] { return std::make_unique<SettingsPage>(); });

		// 切换到初始页面（基于注入的导航数据提供者）
		const auto& items = m_navVm.itemsInternal();
		if (m_navVm.selectedIndex() >= 0 && m_navVm.selectedIndex() < items.size())
		{
			m_pageRouter.switchToPage(items[m_navVm.selectedIndex()].id);
		}
	}
	catch (const std::exception& e)
	{
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
	m_topBar.setFollowSystem(followSystem, false); // false = 无动画
}

void MainOpenGlWindow::setupThemeListeners()
{
	// 监听主题变化
	if (m_themeMgr)
	{
		connect(m_themeMgr.get(), &ThemeManager::effectiveColorSchemeChanged, this,
			[this](const Qt::ColorScheme s)
			{
				setTheme(schemeToTheme(s));
			});

		connect(m_themeMgr.get(), &ThemeManager::modeChanged, this,
			[this](const ThemeManager::ThemeMode mode)
			{
				// Defer rebuild to the next event loop turn to avoid re-entrant destruction
				QTimer::singleShot(0, this, [this]() {
					// 声明式TopBar会在Shell重建时自动获取最新的followSystem状态
					// 触发Shell重建以更新TopBar的followSystem状态
					if (m_shellRebuildHost)
					{
						m_shellRebuildHost->requestRebuild();
						// Ensure animation timer is running if a follow animation is expected
						if (m_animateFollowChange && !m_animTimer.isActive()) {
							m_animClock.start();
							m_animTimer.start();
						}
						// Clear the animation intent after a short delay to avoid racing the rebuild
						QTimer::singleShot(300, [this]() {
							m_animateFollowChange = false;
							});
					}
					updateLayout();
					update();
					});
			});
	}
}

void MainOpenGlWindow::updateLayout()
{
	const QSize winSize = size();

	// 声明式模式：让AppShell/CurrentPageHost处理页面视口，无需手动设置
	// 更新所有组件布局
	m_uiRoot.updateLayout(winSize);
	m_uiRoot.updateResourceContext(m_iconCache, this, static_cast<float>(devicePixelRatio()));

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
	if (isDark)
	{
		m_clearColor = QColor::fromRgbF(0.05f, 0.10f, 0.15f);
	}
	else
	{
		m_clearColor = QColor::fromRgbF(0.91f, 0.92f, 0.94f);
	}

	// 通过UiRoot传播主题变化到所有组件
	m_uiRoot.propagateThemeChange(isDark);

	// 更新资源上下文（图标可能需要重新加载）
	m_uiRoot.updateResourceContext(m_iconCache, this, static_cast<float>(devicePixelRatio()));

	update();
}

bool MainOpenGlWindow::followSystem() const noexcept
{
	return m_themeMgr && m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem;
}

void MainOpenGlWindow::setFollowSystem(const bool on) const
{
	if (!m_themeMgr) return;

	if (on)
	{
		m_themeMgr->setMode(ThemeManager::ThemeMode::FollowSystem);
	}
	else
	{
		const Theme cur = schemeToTheme(m_themeMgr->effectiveColorScheme());
		m_themeMgr->setMode(cur == Theme::Dark ? ThemeManager::ThemeMode::Dark : ThemeManager::ThemeMode::Light);
	}
}

void MainOpenGlWindow::onNavSelectionChanged(const int index)
{
	const auto& items = m_navVm.itemsInternal();
	if (index >= 0 && index < items.size())
	{
		const QString pageId = items[index].id;

		// 声明式模式：仅切换页面，AppShell会通过重建自动更新UI
		// CurrentPageHost负责处理视口设置，UiRoot负责主题传播和资源上下文更新
		m_pageRouter.switchToPage(pageId);

		// 可选：请求重建以确保UI更新
		if (m_shellRebuildHost)
		{
			m_shellRebuildHost->requestRebuild();
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

	// Set animation flag before changing theme mode so the next rebuild knows to animate
	const_cast<MainOpenGlWindow*>(this)->m_animateFollowChange = true;

	setFollowSystem(m_themeMgr->mode() != ThemeManager::ThemeMode::FollowSystem);

	// Defer rebuild and animation to the next event loop turn to avoid re-entrant destruction
	auto* self = const_cast<MainOpenGlWindow*>(this);
	QTimer::singleShot(0, self, [self]() {
		// Proactively rebuild and kick animation so the TopBar can start animating immediately
		if (self->m_shellRebuildHost) {
			self->m_shellRebuildHost->requestRebuild();
		}
		if (!self->m_animTimer.isActive()) {
			self->m_animClock.start();
			self->m_animTimer.start();
		}
		self->update();
		});
}

void MainOpenGlWindow::onAnimationTick()
{
	const bool hasAnimation = m_uiRoot.tick();

	if (m_nav.hasActiveAnimation())
	{
		updateLayout();

		// 如果导航栏有动画，请求重建以保持列宽同步
		if (m_shellRebuildHost)
		{
			m_shellRebuildHost->requestRebuild();
		}
	}

	if (!hasAnimation)
	{
		m_animTimer.stop();
	}

	update();
}

void MainOpenGlWindow::initializeDeclarativeShell()
{
	// 创建页面宿主适配器
	m_pageHost = std::make_unique<CurrentPageHost>(m_pageRouter);

	// 创建包装整个Shell的BindingHost，这样可以在动画期间重建整个布局
	m_shellHost = bindingHost([this]() -> WidgetPtr
		{
			const bool animateNow = m_animateFollowChange;

			// 确定跟随系统状态
			const bool followSystem = m_themeMgr && m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem;

			// Shell构建器：每次重建时都创建新的AppShell布局
			return appShell()
				->nav(wrap(&m_nav))
				->topBar(UI::topBar()
					->followSystem(followSystem, animateNow) // 使用稳定的动画标志
					->cornerRadius(8.0f)
					->svgTheme(":/icons/sun.svg", ":/icons/moon.svg")
					->svgFollow(":/icons/follow_on.svg", ":/icons/follow_off.svg")
					->svgSystem(":/icons/sys_min.svg", ":/icons/sys_max.svg", ":/icons/sys_close.svg")
					->onThemeToggle([this]() { onThemeToggle(); })
					->onFollowToggle([this]() { onFollowSystemToggle(); })
					->onMinimize([this]() { showMinimized(); })
					->onMaxRestore([this]() {
						if (visibility() == Maximized) showNormal();
						else showMaximized();
						})
					->onClose([this]() { close(); })
				)
				->content([this]() -> WidgetPtr
					{
						// 内容构建器：总是返回当前页面宿主
						return wrap(m_pageHost.get());
					})
				->navWidthProvider([this]
					{
						// 导航栏宽度提供器：反映运行时动画状态
						return m_nav.currentWidth();
					})
				->topBarHeight(52) // 固定顶栏高度
				->connect([this](RebuildHost* host)
					{
						// 观察导航选择变化（展开/收缩由动画tick处理）
						observe(&m_navVm, &NavViewModel::selectedIndexChanged, [host](int)
							{
								host->requestRebuild();
							});
					});
		})
		// 添加观察导航展开状态变化的连接器（用于非动画的立即变化）
		->connect([this](RebuildHost* host)
			{
				// 保存RebuildHost引用以便在动画期间使用
				m_shellRebuildHost = host;

				observe(&m_navVm, &NavViewModel::expandedChanged, [host](bool)
					{
						host->requestRebuild();
					});
			});

	// 将Shell BindingHost添加到UiRoot
	auto shellComponent = m_shellHost->build();
	m_uiRoot.add(shellComponent.release()); // 转移所有权给UiRoot
}

// 计算右上角系统按钮集的矩形（与 UiTopBar 的布局常量保持一致）
QRect MainOpenGlWindow::topBarSystemButtonsRect() const {
	// 与 UiTopBar::updateLayout 常量保持一致
	constexpr int margin = 12;
	constexpr int btnSize = 28;
	constexpr int gap = 8;
	constexpr int count = 5; // follow, theme, min, max, close

	const int clusterW = count * btnSize + (count - 1) * gap; // 5*28 + 4*8 = 172
	const int x = width() - margin - clusterW;
	const int y = margin;           // 顶部对齐
	const int h = btnSize;          // 高度为按钮高度
	return QRect(x, y, clusterW, h);
}

