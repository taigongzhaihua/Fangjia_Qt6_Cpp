#include "RenderData.hpp"
#include "TabViewModel.h"
#include "UiTabView.h"

#include "IconCache.h"
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include <algorithm>
#include <cmath>
#include <qcolor.h>
#include <qfont.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <ranges>
#include <unordered_map>
#include <vector>

#include "RenderUtils.hpp"

void UiTabView::setViewModel(TabViewModel* vm)
{
	if (m_vm == vm) return;
	m_vm = vm;

	// 清理交互状态
	m_hover = -1;
	m_pressed = -1;

	// 同步视图状态（无动画）
	syncFromVmInstant();

	// 确保当前内容拿到 viewport 与资源上下文
	ensureCurrentContentSynced();
}

void UiTabView::syncFromVmInstant()
{
	if (!m_vm) return;

	const int sel = m_vm->selectedIndex();
	m_viewSelected = sel;

	if (sel >= 0 && sel < m_vm->count()) {
		const QRectF r = tabRectF(sel);
		m_highlightCenterX = r.isValid() ? static_cast<float>(r.center().x()) : -1.0f;
	}
	else {
		m_highlightCenterX = -1.0f;
	}
	m_animHighlight.active = false;
}

int UiTabView::tabCount() const
{
	return m_vm ? m_vm->count() : 0;
}

QString UiTabView::tabLabel(const int i) const
{
	if (m_vm) {
		if (const auto& items = m_vm->items(); i >= 0 && i < items.size()) {
			return items[i].label;
		}
	}
	return {};
}

QRectF UiTabView::contentRectF() const
{
	if (!m_viewport.isValid()) return {};
	const float left = static_cast<float>(m_viewport.left() + m_margin.left() + m_padding.left() + m_contentMargin.left() + m_contentPadding.left());
	const float top = static_cast<float>(
		m_viewport.top() +
		m_margin.top() + m_padding.top() +
		m_tabBarMargin.top() + m_tabHeight + m_tabBarMargin.bottom() +
		m_contentMargin.top() + m_contentPadding.top()) + m_spacing;
	const float width =
		std::max(
			0.0f,
			static_cast<float>(
				m_viewport.width() -
				m_margin.left() - m_margin.right() -
				m_padding.left() - m_padding.right() -
				m_contentMargin.left() - m_contentMargin.right() -
				m_contentPadding.left() - m_contentPadding.right()));
	const float height = std::max(
		0.0f,
		static_cast<float>(
			m_viewport.height() -
			m_margin.top() - m_margin.bottom() -
			m_padding.top() - m_padding.bottom() -
			m_contentMargin.top() - m_contentMargin.bottom() -
			m_contentPadding.top() - m_contentPadding.bottom() -
			m_tabHeight - m_spacing));
	return { left, top, width, height };

}
int UiTabView::selectedIndex() const noexcept
{
	return m_vm ? m_vm->selectedIndex() : -1;
}

void UiTabView::updateLayout(const QSize& windowSize)
{
	// 窗口尺寸变化时，确保高亮位置正确
	if (!m_animHighlight.active && m_viewSelected >= 0 && m_viewSelected < tabCount()) {
		const QRectF r = tabRectF(m_viewSelected);
		m_highlightCenterX = r.isValid() ? static_cast<float>(r.center().x()) : -1.0f;
	}
	// 更新当前内容的布局
	const int curIdx = selectedIndex();
	if (IUiComponent* curContent = content(curIdx)) {
		// 计算内容区域
		const QRect contentRect = contentRectF().toRect();

		// 如果内容实现了 IUiContent，设置视口
		if (auto* c = dynamic_cast<IUiContent*>(curContent)) {
			c->setViewportRect(contentRect);
		}

		// 更新内容布局
		curContent->updateLayout(windowSize);
	}
}

void UiTabView::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float devicePixelRatio)
{
	m_cache = &cache;
	m_gl = gl;
	m_dpr = std::max(0.5f, devicePixelRatio);

	// 更新当前内容的资源上下文
	const int curIdx = selectedIndex();
	if (IUiComponent* curContent = content(curIdx)) {
		curContent->updateResourceContext(cache, gl, devicePixelRatio);
	}
}

QRectF UiTabView::tabBarRectF() const
{
	if (!m_viewport.isValid()) return {};
	return {
		static_cast<float>(m_viewport.left() + m_margin.left() + m_padding.left()),
		static_cast<float>(m_viewport.top() + m_margin.top() + m_padding.top()),
		std::max(0.0, static_cast<double>(m_viewport.width() - m_margin.left() - m_margin.right() - m_padding.left() - m_padding.right())),
		static_cast<float>(m_tabHeight)
	};
}

