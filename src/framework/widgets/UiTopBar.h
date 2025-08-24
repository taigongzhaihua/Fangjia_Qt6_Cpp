#pragma once
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiButton.hpp"
#include "UiComponent.hpp"

#include <cstdint>
#include <qbytearray.h>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qhash.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>

// 右上角按钮（主题/跟随 + 三大键）
class UiTopBar final : public IUiComponent
{
public:
	// 调色板
	struct Palette {
		QColor bg, bgHover, bgPressed, icon;
	};

	UiTopBar();
	~UiTopBar() override = default;

	// 状态
	void setDarkTheme(bool dark);
	bool isDarkTheme() const noexcept { return m_dark; }

	// 新增 animate 参数：初始化时可无动画就位
	void setFollowSystem(bool on, bool animate = true);
	bool followSystem() const noexcept { return m_followSystem; }

	// 外观
	void setPalette(const Palette& p);
	void setCornerRadius(float r);

	// 图标资源（路径）
	void setSvgPaths(QString themeWhenDark, QString themeWhenLight, QString followOn, QString followOff);

	// 新增：设置系统三大键图标资源（可不调用，已提供默认值）
	void setSystemButtonSvgPaths(QString sysMin, QString sysMax, QString sysClose) {
		m_svgSysMin = std::move(sysMin);
		m_svgSysMax = std::move(sysMax);
		m_svgSysClose = std::move(sysClose);
	}

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool tick() override;
	QRect bounds() const override { return m_bounds; }

	// 供上层查询一次性动作（释放后清零）
	bool takeActions(bool& clickedTheme, bool& clickedFollow)
	{
		clickedTheme = m_clickThemePending;
		clickedFollow = m_clickFollowPending;
		const bool any = clickedTheme || clickedFollow;
		m_clickThemePending = m_clickFollowPending = false;
		return any;
	}

	// 供上层查询三大键动作（释放后清零）
	bool takeSystemActions(bool& clickedMin, bool& clickedMaxRestore, bool& clickedClose)
	{
		clickedMin = m_clickMinPending;
		clickedMaxRestore = m_clickMaxPending;
		clickedClose = m_clickClosePending;
		const bool any = clickedMin || clickedMaxRestore || clickedClose;
		m_clickMinPending = m_clickMaxPending = m_clickClosePending = false;
		return any;
	}

	// 顶栏按钮可交互判断（根据动画态决定主题按钮是否禁用）
	bool themeInteractive() const;

	// 基础矩形（供上层在需要时查询）
	QRect themeButtonRect() const { return m_btnTheme.baseRect(); }
	QRect followButtonRect() const { return m_btnFollow.baseRect(); }

	// 三大键矩形
	QRect sysMinRect() const { return m_btnMin.baseRect(); }
	QRect sysMaxRect() const { return m_btnMax.baseRect(); }
	QRect sysCloseRect() const { return m_btnClose.baseRect(); }

private:
	enum class AnimPhase : uint8_t { Idle, HideTheme_FadeOut, MoveFollow_Right, MoveFollow_Left, ShowTheme_FadeIn };
	static float easeInOut(float t);
	static float lerp(float a, float b, float t) { return a + (b - a) * t; }

	void startAnimSequence(bool followOn);
	void beginPhase(AnimPhase ph, int durationMs);

	QString iconCacheKey(const QString& baseKey, int logicalPx, float dpr) const;
	QByteArray svgDataCached(const QString& path) const;

private:
	// 状态
	bool  m_dark{ true };
	bool  m_followSystem{ false };

	// 按钮：主题、跟随、三大键
	Ui::Button m_btnTheme;
	Ui::Button m_btnFollow;
	Ui::Button m_btnMin;
	Ui::Button m_btnMax;
	Ui::Button m_btnClose;

	// 布局包围盒
	QRect m_bounds;

	// 动画状态
	AnimPhase     m_animPhase{ AnimPhase::Idle };
	int           m_animDurationMs{ 0 };
	qint64        m_phaseStartMs{ 0 };
	QElapsedTimer m_animClock;

	float m_themeAlpha{ 1.0f };    // 亮暗按钮透明度
	float m_followSlide{ 0.0f };   // 跟随按钮 0..1 插值（左->右）
	float m_phaseStartAlpha{ 1.0f };
	float m_phaseStartSlide{ 0.0f };

	// 图标路径
	QString m_svgThemeWhenDark{ ":/icons/sun.svg" };
	QString m_svgThemeWhenLight{ ":/icons/moon.svg" };
	QString m_svgFollowOn{ ":/icons/follow_on.svg" };
	QString m_svgFollowOff{ ":/icons/follow_off.svg" };

	// 新增：系统三大键 SVG 路径（提供默认资源）
	QString m_svgSysMin{ ":/icons/sys_min.svg" };
	QString m_svgSysMax{ ":/icons/sys_max.svg" };
	QString m_svgSysClose{ ":/icons/sys_close.svg" };

	// 缓存
	mutable QHash<QString, QByteArray> m_svgDataCache;

	// 资源上下文（用于刷新图标绘制器）
	IconLoader* m_loader{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	// 点击动作挂起位
	bool m_clickThemePending{ false };
	bool m_clickFollowPending{ false };
	bool m_clickMinPending{ false };
	bool m_clickMaxPending{ false };
	bool m_clickClosePending{ false };
};