#include "UiPage.h"

#include "RenderData.hpp"
#include <algorithm>
#include <cmath>
#include <qcolor.h>
#include <qfont.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringliteral.h>

void UiPage::append(Render::FrameData& fd) const
{
	if (!m_viewport.isValid() || m_viewport.width() <= 0 || m_viewport.height() <= 0) return;

	// 在 viewport 内部绘制内容卡片（留边距）
	constexpr int margin = 8;
	constexpr int marginTop = 48;

	const QRectF card(
		m_viewport.left() + margin,
		m_viewport.top() + marginTop,
		std::max(0, m_viewport.width() - margin * 2),
		std::max(0, m_viewport.height() - margin - marginTop)
	);

	// 背景卡片
	fd.roundedRects.push_back(Render::RoundedRectCmd{
		.rect = card,
		.radiusPx = 8.0f,
		.color = m_pal.cardBg
		});

	// 标题文字
	if (!m_loader || !m_gl) return;

	QFont font;
	const int headingPx = std::lround(24.0f * m_dpr);
	font.setPixelSize(headingPx);

	const QString key = textCacheKey(QStringLiteral("heading|") + m_title, headingPx, m_pal.headingColor);
	const int tex = m_loader->ensureTextPx(key, font, m_title, m_pal.headingColor, m_gl);
	const QSize ts = m_loader->textureSizePx(tex);

	// 将像素尺寸换成逻辑像素
	const float wLogical = static_cast<float>(ts.width()) / m_dpr;
	const float hLogical = static_cast<float>(ts.height()) / m_dpr;

	// 放在卡片左上角，四周 20px 内边距
	constexpr float pad = 24.0f;
	const QRectF dst(
		card.left() + pad,
		card.top() + pad,
		wLogical,
		hLogical
	);

	fd.images.push_back(Render::ImageCmd{
		.dstRect = dst,
		.textureId = tex,
		.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
		.tint = QColor(255,255,255,255) // 文字已在纹理阶段着色
		});
}