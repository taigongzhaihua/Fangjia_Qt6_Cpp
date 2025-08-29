#include "UiPanel.h"
#include <algorithm>

#include "ILayoutable.hpp"
#include <limits>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <ranges>
#include <RenderData.hpp>
#include <RenderUtils.hpp>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <vector>

UiPanel::UiPanel(Orientation o)
	: m_orient(o)
{
}

void UiPanel::addChild(IUiComponent* c, const CrossAlign a)
{
	if (!c) return;
	for (const auto& ch : m_children) if (ch.component == c) return;
	m_children.push_back(Child{ .component = c, .crossAlign = a, .visible = true });
	m_childRects.resize(m_children.size());
}

void UiPanel::clearChildren()
{
	m_children.clear();
	m_childRects.clear();
}

QRect UiPanel::contentRect() const
{
	const QRect r = m_viewport.adjusted(
		m_margins.left() + m_padding.left(),
		m_margins.top() + m_padding.top(),
		-(m_margins.right() + m_padding.right()),
		-(m_margins.bottom() + m_padding.bottom()));
	if (r.width() < 0 || r.height() < 0) return {};
	return r;
}

// 回退测量：优先 ILayoutable；否则用 bounds().size()（可能为 0）
QSize UiPanel::measureChild(IUiComponent* c, const int crossAvail) const
{
	if (!c) return { 0, 0 };

	if (auto* l = dynamic_cast<ILayoutable*>(c))
	{
		// 主轴无上限，交叉轴受限
		if (m_orient == Orientation::Horizontal)
		{
			return l->measure(SizeConstraints{
				.minW = 0, .minH = 0, .maxW = std::numeric_limits<int>::max() / 2, .maxH =
				std::max(0, crossAvail)
				});
		}
		{
			return l->measure(SizeConstraints{
				.minW = 0, .minH = 0, .maxW = std::max(0, crossAvail), .maxH =
				std::numeric_limits<int>::max() / 2
				});
		}
	}

	const QSize s = c->bounds().size();
	// 交叉轴限制
	if (m_orient == Orientation::Horizontal)
	{
		return { std::max(0, s.width()), std::min(std::max(0, crossAvail), std::max(0, s.height())) };
	}
	return { std::min(std::max(0, crossAvail), std::max(0, s.width())), std::max(0, s.height()) };
}

QRect UiPanel::placeChild(const QRect& area, const int cur, const QSize& desired, const CrossAlign a) const
{
	if (m_orient == Orientation::Horizontal)
	{
		const int w = std::max(0, desired.width());
		const int h = std::max(0, std::min(desired.height(), area.height()));
		int y;
		switch (a)
		{
		case CrossAlign::Center: y = area.center().y() - h / 2;
			break;
		case CrossAlign::End: y = area.bottom() - h;
			break;
		case CrossAlign::Stretch: default: y = area.top();
			break;
		}
		return { area.left() + cur, y, w, (a == CrossAlign::Stretch ? area.height() : h) };
	}
	{
		const int h = std::max(0, desired.height());
		const int w = std::max(0, std::min(desired.width(), area.width()));
		int x;
		switch (a)
		{
		case CrossAlign::Center: x = area.center().x() - w / 2;
			break;
		case CrossAlign::End: x = area.right() - w;
			break;
		case CrossAlign::Stretch: default: x = area.left();
			break;
		}
		return { x, area.top() + cur, (a == CrossAlign::Stretch ? area.width() : w), h };
	}
}

// ========== 新增：ILayoutable 实现 ==========
QSize UiPanel::measure(const SizeConstraints& cs)
{
	// 先扣除 panel 自身的 margins + padding
	const int padW = m_margins.left() + m_margins.right() + m_padding.left() + m_padding.right();
	const int padH = m_margins.top() + m_margins.bottom() + m_padding.top() + m_padding.bottom();

	const bool isH = (m_orient == Orientation::Horizontal);
	const int crossMaxAvail = std::max(0, isH ? (cs.maxH - padH) : (cs.maxW - padW));

	int mainSum = 0;
	int crossMax = 0;
	int visCount = 0;

	for (const auto& ch : m_children) {
		if (!ch.visible || !ch.component) continue;

		QSize desired(0, 0);
		if (auto* l = dynamic_cast<ILayoutable*>(ch.component)) {
			SizeConstraints childCs = {};
			if (isH) {
				childCs.minW = 0; childCs.minH = 0;
				childCs.maxW = std::numeric_limits<int>::max() / 2;
				childCs.maxH = crossMaxAvail;
			}
			else {
				childCs.minW = 0; childCs.minH = 0;
				childCs.maxW = crossMaxAvail;
				childCs.maxH = std::numeric_limits<int>::max() / 2;
			}
			desired = l->measure(childCs);
		}
		else {
			desired = ch.component->bounds().size();
			// 限制交叉轴
			if (isH) {
				desired.setHeight(std::min(std::max(0, desired.height()), crossMaxAvail));
			}
			else {
				desired.setWidth(std::min(std::max(0, desired.width()), crossMaxAvail));
			}
		}

		if (isH) {
			mainSum += std::max(0, desired.width());
			crossMax = std::max({ crossMax, 0, desired.height() });
		}
		else {
			mainSum += std::max(0, desired.height());
			crossMax = std::max({ crossMax, 0, desired.width() });
		}
		++visCount;
	}

	if (visCount > 1) mainSum += m_spacing * (visCount - 1);

	int outW, outH;
	if (isH) {
		outW = padW + mainSum;
		outH = padH + crossMax;
	}
	else {
		outW = padW + crossMax;
		outH = padH + mainSum;
	}

	outW = std::clamp(outW, cs.minW, cs.maxW);
	outH = std::clamp(outH, cs.minH, cs.maxH);
	return { outW, outH };
}

