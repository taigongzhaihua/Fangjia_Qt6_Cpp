#pragma once
#include <cstdint>
#include <memory>

#include "IconLoader.h"
#include "NavViewModel.h"
#include "RenderData.hpp"
#include "Renderer.h"
#include "TabViewModel.h"
#include "ThemeManager.h"
#include "UiFormulaView.h"
#include "UiNav.h"
#include "UiPage.h"
#include "UiRoot.h"
#include "UiTabView.h"
#include "UiTopBar.h"

#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qevent.h>
#include <qopenglfunctions.h>
#include <qopenglwindow.h>
#include <qstring.h>
#include <qtimer.h>

#ifdef Q_OS_WIN
class WinWindowChrome;
#endif
#include <qsystemdetection.h>
#include <qrect.h>

// 前向声明
class AppConfig;

// 主窗口：左侧导航 + 右上角两个图标按钮 + 主内容页面
class MainOpenGlWindow final : public QOpenGLWindow, protected QOpenGLFunctions
{
public:
	enum class Theme : uint8_t { Light, Dark };

public:
	explicit MainOpenGlWindow(UpdateBehavior updateBehavior = NoPartialUpdate);
	~MainOpenGlWindow() override;

	void setTheme(Theme t);
	Theme theme() const noexcept { return m_theme; }

	void setFollowSystem(bool on);
	bool followSystem() const noexcept {
		return m_themeMgr && m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem;
	}

	void submitFrame(const Render::FrameData& fd, bool scheduleUpdate = true);

	// 命中测试辅助（Windows 下自定义 Chrome 使用）
	QRect navBounds() const { return m_nav.bounds(); }
	QRect topBarThemeRect() const { return m_topBar.themeButtonRect(); }
	QRect topBarFollowRect() const { return m_topBar.followButtonRect(); }
	QRect topBarSysMinRect() const { return m_topBar.sysMinRect(); }
	QRect topBarSysMaxRect() const { return m_topBar.sysMaxRect(); }
	QRect topBarSysCloseRect() const { return m_topBar.sysCloseRect(); }

protected:
	void initializeGL() override;
	void resizeGL(int w, int h) override;
	void paintGL() override;

	void mousePressEvent(QMouseEvent* e) override;
	void mouseMoveEvent(QMouseEvent* e) override;
	void mouseReleaseEvent(QMouseEvent* e) override;
	void mouseDoubleClickEvent(QMouseEvent* e) override;

private:
	void updateLayout();
	void updateTitle();
	void toggleTheme();
	void toggleFollowSystem();
	void toggleNavExpanded();
	void applyThemeColors();

	void appendUiOverlay(Render::FrameData& fd) const;
	void onAnimTick();

	void applyTopBarPalette();
	void applyNavPalette();
	void applyPagePalette();
	void applyTabViewPalette();

	// 根据导航选中项更新页面标题与内容
	void updatePageFromSelection(int idx);

	bool isDataPageIndex(int idx) const;

private:
	Theme m_theme{ Theme::Dark };
	QColor m_clearColor{ 25, 38, 51 };

	// 从 DI 获取的服务
	std::shared_ptr<ThemeManager> m_themeMgr;
	std::shared_ptr<AppConfig> m_config;

	// 本地实例（如果 DI 没有提供）
	std::shared_ptr<ThemeManager> m_localThemeMgr;

	int m_fbWpx{ 0 };
	int m_fbHpx{ 0 };

	// 启动阶段标记：用于抑制首次动画
	bool m_booting{ true };

	// 导航 VM
	NavViewModel m_navVm;

	// 页面（主内容）
	UiPage m_page;
	TabViewModel m_dataTabsVm;  // 新增：数据页 TabViewModel
	UiTabView m_dataTabView;    // 新增：通用 TabView
	std::unique_ptr<UiFormulaView> m_formulaView;

	// 导航视图
	Ui::NavRail m_nav;
	int m_navCollapsedW{ 48 };
	int m_navExpandedW{ 220 };

	UiTopBar m_topBar;
	UiRoot   m_uiRoot;

	Renderer  m_renderer;
	IconLoader m_iconLoader;

	Render::DataBus   m_renderBus;
	Render::FrameData m_baseFrameData;

	QElapsedTimer m_animClock;
	QTimer        m_animTimer;

	QString m_svgThemeWhenDark{ ":/icons/sun.svg" };
	QString m_svgThemeWhenLight{ ":/icons/moon.svg" };
	QString m_svgFollowOn{ ":/icons/follow_on.svg" };
	QString m_svgFollowOff{ ":/icons/follow_off.svg" };

#ifdef Q_OS_WIN
	WinWindowChrome* m_winChrome{ nullptr };
#endif
};