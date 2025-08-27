#include "UiTopBar.h"

#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiButton.hpp"
#include <algorithm>
#include <cmath>
#include <qbytearray.h>
#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>
#include <utility>
#include <RenderUtils.hpp>

UiTopBar::UiTopBar()
{
	m_btnTheme.setCornerRadius(6.0f);
	m_btnFollow.setCornerRadius(6.0f);
	m_btnMin.setCornerRadius(6.0f);
	m_btnMax.setCornerRadius(6.0f);
	m_btnClose.setCornerRadius(6.0f);
}

void UiTopBar::setDarkTheme(const bool dark)
{
	if (m_dark == dark) return;
	m_dark = dark;
	// 主题变更时，由 updateResourceContext 刷新图标
}

void UiTopBar::setFollowSystem(const bool on, const bool animate)
{
	if (m_followSystem == on && animate) {
		// 状态未变且是动画调用，无需处理
		return;
	}
	if (!animate) {
		// 无动画就位：用于启动时的首帧
		m_followSystem = on;
		m_animPhase = AnimPhase::Idle;
		m_themeAlpha = on ? 0.0f : 1.0f;
		m_followSlide = on ? 1.0f : 0.0f;

		// 立即同步到按钮，避免等待下一次 layout/tick
		m_btnTheme.setOpacity(m_themeAlpha);
		const float deltaX = static_cast<float>(m_btnTheme.baseRect().x() - m_btnFollow.baseRect().x());
		m_btnFollow.setOffset(QPointF(deltaX * std::clamp(m_followSlide, 0.0f, 1.0f), 0.0f));
		m_btnTheme.setEnabled(themeInteractive());
		return;
	}

	// 动画过渡
	if (m_followSystem != on) {
		m_followSystem = on;
		startAnimSequence(on);
	}
}

void UiTopBar::setPalette(const Palette& p)
{
	m_btnTheme.setPalette(p.bg, p.bgHover, p.bgPressed, p.icon);
	m_btnFollow.setPalette(p.bg, p.bgHover, p.bgPressed, p.icon);
	m_btnMin.setPalette(p.bg, p.bgHover, p.bgPressed, p.icon);
	m_btnMax.setPalette(p.bg, p.bgHover, p.bgPressed, p.icon);
	// 关闭键常见是红色 hover/press；这里如需可定制，可另做参数。暂沿用统一调色板。
	m_btnClose.setPalette(p.bg, p.bgHover, p.bgPressed, p.icon);
}

void UiTopBar::setCornerRadius(const float r)
{
	m_btnTheme.setCornerRadius(r);
	m_btnFollow.setCornerRadius(r);
	m_btnMin.setCornerRadius(r);
	m_btnMax.setCornerRadius(r);
	m_btnClose.setCornerRadius(r);
}

void UiTopBar::setSvgPaths(QString themeWhenDark, QString themeWhenLight, QString followOn, QString followOff)
{
	m_svgThemeWhenDark = std::move(themeWhenDark);
	m_svgThemeWhenLight = std::move(themeWhenLight);
	m_svgFollowOn = std::move(followOn);
	m_svgFollowOff = std::move(followOff);
}

void UiTopBar::updateLayout(const QSize& windowSize)
{
	constexpr int margin = 12;
	constexpr int btnSize = 28;
	constexpr int gap = 8;

	// 右侧从 Close -> Max -> Min，然后才是 Follow -> Theme
	int x = windowSize.width() - margin - btnSize;
	constexpr int y = margin;

	m_btnClose.setBaseRect(QRect(x, y, btnSize, btnSize));
	x -= (btnSize + gap);
	m_btnMax.setBaseRect(QRect(x, y, btnSize, btnSize));
	x -= (btnSize + gap);
	m_btnMin.setBaseRect(QRect(x, y, btnSize, btnSize));
	x -= (btnSize + gap);
	m_btnTheme.setBaseRect(QRect(x, y, btnSize, btnSize));
	x -= (btnSize + gap);
	m_btnFollow.setBaseRect(QRect(x, y, btnSize, btnSize));

	// 如果没有动画，按当前 follow 状态立即就位
	if (m_animPhase == AnimPhase::Idle) {
		m_themeAlpha = m_followSystem ? 0.0f : 1.0f;
		m_followSlide = m_followSystem ? 1.0f : 0.0f;
	}
	m_btnTheme.setOpacity(m_themeAlpha);

	const float deltaX = static_cast<float>(m_btnTheme.baseRect().x() - m_btnFollow.baseRect().x());
	m_btnFollow.setOffset(QPointF(deltaX * std::clamp(m_followSlide, 0.0f, 1.0f), 0.0f));

	m_btnTheme.setEnabled(themeInteractive());

	// 更新整体包围盒（含所有按钮）
	QRect u = m_btnTheme.visualRectF().toRect();
	u = u.united(m_btnFollow.visualRectF().toRect());
	u = u.united(m_btnMin.visualRectF().toRect());
	u = u.united(m_btnMax.visualRectF().toRect());
	u = u.united(m_btnClose.visualRectF().toRect());
	m_bounds = u;
}