QRectF UiTabView::tabRectF(const int i) const
{
	const QRectF bar = tabBarRectF();
	const int n = tabCount();
	if (i < 0 || i >= n || bar.width() <= 0.0) return {};
	const qreal w = (bar.width() - static_cast<float>(std::max(0, n - 1)) * m_tabBarSpacing - m_tabBarPadding.left() - m_tabBarPadding.right()) / std::max(1, n);
	return {
		bar.left() + m_tabBarPadding.left() + (w + m_tabBarSpacing) * i ,
		bar.top() + m_tabBarPadding.top(),
		w,
		bar.height() - m_tabBarPadding.top() - m_tabBarPadding.bottom()
	};
}
void UiTabView::append(Render::FrameData& fd) const
{
	if (!m_viewport.isValid() || m_viewport.width() <= 0 || m_viewport.height() <= 0) return;
	if (!m_cache || !m_gl) return;

	const QRectF bar = tabBarRectF();

	// TabBar 背景
	if (m_pal.barBg.alpha() > 0) {
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = bar.adjusted(m_tabBarMargin.left(), m_tabBarMargin.top(), -m_tabBarMargin.right(), -m_tabBarMargin.bottom()),
			.radiusPx = 8.0f,
			.color = m_pal.barBg,
			.clipRect = QRectF(m_viewport) // 裁剪到整个 TabView 区域
			});
	}

	// Content 背景
	if (m_pal.contentBg.alpha() > 0) {
		const QRectF contentR = contentRectF();
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = contentR.adjusted(-m_contentPadding.left(), -m_contentPadding.top(), m_contentPadding.right(), m_contentPadding.bottom()),
			.radiusPx = 8.0f,
			.color = m_pal.contentBg,
			.clipRect = QRectF(m_viewport)
			});
	}

	// 整体高亮单元（保持不变）...
	if (m_viewSelected >= 0 && m_viewSelected < tabCount() && m_highlightCenterX >= 0.0f) {
		const QRectF rSelTmpl = tabRectF(m_viewSelected);
		const float bgW = std::max(8.0f, static_cast<float>(rSelTmpl.width()));
		const float bgH = std::max(8.0f, static_cast<float>(rSelTmpl.height()));
		const QRectF bgRect(
			m_highlightCenterX - bgW * 0.5f,
			rSelTmpl.top(),
			bgW,
			bgH
		);
		if (m_indicatorStyle == IndicatorStyle::Full || m_pal.tabSelectedBg.alpha() > 0) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = bgRect,
				.radiusPx = 6.0f,
				.color = m_pal.tabSelectedBg,
				.clipRect = bgRect
				});
		}
		if (m_indicatorStyle != IndicatorStyle::Full) {
			const float indW = std::clamp(bgW * 0.5f, 24.0f, std::max(24.0f, bgW - 10.0f));
			constexpr float indH = 3.0f;
			QRectF indRect;
			if (m_indicatorStyle == IndicatorStyle::Bottom) {
				constexpr float indOffsetUp = 4.0f;
				indRect = QRectF(
					bgRect.center().x() - indW * 0.5f,
					bgRect.bottom() - indOffsetUp,
					indW,
					indH
				);
			}
			else {
				constexpr float indOffsetDown = 4.0f;
				indRect = QRectF(
					bgRect.center().x() - indW * 0.5f,
					bgRect.top() + indOffsetDown,
					indW,
					indH
				);
			}
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = indRect,
				.radiusPx = indH * 0.5f,
				.color = m_pal.indicator,
				.clipRect = bgRect
				});
		}
	}

	// hover/press 背景与标签绘制（保持不变）...

	const int n = tabCount();
	for (int i = 0; i < n; ++i) {
		if (i == m_viewSelected) continue;
		const QRectF r = tabRectF(i);
		if (i == m_pressed) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = r,
				.radiusPx = 6.0f,
				.color = m_pal.tabHover.darker(115)
				});
		}
		else if (i == m_hover) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = r,
				.radiusPx = 6.0f,
				.color = m_pal.tabHover
				});
		}
	}

	const int fontPx = std::lround(14.0f * m_dpr);
	QFont font;
	font.setPixelSize(fontPx);
	font.setStyleStrategy(QFont::PreferAntialias);

	for (int i = 0; i < n; ++i) {
		const QRectF r = tabRectF(i);
		const QString label = tabLabel(i);
		if (label.isEmpty()) continue;

		const QColor textColor = (i == m_viewSelected ? m_pal.labelSelected : m_pal.label);

		const QString key = textCacheKey(QString("tab|%1").arg(label), fontPx, textColor);
		const int tex = m_cache->ensureTextPx(key, font, label, textColor, m_gl);
		const QSize ts = m_cache->textureSizePx(tex);

		const float wLogical = static_cast<float>(ts.width()) / m_dpr;
		const float hLogical = static_cast<float>(ts.height()) / m_dpr;

		const float centerX = std::round(r.center().x());
		const float centerY = std::round(r.center().y());
		const float textX = std::round(centerX - wLogical * 0.5f);
		const float textY = std::round(centerY - hLogical * 0.5f);

		const QRectF textDst(textX, textY, wLogical, hLogical);

		fd.images.push_back(Render::ImageCmd{
			.dstRect = textDst,
			.textureId = tex,
			.srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
			.tint = QColor(255,255,255,255),
			.clipRect = r
			});
	}

	// 当前内容 + 父裁剪叠加到内容区
	int curIdx = selectedIndex();
	if (IUiComponent* curContent = content(curIdx)) {
		const int rr0 = static_cast<int>(fd.roundedRects.size());
		const int im0 = static_cast<int>(fd.images.size());

		curContent->append(fd);

		RenderUtils::applyParentClip(fd, rr0, im0, contentRectF());
	}
}

