#pragma once
#include <memory>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qopenglwindow.h>
#include <qtimer.h>

#include "IconLoader.h"
#include "NavViewModel.h"
#include "PageRouter.h"
#include "Renderer.h"
#include "ThemeManager.h"
#include "UiNav.h"
#include "UiRoot.h"
#include "UiTopBar.h"

#ifdef Q_OS_WIN
class WinWindowChrome;
#endif
#include <qsystemdetection.h>
#include <qrect.h>

// 前向声明
class AppConfig;

// 简化后的主窗口
class MainOpenGlWindow final : public QOpenGLWindow, protected QOpenGLFunctions
{
public:
	enum class Theme { Light, Dark };

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

	// Windows Chrome 命中测试辅助
	QRect navBounds() const { return m_nav.bounds(); }
	QRect topBarBounds() const { return m_topBar.bounds(); }

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseDoubleClickEvent(QMouseEvent* e) override;

private:
	// 初始化
	void initializeNavigation();
	void initializePages();
	void initializeTopBar();
	void setupThemeListeners();

	// 布局和渲染
	void updateLayout();
	void applyTheme();

	// 事件处理
	void onNavSelectionChanged(int index);
	void onThemeToggle() const;
	void onFollowSystemToggle() const;
	void onAnimationTick();

private:
	// 主题
	Theme m_theme{ Theme::Dark };
	QColor m_clearColor;

	// 服务
	std::shared_ptr<ThemeManager> m_themeMgr;
	std::shared_ptr<AppConfig> m_config;

	// 数据模型
	NavViewModel m_navVm;

	// UI组件
	Ui::NavRail m_nav;
	UiTopBar m_topBar;
	UiRoot m_uiRoot;

	// 页面管理
	PageRouter m_pageRouter;

	// 渲染
	Renderer m_renderer;
	IconLoader m_iconLoader;
	int m_fbWpx{ 0 };
	int m_fbHpx{ 0 };

	// 动画
	QTimer m_animTimer;
	QElapsedTimer m_animClock;

#ifdef Q_OS_WIN
	WinWindowChrome* m_winChrome{ nullptr };
#endif
};