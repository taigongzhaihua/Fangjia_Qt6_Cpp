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

#include "CurrentPageHost.h"
#include "IconCache.h"
#include "NavViewModel.h"
#include "PageRouter.h"
#include "Renderer.h"
#include "ThemeManager.h"
#include "UiNav.h"
#include "UiRoot.h"
#include "UiTopBar.h"
#include "Window.h"

#ifdef Q_OS_WIN
class WinWindowChrome;
#endif
#include <qsystemdetection.h>
#include <qrect.h>
#include <algorithm>
#include <qevent.h>
#include <Binding.h>
#include <RebuildHost.h>

// 前向声明
class AppConfig;

namespace fj::presentation::binding {
	class INavDataProvider;
}

/// 主窗口：基于OpenGL的自绘UI应用程序窗口
/// 
/// 功能：
/// - 应用级UI组件管理（导航栏、顶栏、页面路由）
/// - 主题模式切换与传播
/// - 页面导航状态管理
/// - 平台特定功能集成（Windows窗口装饰）
/// 
/// 生命周期：
/// 1. 构造时注入依赖服务（配置、主题管理器）
/// 2. initializeUI()中初始化应用UI组件
/// 3. updateLayout()中计算应用布局
/// 4. 继承基类的渲染和事件处理流程
/// 5. 析构时清理应用资源
class MainOpenGlWindow final : public Window
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
	/// 应用UI初始化：设置导航、页面和声明式Shell
	void initializeUI() override;
	
	/// 应用布局更新：计算组件位置和大小
	void updateLayout() override;
	
	/// 应用动画更新：处理应用特定的动画逻辑
	bool onAnimationTick(qint64 deltaTime) override;
	
	/// 获取主题相关的清屏颜色
	QColor getClearColor() const override;
	
	/// 自定义鼠标事件处理：TopBar拖拽功能
	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseDoubleClickEvent(QMouseEvent* e) override;

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
	bool m_animateFollowChange{ false };

	// 依赖服务（注入）
	std::shared_ptr<ThemeManager> m_themeMgr;
	std::shared_ptr<AppConfig> m_config;

	// 数据模型
	NavViewModel m_navVm;

	// UI组件层次结构  
	Ui::NavRail m_nav;
	UiTopBar m_topBar;
	// 注意：UiRoot现在由基类Window管理

	// 声明式Shell支持
	std::unique_ptr<CurrentPageHost> m_pageHost;
	std::shared_ptr<UI::BindingHost> m_shellHost;  // 包装整个Shell的BindingHost
	UI::RebuildHost* m_shellRebuildHost{ nullptr };  // 内部RebuildHost的引用，用于动画期间重建

	// 页面路由管理
	PageRouter m_pageRouter;

	// 注意：Renderer和IconCache现在由基类Window管理

	// 动画状态（应用级）
	bool m_hasActiveAnimation{ false };

#ifdef Q_OS_WIN
	WinWindowChrome* m_winChrome{ nullptr };  // Windows平台自定义标题栏
#endif
};