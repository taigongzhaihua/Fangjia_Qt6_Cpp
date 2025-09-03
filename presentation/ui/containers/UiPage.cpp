#include "RenderData.hpp"
#include "UiContent.hpp"
#include "UiPage.h"
#include "IFocusable.hpp"
#include "IFocusContainer.hpp"
#include "ILayoutable.hpp"

#include <algorithm>
#include <cmath>
#include <IconCache.h>
#include <qcolor.h>
#include <qfont.h>
#include <qglobal.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringliteral.h>
#include "RenderUtils.hpp"

QRectF UiPage::cardRectF() const
{
	if (!m_viewport.isValid()) return {};
	return {
		static_cast<qreal>(m_viewport.left() + m_margins.left()),
		static_cast<qreal>(m_viewport.top() + m_margins.top()),
		static_cast<qreal>(std::max(0, m_viewport.width() - m_margins.left() - m_margins.right())),
		static_cast<qreal>(std::max(0, m_viewport.height() - m_margins.top() - m_margins.bottom()))
	};
}

QRectF UiPage::contentRectF() const
{
	const QRectF card = cardRectF();
	// 内容区域：卡片内部，预留标题区域与内边距
	return card.adjusted(m_padding.left(), m_padding.top() + kTitleAreaH, -m_padding.right(), -m_padding.bottom());
}

QSize UiPage::measure(const SizeConstraints& cs)
{
	// Page measurement considers margins + padding + title area (kTitleAreaH), and uses child.measure(widthBounded) when the child supports ILayoutable
	const int marginW = m_margins.left() + m_margins.right();
	const int marginH = m_margins.top() + m_margins.bottom();
	const int paddingW = m_padding.left() + m_padding.right();
	const int paddingH = m_padding.top() + m_padding.bottom();
	
	const int frameW = marginW + paddingW;
	const int frameH = marginH + paddingH + kTitleAreaH;
	
	// Available space for content
	const int availableW = std::max(0, cs.maxW - frameW);
	const int availableH = std::max(0, cs.maxH - frameH);
	
	QSize contentSize(0, 0);
	if (m_content) {
		if (auto* l = dynamic_cast<ILayoutable*>(m_content)) {
			// Use child's measure with width bounded constraints
			SizeConstraints childCs = SizeConstraints::widthBounded(availableW, availableH);
			contentSize = l->measure(childCs);
		} else {
			// Fallback to bounds size
			contentSize = m_content->bounds().size();
			contentSize.setWidth(std::min(contentSize.width(), availableW));
			contentSize.setHeight(std::min(contentSize.height(), availableH));
		}
	}
	
	// Total size is content + frame
	const int totalW = std::clamp(contentSize.width() + frameW, cs.minW, cs.maxW);
	const int totalH = std::clamp(contentSize.height() + frameH, cs.minH, cs.maxH);
	
	return QSize(totalW, totalH);
}

void UiPage::arrange(const QRect& finalRect)
{
	// Store the final rect as our viewport
	m_viewport = finalRect;
	
	// Recompute content rect (card -> title area -> content area) and forward to child content
	if (m_content) {
		const QRect contentRect = contentRectF().toRect();
		
		// Set viewport on child if it implements IUiContent
		if (auto* c = dynamic_cast<IUiContent*>(m_content)) {
			c->setViewportRect(contentRect);
		}
		
		// Arrange child if it implements ILayoutable
		if (auto* l = dynamic_cast<ILayoutable*>(m_content)) {
			l->arrange(contentRect);
		}
	}
}

void UiPage::updateLayout(const QSize& /*windowSize*/)
{
	// Forward viewport and arrange to child if applicable
	if (m_content) {
		const QRect contentRect = contentRectF().toRect();
		
		// Set viewport on child if it implements IUiContent
		if (auto* c = dynamic_cast<IUiContent*>(m_content)) {
			c->setViewportRect(contentRect);
		}
		
		// Arrange child if it implements ILayoutable  
		if (auto* l = dynamic_cast<ILayoutable*>(m_content)) {
			l->arrange(contentRect);
		}
		
		// Let content component also have opportunity to perform its own internal layout
		m_content->updateLayout(m_viewport.size());
	}
}