void UiTopBar::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, const float devicePixelRatio)
{
	m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);

	if (!m_loader || !m_gl) return;

	// 主题/跟随图标
	const QString themePath = m_dark ? m_svgThemeWhenDark : m_svgThemeWhenLight;
	const QString themeBaseKey = m_dark ? "theme_sun" : "theme_moon";

	const QString followPath = m_followSystem ? m_svgFollowOn : m_svgFollowOff;
	const QString followBaseKey = m_followSystem ? "follow_on" : "follow_off";

	m_btnTheme.setIconPainter([this, themePath, themeBaseKey](const QRectF& r, Render::FrameData& fd, const QColor& iconColor, float) {
		if (!m_loader || !m_gl) return;
		constexpr int iconLogical = 18;
		const int px = std::lround(iconLogical * m_dpr);
		const QString key = iconCacheKey(themeBaseKey, iconLogical, m_dpr); // 已带 @px

		QByteArray svg = RenderUtils::loadSvgCached(themePath);
		const int tex = m_loader->ensureSvgPx(key, svg, QSize(px, px), QColor(255, 255, 255, 255), m_gl);
		const QSize texSz = m_loader->textureSizePx(tex);

		const QRectF dst(r.center().x() - iconLogical * 0.5, r.center().y() - iconLogical * 0.5, iconLogical, iconLogical);
		fd.images.push_back(Render::ImageCmd{
			.dstRect = dst, .textureId = tex, .srcRectPx = QRectF(0, 0, texSz.width(), texSz.height()),
			.tint = iconColor, .clipRect = r // 新增：按钮图标裁剪到按钮矩形
			});
		});

	m_btnFollow.setIconPainter([this, followPath, followBaseKey](const QRectF& r, Render::FrameData& fd, const QColor& iconColor, float) {
		if (!m_loader || !m_gl) return;
		constexpr int iconLogical = 18;
		const int px = std::lround(iconLogical * m_dpr);
		const QString key = iconCacheKey(followBaseKey, iconLogical, m_dpr);

		QByteArray svg = RenderUtils::loadSvgCached(followPath);
		const int tex = m_loader->ensureSvgPx(key, svg, QSize(px, px), QColor(255, 255, 255, 255), m_gl);
		const QSize texSz = m_loader->textureSizePx(tex);

		const QRectF dst(r.center().x() - iconLogical * 0.5, r.center().y() - iconLogical * 0.5, iconLogical, iconLogical);
		fd.images.push_back(Render::ImageCmd{
			.dstRect = dst, .textureId = tex, .srcRectPx = QRectF(0, 0, texSz.width(), texSz.height()),
			.tint = iconColor, .clipRect = r
			});
		});

	// 三大键图标
	auto setupSvgIcon = [this](Ui::Button& btn, const QString& baseKey, const QString& path, int logicalPx) {
		btn.setIconPainter([this, baseKey, path, logicalPx](const QRectF& r, Render::FrameData& fd, const QColor& iconColor, float) {
			if (!m_loader || !m_gl) return;
			const int px = std::lround(static_cast<float>(logicalPx) * m_dpr);
			const QString key = iconCacheKey(baseKey, logicalPx, m_dpr);

			QByteArray svg = RenderUtils::loadSvgCached(path);
			const int tex = m_loader->ensureSvgPx(key, svg, QSize(px, px), QColor(255, 255, 255, 255), m_gl);
			const QSize texSz = m_loader->textureSizePx(tex);

			const QRectF dst(
				r.center().x() - static_cast<qreal>(logicalPx) * 0.5,
				r.center().y() - static_cast<qreal>(logicalPx) * 0.5,
				logicalPx,
				logicalPx
			);

			fd.images.push_back(Render::ImageCmd{
				.dstRect = dst, .textureId = tex, .srcRectPx = QRectF(0,0,texSz.width(), texSz.height()),
				.tint = iconColor, .clipRect = r
				});
			});
		};

	// 统一用 16 的逻辑像素大小，视觉更协调
	setupSvgIcon(m_btnMin, "sys_min", m_svgSysMin, 16);
	setupSvgIcon(m_btnMax, "sys_max", m_svgSysMax, 16);
	setupSvgIcon(m_btnClose, "sys_close", m_svgSysClose, 16);
}

bool UiTopBar::onMousePress(const QPoint& pos)
{
	bool handled = false;
	handled = m_btnTheme.onMousePress(pos) || handled;
	handled = m_btnFollow.onMousePress(pos) || handled;
	handled = m_btnMin.onMousePress(pos) || handled;
	handled = m_btnMax.onMousePress(pos) || handled;
	handled = m_btnClose.onMousePress(pos) || handled;
	return handled;
}

bool UiTopBar::onMouseMove(const QPoint& pos)
{
	m_btnTheme.setEnabled(themeInteractive());
	bool c = false;
	c = m_btnTheme.onMouseMove(pos) || c;
	c = m_btnFollow.onMouseMove(pos) || c;
	c = m_btnMin.onMouseMove(pos) || c;
	c = m_btnMax.onMouseMove(pos) || c;
	c = m_btnClose.onMouseMove(pos) || c;
	return c;
}

