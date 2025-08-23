#include "UiTopBar.h"

#include "IconLoader.h"
#include "RenderData.hpp"
#include <algorithm>
#include <cmath>
#include <qbytearray.h>
#include <qcolor.h>
#include <qfile.h>
#include <qiodevice.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>
#include <utility>

UiTopBar::UiTopBar()
{
	m_btnTheme.setCornerRadius(6.0f);
	m_btnFollow.setCornerRadius(6.0f);
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
}

void UiTopBar::setCornerRadius(const float r)
{
	m_btnTheme.setCornerRadius(r);
	m_btnFollow.setCornerRadius(r);
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

	const int rightX = windowSize.width() - margin - btnSize;
	constexpr int topY = margin;

	m_btnTheme.setBaseRect(QRect(rightX, topY, btnSize, btnSize));
	m_btnFollow.setBaseRect(QRect(rightX - gap - btnSize, topY, btnSize, btnSize));

	// 如果没有动画，按当前 follow 状态立即就位
	if (m_animPhase == AnimPhase::Idle) {
		m_themeAlpha = m_followSystem ? 0.0f : 1.0f;
		m_followSlide = m_followSystem ? 1.0f : 0.0f;
	}
	m_btnTheme.setOpacity(m_themeAlpha);

	const float deltaX = static_cast<float>(m_btnTheme.baseRect().x() - m_btnFollow.baseRect().x());
	m_btnFollow.setOffset(QPointF(deltaX * std::clamp(m_followSlide, 0.0f, 1.0f), 0.0f));

	m_btnTheme.setEnabled(themeInteractive());

	// 更新整体包围盒（取两个按钮的可视 rect 合并）
	const QRectF r1 = m_btnTheme.visualRectF();
	const QRectF r2 = m_btnFollow.visualRectF();
	m_bounds = r1.toRect().united(r2.toRect());
}

void UiTopBar::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, const float devicePixelRatio)
{
	m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);

	if (!m_loader || !m_gl) return;

	const QString themePath = m_dark ? m_svgThemeWhenDark : m_svgThemeWhenLight;
	const QString themeBaseKey = m_dark ? "theme_sun" : "theme_moon";

	const QString followPath = m_followSystem ? m_svgFollowOn : m_svgFollowOff;
	const QString followBaseKey = m_followSystem ? "follow_on" : "follow_off";

	m_btnTheme.setIconPainter([this, themePath, themeBaseKey](const QRectF& r, Render::FrameData& fd, const QColor& iconColor, float) {
		if (!m_loader || !m_gl) return;
		constexpr int iconLogical = 18;
		const int px = std::lround(iconLogical * m_dpr);
		const QString key = iconCacheKey(themeBaseKey, iconLogical, m_dpr);

		const QByteArray svg = svgDataCached(themePath);
		const int tex = m_loader->ensureSvgPx(key, svg, QSize(px, px), m_gl);
		const QSize texSz = m_loader->textureSizePx(tex);

		const QRectF dst(r.center().x() - iconLogical * 0.5, r.center().y() - iconLogical * 0.5, iconLogical, iconLogical);
		fd.images.push_back(Render::ImageCmd{
			.dstRect = dst,
			.textureId = tex,
			.srcRectPx = QRectF(0, 0, texSz.width(), texSz.height()),
			.tint = iconColor
			});
		});

	m_btnFollow.setIconPainter([this, followPath, followBaseKey](const QRectF& r, Render::FrameData& fd, const QColor& iconColor, float) {
		if (!m_loader || !m_gl) return;
		constexpr int iconLogical = 18;
		const int px = std::lround(iconLogical * m_dpr);
		const QString key = iconCacheKey(followBaseKey, iconLogical, m_dpr);

		const QByteArray svg = svgDataCached(followPath);
		const int tex = m_loader->ensureSvgPx(key, svg, QSize(px, px), m_gl);
		const QSize texSz = m_loader->textureSizePx(tex);

		const QRectF dst(r.center().x() - iconLogical * 0.5, r.center().y() - iconLogical * 0.5, iconLogical, iconLogical);
		fd.images.push_back(Render::ImageCmd{
			.dstRect = dst,
			.textureId = tex,
			.srcRectPx = QRectF(0, 0, texSz.width(), texSz.height()),
			.tint = iconColor
			});
		});
}

bool UiTopBar::onMousePress(const QPoint& pos)
{
	bool handled = false;
	handled = m_btnTheme.onMousePress(pos) || handled;
	handled = m_btnFollow.onMousePress(pos) || handled;
	return handled;
}

bool UiTopBar::onMouseMove(const QPoint& pos)
{
	m_btnTheme.setEnabled(themeInteractive());
	const bool c1 = m_btnTheme.onMouseMove(pos);
	const bool c2 = m_btnFollow.onMouseMove(pos);
	return (c1 || c2);
}

bool UiTopBar::onMouseRelease(const QPoint& pos)
{
	m_btnTheme.setEnabled(themeInteractive());
	bool clickedTheme = false;
	bool clickedFollow = false;
	bool handled = m_btnTheme.onMouseRelease(pos, clickedTheme);
	handled = m_btnFollow.onMouseRelease(pos, clickedFollow) || handled;

	// 记录待处理动作
	m_clickThemePending = m_clickThemePending || clickedTheme;
	m_clickFollowPending = m_clickFollowPending || clickedFollow;

	return handled || clickedTheme || clickedFollow;
}

void UiTopBar::append(Render::FrameData& fd) const
{
	m_btnTheme.append(fd);
	m_btnFollow.append(fd);
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
	return QString("%1@%2px").arg(baseKey).arg(px);
}

QByteArray UiTopBar::svgDataCached(const QString& path) const
{
	if (const auto it = m_svgDataCache.find(path); it != m_svgDataCache.end()) return it.value();
	QFile f(path);
	if (!f.open(QIODevice::ReadOnly)) return {};
	QByteArray data = f.readAll();
	m_svgDataCache.insert(path, data);
	return data;
}