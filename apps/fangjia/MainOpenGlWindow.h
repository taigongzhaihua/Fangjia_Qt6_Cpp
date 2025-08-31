/*
 * 文件名：MainOpenGlWindow.h
 * 职责：主窗口类，集成OpenGL渲染、UI组件管理、页面路由、主题切换和动画驱动。
 * 依赖：Qt6 OpenGL、自绘UI框架、渲染器、图标缓存、主题管理器。
 * 线程：仅在UI线程使用，所有OpenGL操作在当前上下文中执行。
 * 备注：通过依赖注入接收配置和主题管理器，避免全局状态耦合。
 */

#pragma once
#include <memory>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qopenglwindow.h>
#include <qtimer.h>

#include "IconCache.h"
#include "PageRouter.h"
#include "Renderer.h"
#include "ThemeManager.h"
#include "UiNav.h"
#include "UiRoot.h"
#include "UiTopBar.h"
#include "CurrentPageHost.h"
#include "UI.h"

#ifdef Q_OS_WIN
class WinWindowChrome;
#endif
#include <qsystemdetection.h>
#include <qrect.h>

// 前向声明
class AppConfig;

namespace fj::presentation::binding {
	class INavDataProvider;
}

/// 主窗口：基于OpenGL的自绘UI应用程序窗口
/// 
/// 功能：
/// - OpenGL渲染管线与资源管理
/// - UI组件层次结构与事件分发
/// - 页面路由与导航状态管理  
/// - 主题模式切换与传播
/// - 动画循环驱动（60fps目标）
/// 
/// 生命周期：
/// 1. 构造时注入依赖服务（配置、主题管理器）
/// 2. initializeGL()中初始化渲染器和UI组件
/// 3. resizeGL()中更新视口和布局
/// 4. paintGL()中执行帧渲染
/// 5. 析构时清理OpenGL资源
class MainOpenGlWindow final : public QOpenGLWindow, protected QOpenGLFunctions
{
public:
	enum class Theme { Light, Dark };

	/// 构造函数：注入核心依赖服务
	/// 参数：config — 应用配置管理器（窗口几何、主题设置等）
	/// 参数：themeManager — 主题管理器（模式切换、系统主题监听）
	/// 参数：updateBehavior — Qt窗口更新行为控制
	explicit MainOpenGlWindow(
		std::shared_ptr<AppConfig> config,
		std::shared_ptr<ThemeManager> themeManager,
		UpdateBehavior updateBehavior = NoPartialUpdate);
	~MainOpenGlWindow() override;

	// 主题管理
	void setTheme(Theme t);
	Theme theme() const noexcept { return m_theme; }

	void setFollowSystem(bool on) const;
	bool followSystem() const noexcept;

	/// Navigation data provider injection
	/// Sets the navigation data provider for the window to read/observe state from
	/// This should be called before showing the window to ensure proper navigation setup
	void setNavDataProvider(fj::presentation::binding::INavDataProvider* provider);

	/// Windows平台窗口Chrome命中测试辅助
	/// 返回：UI组件的边界矩形，用于自定义标题栏区域判定
	QRect navBounds() const { return m_nav.bounds(); }
	QRect topBarBounds() const { 
		// 计算TopBar区域：右上角，从NavRail右侧到窗口右边，高度为固定的52像素
		const int navWidth = m_nav.currentWidth();
		const int topBarHeight = 52; // 与initializeDeclarativeShell中的topBarHeight一致
		return QRect(navWidth, 0, std::max(0, width() - navWidth), topBarHeight);
	}

	// 系统按钮区域（右侧5个按钮：follow/theme/min/max/close）用于禁用拖拽
	QRect topBarSystemButtonsRect() const;

protected:
	/// OpenGL生命周期回调
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	/// 鼠标事件处理：转发到UI组件层次结构
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseDoubleClickEvent(QMouseEvent* e) override;
	void wheelEvent(QWheelEvent* e) override;

	/// 键盘事件处理：转发到UI组件层次结构
	void keyPressEvent(QKeyEvent* e) override;
	void keyReleaseEvent(QKeyEvent* e) override;

private:
	// 初始化子系统
	void initializeNavigation();
	void initializePages();
	void initializeTopBar();
	void setupThemeListeners();

	// 声明式Shell初始化
	void initializeDeclarativeShell();

	// 布局和渲染
	void updateLayout();
	void applyTheme();

	// 事件处理回调
	void onNavSelectionChanged(int index);
	void onThemeToggle() const;
	void onFollowSystemToggle() const;
	void onAnimationTick();

private:
	// 主题状态
	Theme m_theme{ Theme::Dark };
	QColor m_clearColor;
	bool m_animateFollowChange{ false };

	// 依赖服务（注入）
	std::shared_ptr<ThemeManager> m_themeMgr;
	std::shared_ptr<AppConfig> m_config;

	// Navigation data provider (injected)
	fj::presentation::binding::INavDataProvider* m_navProvider{ nullptr };

	// UI组件层次结构
	Ui::NavRail m_nav;
	UiTopBar m_topBar;
	UiRoot m_uiRoot;

	// 声明式Shell支持
	std::unique_ptr<CurrentPageHost> m_pageHost;
	std::shared_ptr<UI::BindingHost> m_shellHost;  // 包装整个Shell的BindingHost
	UI::RebuildHost* m_shellRebuildHost{ nullptr };  // 内部RebuildHost的引用，用于动画期间重建

	// 页面路由管理
	PageRouter m_pageRouter;

	// 渲染子系统
	Renderer m_renderer;
	IconCache m_iconCache;
	int m_fbWpx{ 0 };    // 帧缓冲宽度（像素）
	int m_fbHpx{ 0 };    // 帧缓冲高度（像素）

	// 动画驱动（目标60fps）
	QTimer m_animTimer;
	QElapsedTimer m_animClock;

#ifdef Q_OS_WIN
	WinWindowChrome* m_winChrome{ nullptr };  // Windows平台自定义标题栏
#endif
};