void UiPage::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float devicePixelRatio)
{
	m_cache = &cache; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
	if (m_content) m_content->updateResourceContext(cache, gl, devicePixelRatio);
}

void UiPage::append(Render::FrameData& fd) const
{
	if (!m_viewport.isValid() || m_viewport.width() <= 0 || m_viewport.height() <= 0) return;

	// 在 viewport 内部绘制内容卡片（留边距）
	const QRectF card = cardRectF();

	// 背景卡片
	fd.roundedRects.push_back(Render::RoundedRectCmd{
		.rect = card,
		.radiusPx = m_cornerRadius,
		.color = m_pal.cardBg,
		.clipRect = QRectF(m_viewport) // 裁剪到页面 viewport
		});

	// 标题文字
	if (!m_cache || !m_gl) return;

	QFont font;
	const int headingPx = std::lround(24.0f * m_dpr);
	font.setPixelSize(headingPx);

	const QString key = RenderUtils::makeTextCacheKey(QStringLiteral("heading|") + m_title, headingPx, m_pal.headingColor);
	const int tex = m_cache->ensureTextPx(key, font, m_title, m_pal.headingColor, m_gl);
	const QSize ts = m_cache->textureSizePx(tex);

	// 逻辑尺寸
	const float wLogical = static_cast<float>(ts.width()) / m_dpr;
	const float hLogical = static_cast<float>(ts.height()) / m_dpr;

	const float centerX = std::round(card.left() + 24);
	const float centerY = std::round(card.top() + 36);
	const float textX = std::round(wLogical);
	const float textY = std::round(hLogical);

	const QRectF dst(centerX, centerY, textX, textY);

	fd.images.push_back(Render::ImageCmd{
		.dstRect = dst,
		.textureId = tex,
		.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
		.tint = QColor(255,255,255,255),
		.clipRect = card // 标题裁剪到卡片
		});

	// 内容裁剪到内容区
	if (m_content) {
		const int rr0 = static_cast<int>(fd.roundedRects.size());
		const int im0 = static_cast<int>(fd.images.size());

		m_content->append(fd);

		RenderUtils::applyParentClip(fd, rr0, im0, contentRectF());
	}
}

void UiPage::onThemeChanged(const bool isDark)
{
	m_isDark = isDark;

	// 应用基础主题调色板
	if (isDark) {
		m_pal = Palette{
			.cardBg = QColor(28, 38, 50, 200),
			.headingColor = QColor(235, 240, 245, 255),
			.bodyColor = QColor(210, 220, 230, 220)
		};
	}
	else {
		m_pal = Palette{
			.cardBg = QColor(255, 255, 255, 245),
			.headingColor = QColor(40, 46, 54, 255),
			.bodyColor = QColor(70, 76, 84, 220)
		};
	}

	// 让子类应用特定的主题设置
	applyPageTheme(isDark);

	// 传播主题变化到内容组件
	if (m_content) {
		m_content->onThemeChanged(isDark);
	}
}

void UiPage::applyPageTheme(bool isDark)
{
	// 基类默认实现为空，子类可以重写
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

bool UiPage::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
	// 将滚轮事件转发到内容区域
	if (m_content && contentRectF().toRect().contains(pos)) {
		return m_content->onWheel(pos, angleDelta);
	}
	return false;
}

bool UiPage::tick()
{
	bool any = false;
	if (m_content) any = m_content->tick() || any;
	return any;
}

void UiPage::enumerateFocusables(std::vector<IFocusable*>& out) const
{
	if (!m_content) return;
	
	// 如果内容组件本身可以获得焦点，添加它
	if (auto* focusable = dynamic_cast<IFocusable*>(m_content)) {
		if (focusable->canFocus()) {
			out.push_back(focusable);
		}
	}
	
	// 如果内容组件是容器，递归枚举其可焦点子组件
	if (auto* container = dynamic_cast<IFocusContainer*>(m_content)) {
		container->enumerateFocusables(out);
	}
}