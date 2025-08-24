#include "RenderData.hpp"
#include "UiContent.hpp"
#include "UiPage.h"

#include <algorithm>
#include <cmath>
#include <IconLoader.h>
#include <qcolor.h>
#include <qfont.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringliteral.h>

QRectF UiPage::cardRectF() const
{
	if (!m_viewport.isValid()) return {};
	return QRectF(
		m_viewport.left() + kMargin,
		m_viewport.top() + kMarginTop,
		std::max(0, m_viewport.width() - kMargin * 2),
		std::max(0, m_viewport.height() - kMargin - kMarginTop)
	);
}

QRectF UiPage::contentRectF() const
{
	const QRectF card = cardRectF();
	// 内容区域：卡片内部，预留标题区域与内边距
	return card.adjusted(kCardPad, kCardPad + kTitleAreaH, -kCardPad, -kCardPad);
}

void UiPage::updateLayout(const QSize& /*windowSize*/)
{
	// 将 contentRect 下发给内容组件（若实现了 IUiContent）
	if (m_content) {
		if (auto* c = dynamic_cast<IUiContent*>(m_content)) {
			c->setViewportRect(contentRectF().toRect());
		}
		// 让内容组件也有机会进行它自己的内部布局（传入窗口 size 对它用处不大，但保持调用流程一致）
		m_content->updateLayout(m_viewport.size());
	}
}

void UiPage::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio)
{
	m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
	if (m_content) m_content->updateResourceContext(loader, gl, devicePixelRatio);
}

void UiPage::append(Render::FrameData& fd) const
{
	if (!m_viewport.isValid() || m_viewport.width() <= 0 || m_viewport.height() <= 0) return;

	// 在 viewport 内部绘制内容卡片（留边距）
	const QRectF card = cardRectF();

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

	// 放在卡片左上角，四周 24px 内边距
	constexpr float pad = static_cast<float>(kCardPad);
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

	// 让内容组件在卡片内绘制自身内容（它已知道自己的 viewport）
	if (m_content) {
		m_content->append(fd);
	}
}

bool UiPage::onMousePress(const QPoint& pos)
{
	// 将事件仅转发给内容区域
	if (m_content && contentRectF().toRect().contains(pos)) {
		return m_content->onMousePress(pos);
	}
	return false;
}

bool UiPage::onMouseMove(const QPoint& pos)
{
	if (m_content && contentRectF().toRect().contains(pos)) {
		return m_content->onMouseMove(pos);
	}
	// 若不在内容区，可视为 hover 离开，交给内容组件清理 hover 状态（可选）
	return false;
}

bool UiPage::onMouseRelease(const QPoint& pos)
{
	if (m_content && contentRectF().toRect().contains(pos)) {
		return m_content->onMouseRelease(pos);
	}
	return false;
}

bool UiPage::tick()
{
	bool any = false;
	if (m_content) any = m_content->tick() || any;
	return any;
}