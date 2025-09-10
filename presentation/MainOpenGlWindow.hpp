/*
 * 文件名：MainOpenGlWindow.hpp
 * 职责：主窗口类，集成OpenGL渲染、UI组件管理、页面路由、主题切换和动画驱动。
 * 依赖：基础Window类、自绘UI框架、渲染器、图标缓存、主题管理器。
 * 线程：仅在UI线程使用，所有OpenGL操作在当前上下文中执行。
 * 备注：通过依赖注入接收配置和主题管理器，避免全局状态耦合。继承自基础Window类。
 */

#pragma once
#include "ui/base/Window.hpp"
#include <memory>
#include <qcolor.h>

#include "CurrentPageHost.h"
#include "infrastructure/gfx/IconCache.h"
#include "presentation/viewmodels/NavViewModel.h"
#include "presentation/ui/containers/PageRouter.h"
#include "infrastructure/gfx/Renderer.h"
#include "presentation/viewmodels/ThemeManager.h"
#include "presentation/ui/widgets/UiNav.h"
#include "presentation/ui/containers/UiRoot.h"
#include "presentation/ui/widgets/UiTopBar.h"

#ifdef Q_OS_WIN
class WinWindowChrome;
#endif
#include <qsystemdetection.h>
#include <qrect.h>
#include <algorithm>
#include <qevent.h>
#include <presentation/ui/declarative/Binding.h>
#include <presentation/ui/declarative/RebuildHost.h>

// 前向声明
class AppConfig;

namespace fj::presentation::binding {
	class INavDataProvider;
}

/// 主窗口：基于OpenGL的自绘UI应用程序窗口（继承自基础Window类）
/// 
/// 功能：
/// - UI组件层次结构与事件分发
/// - 页面路由与导航状态管理  
/// - 主题模式切换与传播
/// 
/// 生命周期：
/// 1. 构造时注入依赖服务（配置、主题管理器）
/// 2. initializeWindowGL()中初始化渲染器和UI组件
/// 3. updateWindowLayout()中更新视口和布局
/// 4. renderWindow()中执行帧渲染
/// 5. 析构时清理OpenGL资源
class MainOpenGlWindow final : public fj::presentation::ui::base::Window
{
    Q_OBJECT

public:
	/// 构造函数：注入核心依赖服务
	/// 参数：config — 应用配置管理器（窗口几何、主题设置等）
	/// 参数：themeManager — 主题管理器（模式切换、系统主题监听）
	/// 参数：updateBehavior — Qt窗口更新行为控制
	explicit MainOpenGlWindow(
		std::shared_ptr<AppConfig> config,
		std::shared_ptr<ThemeManager> themeManager,
		UpdateBehavior updateBehavior = NoPartialUpdate);
	~MainOpenGlWindow() override;

	void setFollowSystem(bool on) const;
	bool followSystem() const noexcept;

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
	
	// 重写基类的窗口几何管理
	void saveWindowGeometry() override;
	void restoreWindowGeometry() override;

protected:
	/// 基类Window虚函数实现

	/// 功能：初始化窗口特定的OpenGL资源
	void initializeWindowGL() override;
	
	/// 功能：更新窗口布局
	/// 参数：w, h — 新的窗口尺寸
	void updateWindowLayout(int w, int h) override;
	
	/// 功能：渲染窗口内容
	void renderWindow() override;
	
	/// 功能：处理主题变化
	/// 参数：newTheme — 新的主题模式
	void onThemeChanged(Theme newTheme) override;

	/// 功能：处理动画帧更新
	/// 返回：是否需要继续动画
	bool onAnimationTick() override;

	/// 事件处理重写：转发到UI组件层次结构
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
	void applyTheme();

	// 事件处理回调
	void onNavSelectionChanged(int index);
	void onThemeToggle() const;
	void onFollowSystemToggle() const;

private:
	bool m_animateFollowChange{ false };

	// 依赖服务（注入）
	std::shared_ptr<ThemeManager> m_themeMgr;
	std::shared_ptr<AppConfig> m_config;

	// 数据模型
	NavViewModel m_navVm;

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

#ifdef Q_OS_WIN
	WinWindowChrome* m_winChrome{ nullptr };  // Windows平台自定义标题栏
#endif
};