bool UiTabView::onMousePress(const QPoint& pos)
{
	if (!m_viewport.contains(pos)) return false;

	// 先判断是否点在tab bar上，若否则转发到内容
	for (int i = 0; i < tabCount(); ++i) {
		if (tabRectF(i).toRect().contains(pos)) {
			m_pressed = i;
			return true;
		}
	}

	const int curIdx = selectedIndex();
	IUiComponent* curContent = content(curIdx);
	if (curContent && m_viewport.contains(pos)) {
		return curContent->onMousePress(pos);
	}
	return false;
}

bool UiTabView::onMouseMove(const QPoint& pos)
{
	int hov = -1;
	if (m_viewport.contains(pos)) {
		for (int i = 0; i < tabCount(); ++i) {
			if (tabRectF(i).toRect().contains(pos)) {
				hov = i;
				break;
			}
		}
	}
	const bool changed = (hov != m_hover);
	m_hover = hov;

	const int curIdx = selectedIndex();
	IUiComponent* curContent = content(curIdx);
	if (curContent && m_viewport.contains(pos)) {
		return curContent->onMouseMove(pos) || changed;
	}
	return changed;
}

bool UiTabView::onMouseRelease(const QPoint& pos)
{
	const int wasPressed = m_pressed;
	m_pressed = -1;

	if (!m_viewport.contains(pos)) {
		return (wasPressed >= 0);
	}

	int hit = -1;
	for (int i = 0; i < tabCount(); ++i) {
		if (tabRectF(i).toRect().contains(pos)) {
			hit = i;
			break;
		}
	}

	if (hit >= 0 && hit == wasPressed) {
		if (m_vm) {
			m_vm->setSelectedIndex(hit);
			// 立即保证新内容具备上下文与视口
			ensureCurrentContentSynced();
			return true;
		}
		// 如果没有 VM，不处理点击事件
	}

	const int curIdx = selectedIndex();
	IUiComponent* curContent = content(curIdx);
	if (curContent && m_viewport.contains(pos)) {
		return curContent->onMouseRelease(pos);
	}
	return (wasPressed >= 0);
}

bool UiTabView::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
	if (!m_viewport.contains(pos)) return false;
	const QRect contentR = contentRectF().toRect();
	if (contentR.contains(pos)) {
		if (IUiComponent* cur = content(selectedIndex())) {
			return cur->onWheel(pos, angleDelta);
		}
	}
	return false;
}