void UiPanel::arrange(const QRect& finalRect)
{
	// 仅设置 viewport，具体子项布局仍由 updateLayout 完成
	setViewportRect(finalRect);
}

void UiPanel::updateLayout(const QSize& windowSize)
{
	const QRect area = contentRect();
	m_childRects.assign(m_children.size(), QRect());

	if (!area.isValid() || m_children.empty())
	{
		for (const auto& ch : m_children)
		{
			if (auto* c = dynamic_cast<IUiContent*>(ch.component)) c->setViewportRect(QRect());
		}
		return;
	}

	const bool isH = (m_orient == Orientation::Horizontal);
	const int crossAvail = isH ? area.height() : area.width();
	const int mainAvail = isH ? area.width() : area.height();

	// 1) 测量所有可见子项
	std::vector<QSize> desired(m_children.size(), QSize(0, 0));
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		const auto& ch = m_children[i];
		if (!ch.visible || !ch.component) continue;
		desired[i] = measureChild(ch.component, crossAvail);
	}

	// 2) 排列：按测量尺寸依次放置（主轴不拉伸，交叉轴限制/可 Stretch）
	int cur = 0;
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		const auto& [component, crossAlign, visible] = m_children[i];
		if (!visible || !component) continue;

		const QRect r = placeChild(area, cur, desired[i], crossAlign);
		m_childRects[i] = r;

		// 前进主轴（加 spacing）
		cur += (isH ? r.width() : r.height());
		cur += m_spacing;
	}

	// 3) 将矩形下发：IUiContent -> viewport；ILayoutable -> arrange()；并推进子项 updateLayout
	for (size_t i = 0; i < m_children.size(); ++i)
	{
		const auto& ch = m_children[i];
		if (!ch.visible || !ch.component) continue;
		const QRect r = m_childRects[i];

		if (auto* c = dynamic_cast<IUiContent*>(ch.component))
		{
			c->setViewportRect(r);
		}
		if (auto* l = dynamic_cast<ILayoutable*>(ch.component))
		{
			l->arrange(r);
		}
		ch.component->updateLayout(windowSize);
	}
}

void UiPanel::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float devicePixelRatio)
{
	m_cache = &cache;
	m_gl = gl;
	m_dpr = std::max(0.5f, devicePixelRatio);
	for (const auto& ch : m_children) if (ch.component) ch.component->updateResourceContext(cache, gl, devicePixelRatio);
}

void UiPanel::append(Render::FrameData& fd) const
{
	// 背景在“去除 margin 的区域”内绘制（包含 padding）
	if (m_bg.alpha() > 0 && m_viewport.isValid())
	{
		const QRect bgRect = m_viewport.adjusted(
			m_margins.left(), m_margins.top(),
			-m_margins.right(), -m_margins.bottom());
		if (bgRect.isValid()) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(bgRect),
				.radiusPx = m_radius,
				.color = m_bg,
				.clipRect = QRectF(m_viewport) // 仍以整个 viewport 裁剪
				});
		}
	}

	const QRectF parentClip = QRectF(contentRect());

	for (size_t i = 0; i < m_children.size(); ++i) {
		const auto& ch = m_children[i];
		if (!ch.visible || !ch.component) continue;

		const int rr0 = static_cast<int>(fd.roundedRects.size());
		const int im0 = static_cast<int>(fd.images.size());

		ch.component->append(fd);

		RenderUtils::applyParentClip(fd, rr0, im0, parentClip);
	}
}

bool UiPanel::onMousePress(const QPoint& pos)
{
	if (!m_viewport.contains(pos)) return false;
	for (const auto& it : std::ranges::reverse_view(m_children))
	{
		if (it.visible && it.component && it.component->onMousePress(pos))
		{
			m_capture = it.component;
			return true;
		}
	}
	return false;
}

bool UiPanel::onMouseMove(const QPoint& pos)
{
	if (m_capture) return m_capture->onMouseMove(pos);
	bool any = false;
	for (const auto& it : std::ranges::reverse_view(m_children))
		if (it.visible && it.component) any = it.component->onMouseMove(pos) || any;
	return any;
}

bool UiPanel::onMouseRelease(const QPoint& pos)
{
	if (m_capture)
	{
		const bool h = m_capture->onMouseRelease(pos);
		m_capture = nullptr;
		return h;
	}
	for (const auto& it : std::ranges::reverse_view(m_children))
		if (it.visible && it.component && it.component->onMouseRelease(pos)) return true;
	return false;
}

bool UiPanel::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
	if (!m_viewport.contains(pos)) return false;
	for (const auto& it : std::ranges::reverse_view(m_children))
	{
		if (it.visible && it.component && it.component->onWheel(pos, angleDelta))
			return true;
	}
	return false;
}

bool UiPanel::tick()
{
	bool any = false;
	for (const auto& ch : m_children) if (ch.component) any = ch.component->tick() || any;
	return any;
}

void UiPanel::onThemeChanged(const bool isDark)
{
	for (const auto& ch : m_children) if (ch.component) ch.component->onThemeChanged(isDark);
}