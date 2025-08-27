#pragma once
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiButton.hpp"
#include "UiComponent.hpp"

#include <cstdint>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qrect.h>
#include <qstring.h>
#include <qtypes.h>

// 右上角按钮（主题/跟随 + 三大键）
class UiTopBar final : public IUiComponent
{
public:
	struct Palette {
		QColor bg, bgHover, bgPressed, icon;
	};

	UiTopBar();
	~UiTopBar() override = default;

	void setDarkTheme(bool dark);
	bool isDarkTheme() const noexcept { return m_dark; }

	void setFollowSystem(bool on, bool animate = true);
	bool followSystem() const noexcept { return m_followSystem; }

	void setPalette(const Palette& p);
	void setCornerRadius(float r);

	void setSvgPaths(QString themeWhenDark, QString themeWhenLight, QString followOn, QString followOff);

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

	void onThemeChanged(bool isDark) override {
		setDarkTheme(isDark);
		setPalette(isDark ? getDarkPalette() : getLightPalette());
	}

	QRect bounds() const override { return m_bounds; }

	bool takeActions(bool& clickedTheme, bool& clickedFollow)
	{
		clickedTheme = m_clickThemePending;
		clickedFollow = m_clickFollowPending;
		const bool any = clickedTheme || clickedFollow;
		m_clickThemePending = m_clickFollowPending = false;
		return any;
	}

	bool takeSystemActions(bool& clickedMin, bool& clickedMaxRestore, bool& clickedClose)
	{
		clickedMin = m_clickMinPending;
		clickedMaxRestore = m_clickMaxPending;
		clickedClose = m_clickClosePending;
		const bool any = clickedMin || clickedMaxRestore || clickedClose;
		m_clickMinPending = m_clickMaxPending = m_clickClosePending = false;
		return any;
	}

	bool themeInteractive() const;

	QRect themeButtonRect() const { return m_btnTheme.baseRect(); }
	QRect followButtonRect() const { return m_btnFollow.baseRect(); }

	QRect sysMinRect() const { return m_btnMin.baseRect(); }
	QRect sysMaxRect() const { return m_btnMax.baseRect(); }
	QRect sysCloseRect() const { return m_btnClose.baseRect(); }

private:
	enum class AnimPhase : uint8_t { Idle, HideTheme_FadeOut, MoveFollow_Right, MoveFollow_Left, ShowTheme_FadeIn };
	static float easeInOut(float t);
	static float lerp(float a, float b, float t) { return a + (b - a) * t; }

	void startAnimSequence(bool followOn);
	void beginPhase(AnimPhase ph, int durationMs);

	static Palette getDarkPalette() {
		return Palette{
			.bg = QColor(52,63,76,120),
			.bgHover = QColor(66,78,92,200),
			.bgPressed = QColor(58,70,84,220),
			.icon = QColor(255,255,255,255)
		};
	}

	static Palette getLightPalette() {
		return Palette{
			.bg = QColor(240,243,247,200),
			.bgHover = QColor(232,237,242,220),
			.bgPressed = QColor(225,230,236,230),
			.icon = QColor(60,64,72,255)
		};
	}

private:
	// 状态
	bool  m_dark{ true };
	bool  m_followSystem{ false };

	// 按钮
	Ui::Button m_btnTheme;
	Ui::Button m_btnFollow;
	Ui::Button m_btnMin;
	Ui::Button m_btnMax;
	Ui::Button m_btnClose;

	QRect m_bounds;

	// 动画状态
	AnimPhase     m_animPhase{ AnimPhase::Idle };
	int           m_animDurationMs{ 0 };
	qint64        m_phaseStartMs{ 0 };
	QElapsedTimer m_animClock;

	float m_themeAlpha{ 1.0f };
	float m_followSlide{ 0.0f };
	float m_phaseStartAlpha{ 1.0f };
	float m_phaseStartSlide{ 0.0f };

	// 图标路径
	QString m_svgThemeWhenDark{ ":/icons/sun.svg" };
	QString m_svgThemeWhenLight{ ":/icons/moon.svg" };
	QString m_svgFollowOn{ ":/icons/follow_on.svg" };
	QString m_svgFollowOff{ ":/icons/follow_off.svg" };

	QString m_svgSysMin{ ":/icons/sys_min.svg" };
	QString m_svgSysMax{ ":/icons/sys_max.svg" };
	QString m_svgSysClose{ ":/icons/sys_close.svg" };

	// 资源上下文
	IconLoader* m_loader{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	// 点击动作
	bool m_clickThemePending{ false };
	bool m_clickFollowPending{ false };
	bool m_clickMinPending{ false };
	bool m_clickMaxPending{ false };
	bool m_clickClosePending{ false };
};