bool UiTabView::tick()
{
	bool any = false;
	if (!m_clock.isValid()) m_clock.start();

	// VM 模式：检查并同步变化
	if (m_vm) {
		const int vmSel = m_vm->selectedIndex();
		if (vmSel != m_viewSelected) {
			if (vmSel >= 0 && vmSel < m_vm->count()) {
				const QRectF targetR = tabRectF(vmSel);
				startHighlightAnim(static_cast<float>(targetR.center().x()));
			}
			else {
				m_highlightCenterX = -1.0f;
				m_animHighlight.active = false;
			}
			m_viewSelected = vmSel;

			// 选中项变化后，确保当前内容上下文与视口同步
			ensureCurrentContentSynced();

			any = true;
		}
	}

	// 处理动画
	if (m_animHighlight.active) {
		const qint64 now = m_clock.elapsed();
		const float t = easeInOut(
			static_cast<float>(now - m_animHighlight.startMs) /
			static_cast<float>(std::max(1, m_animHighlight.durationMs))
		);
		m_highlightCenterX = m_animHighlight.start +
			(m_animHighlight.end - m_animHighlight.start) * std::clamp(t, 0.0f, 1.0f);
		if (t >= 1.0f) m_animHighlight.active = false;
		return true;
	}

	const int curIdx = selectedIndex();
	if (IUiComponent* curContent = content(curIdx)) any = curContent->tick() || any;
	return any;
}

void UiTabView::onThemeChanged(const bool isDark)
{
	// 自动根据主题设置调色板
	if (isDark) {
		m_pal = Palette{
			.barBg = QColor(220,233,245,10),
			.contentBg = QColor(220,233,245,10),
			.tabHover = QColor(255,255,255,18),
			.tabSelectedBg = QColor(255,255,255,28),
			.indicator = QColor(0,122,255,220),
			.label = QColor(230,235,240,220),
			.labelSelected = QColor(255,255,255,255)
		};
	}
	else {
		m_pal = Palette{
			.barBg = QColor(10,23,35,10),
			.contentBg = QColor(10,23,35,10),
			.tabHover = QColor(0,0,0,16),
			.tabSelectedBg = QColor(0,0,0,22),
			.indicator = QColor(0,102,204,220),
			.label = QColor(50,60,70,255),
			.labelSelected = QColor(20,32,48,255)
		};
	}

	// 新增：将主题变化传播给所有 tab 内容（不仅仅是当前内容）
	for (const auto& val : m_tabContents | std::views::values) {
		if (val) val->onThemeChanged(isDark);
	}
	// 不需要重建；下一帧按新配色渲染
}

void UiTabView::startHighlightAnim(const float toCenterX)
{
	if (!m_clock.isValid()) m_clock.start();
	m_animHighlight.active = true;
	m_animHighlight.start = (m_highlightCenterX < 0.0f ? toCenterX : m_highlightCenterX);
	m_animHighlight.end = toCenterX;
	m_animHighlight.startMs = m_clock.elapsed();
	m_animHighlight.durationMs = m_animDuration;
}

QString UiTabView::textCacheKey(const QString& baseKey, const int px, const QColor& color)
{
	return RenderUtils::makeTextCacheKey(baseKey, px, color);
}
float UiTabView::easeInOut(float t)
{
	t = std::clamp(t, 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

void UiTabView::setContent(const int tabIdx, IUiComponent* content)
{
	if (tabIdx < 0) return;
	m_tabContents[tabIdx] = content;
	// 如果刚好是当前选中的内容，保证其有上下文与视口
	if (tabIdx == selectedIndex()) {
		ensureCurrentContentSynced();
	}
}

void UiTabView::setContents(const std::vector<IUiComponent*>& contents)
{
	m_tabContents.clear();
	for (size_t i = 0; i < contents.size(); ++i) {
		if (contents[i]) m_tabContents[static_cast<int>(i)] = contents[i];
	}
	// 同步当前内容上下文
	ensureCurrentContentSynced();
}

IUiComponent* UiTabView::content(const int tabIdx) const
{
	const auto it = m_tabContents.find(tabIdx);
	return (it != m_tabContents.end()) ? it->second : nullptr;
}

void UiTabView::ensureCurrentContentSynced() const
{
	const int curIdx = selectedIndex();
	IUiComponent* cur = content(curIdx);
	if (!cur) return;

	// 先下发 viewport 给顶层内容
	if (auto* c = dynamic_cast<IUiContent*>(cur)) {
		const QRect contentRect = contentRectF().toRect();
		if (contentRect.isValid()) c->setViewportRect(contentRect);
	}

	// 关键补充：推进一轮布局，让容器把 viewport 继续传播给子孙
	cur->updateLayout(m_viewport.size());

	// 再补资源上下文（如可用）
	if (m_cache && m_gl) {
		cur->updateResourceContext(*m_cache, m_gl, m_dpr);
	}
}