bool UiTopBar::onMouseRelease(const QPoint& pos)
{
	m_btnTheme.setEnabled(themeInteractive());
	bool clickedTheme = false, clickedFollow = false;
	bool handled = false;

	handled = m_btnTheme.onMouseRelease(pos, clickedTheme) || handled;
	handled = m_btnFollow.onMouseRelease(pos, clickedFollow) || handled;

	bool cMin = false, cMax = false, cClose = false;
	handled = m_btnMin.onMouseRelease(pos, cMin) || handled;
	handled = m_btnMax.onMouseRelease(pos, cMax) || handled;
	handled = m_btnClose.onMouseRelease(pos, cClose) || handled;

	// 记录待处理动作
	m_clickThemePending = m_clickThemePending || clickedTheme;
	m_clickFollowPending = m_clickFollowPending || clickedFollow;
	m_clickMinPending = m_clickMinPending || cMin;
	m_clickMaxPending = m_clickMaxPending || cMax;
	m_clickClosePending = m_clickClosePending || cClose;

	return handled || clickedTheme || clickedFollow || cMin || cMax || cClose;
}

void UiTopBar::append(Render::FrameData& fd) const
{
	m_btnTheme.append(fd);
	m_btnFollow.append(fd);
	m_btnMin.append(fd);
	m_btnMax.append(fd);
	m_btnClose.append(fd);
}

bool UiTopBar::tick()
{
	bool active = m_animPhase != AnimPhase::Idle;

	if (!active) return false;
	if (!m_animClock.isValid()) m_animClock.start();

	const qint64 now = m_animClock.elapsed();
	const float  tRaw = m_animDurationMs > 0 ? static_cast<float>(now - m_phaseStartMs) / static_cast<float>(m_animDurationMs) : 1.0f;
	const float  t = std::clamp(tRaw, 0.0f, 1.0f);
	const float  e = easeInOut(t);

	switch (m_animPhase) {
	case AnimPhase::HideTheme_FadeOut:
		m_themeAlpha = lerp(m_phaseStartAlpha, 0.0f, e);
		if (t >= 1.0f) { m_phaseStartSlide = m_followSlide; beginPhase(AnimPhase::MoveFollow_Right, 200); }
		break;
	case AnimPhase::MoveFollow_Right: {
		m_followSlide = lerp(m_phaseStartSlide, 1.0f, e);
		if (t >= 1.0f) { m_animPhase = AnimPhase::Idle; }
		break;
	}
	case AnimPhase::MoveFollow_Left: {
		m_followSlide = lerp(m_phaseStartSlide, 0.0f, e);
		if (t >= 1.0f) { m_phaseStartAlpha = m_themeAlpha; beginPhase(AnimPhase::ShowTheme_FadeIn, 160); }
		break;
	}
	case AnimPhase::ShowTheme_FadeIn:
		m_themeAlpha = lerp(m_phaseStartAlpha, 1.0f, e);
		if (t >= 1.0f) { m_animPhase = AnimPhase::Idle; }
		break;
	case AnimPhase::Idle:
	default:
		break;
	}

	// 同步到控件
	m_btnTheme.setOpacity(std::clamp(m_themeAlpha, 0.0f, 1.0f));
	const float deltaX = static_cast<float>(m_btnTheme.baseRect().x() - m_btnFollow.baseRect().x());
	m_btnFollow.setOffset(QPointF(deltaX * std::clamp(m_followSlide, 0.0f, 1.0f), 0.0f));

	// 动画过程中根据可见度控制交互
	m_btnTheme.setEnabled(themeInteractive());

	return m_animPhase != AnimPhase::Idle;
}

bool UiTopBar::themeInteractive() const
{
	if (m_followSystem && m_animPhase != AnimPhase::ShowTheme_FadeIn) {
		return m_themeAlpha > 0.6f;
	}
	return m_themeAlpha > 0.4f;
}

float UiTopBar::easeInOut(float t)
{
	t = std::clamp(t, 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

void UiTopBar::startAnimSequence(const bool followOn)
{
	if (!m_animClock.isValid()) m_animClock.start();
	m_phaseStartAlpha = m_themeAlpha;
	m_phaseStartSlide = m_followSlide;

	if (followOn) beginPhase(AnimPhase::HideTheme_FadeOut, 160);
	else          beginPhase(AnimPhase::MoveFollow_Left, 180);
}

void UiTopBar::beginPhase(const AnimPhase ph, const int durationMs)
{
	m_animPhase = ph;
	m_animDurationMs = durationMs;
	m_phaseStartMs = m_animClock.elapsed();
}

QString UiTopBar::iconCacheKey(const QString& baseKey, const int logicalPx, const float dpr) const
{
	const int px = std::lround(static_cast<float>(logicalPx) * dpr);
	return RenderUtils::makeIconCacheKey(baseKey, px);
}

QByteArray UiTopBar::svgDataCached(const QString& path) const
{
	return RenderUtils::loadSvgCached(path);
}