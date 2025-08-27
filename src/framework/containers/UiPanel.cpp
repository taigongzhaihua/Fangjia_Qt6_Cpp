#include "UiPanel.h"
#include <algorithm>

#include "ILayoutable.hpp"
#include <cmath>
#include <IconLoader.h>
#include <limits>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <ranges>
#include <RenderData.hpp>
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
			SizeConstraints childCs{};
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

	// 可见子项索引
	std::vector<size_t> vis;
	vis.reserve(m_children.size());
	for (size_t i = 0; i < m_children.size(); ++i) {
		if (m_children[i].visible && m_children[i].component) vis.push_back(i);
	}
	const int nVis = static_cast<int>(vis.size());

	// 交叉轴尺寸测量 + 主轴偏好与“最小”测量（可调子项才有 min<pref）
	std::vector desired(nVis, QSize(0, 0));
	std::vector prefMain(nVis, 0);
	std::vector minMain(nVis, 0);
	std::vector adjustable(nVis, false);
	int spacingTotal = std::max(0, nVis - 1) * m_spacing;

	auto measureWithMainMax = [&](IUiComponent* c, const int crossAvailLimit, const int mainMax) -> QSize {
		if (auto* l = dynamic_cast<ILayoutable*>(c)) {
			SizeConstraints cs;
			cs.minW = 0; cs.minH = 0;
			if (isH) {
				cs.maxW = (mainMax >= 0 ? std::max(0, mainMax) : std::numeric_limits<int>::max() / 2);
				cs.maxH = std::max(0, crossAvailLimit);
			}
			else {
				cs.maxW = std::max(0, crossAvailLimit);
				cs.maxH = (mainMax >= 0 ? std::max(0, mainMax) : std::numeric_limits<int>::max() / 2);
			}
			return l->measure(cs);
		}
		// 非 ILayoutable：固定尺寸，受交叉轴限制
		QSize s = c->bounds().size();
		if (isH) {
			s.setHeight(std::min(std::max(0, s.height()), std::max(0, crossAvailLimit)));
		}
		else {
			s.setWidth(std::min(std::max(0, s.width()), std::max(0, crossAvailLimit)));
		}
		return s;
		};

	// 首次测量：偏好尺寸（主轴无限制，交叉轴受限）
	for (int k = 0; k < nVis; ++k) {
		const auto idx = vis[k];
		IUiComponent* comp = m_children[idx].component;
		desired[k] = measureWithMainMax(comp, crossAvail, -1); // -1 表示主轴无上限
		prefMain[k] = std::max(0, isH ? desired[k].width() : desired[k].height());
		adjustable[k] = (dynamic_cast<ILayoutable*>(comp) != nullptr);
	}

	// 可调子项的“最小”尺寸：把主轴上限设为 0 来探测（大多数组件会返回 0 或更小的自然最小值）
	for (int k = 0; k < nVis; ++k) {
		if (adjustable[k]) {
			const auto idx = vis[k];
			IUiComponent* comp = m_children[idx].component;
			const QSize mn = measureWithMainMax(comp, crossAvail, 0);
			minMain[k] = std::max(0, isH ? mn.width() : mn.height());
		}
		else {
			minMain[k] = prefMain[k]; // 固定子项：不可缩
		}
	}

	// 目标主轴可用空间（剔除 spacing）
	const int mainAvail = std::max(0, (isH ? area.width() : area.height()));
	const int mainAvailForSizes = std::max(0, mainAvail - spacingTotal);

	// 计算总偏好和总最小
	long long sumPref = 0, sumMin = 0, sumFixedPref = 0, sumCaps = 0;
	for (int k = 0; k < nVis; ++k) {
		sumPref += prefMain[k];
		sumMin += minMain[k];
		if (!adjustable[k]) sumFixedPref += prefMain[k];
		else sumCaps += std::max(0, prefMain[k] - minMain[k]);
	}

	// 最终分配的主轴尺寸
	std::vector finalMain(nVis, 0);

	if (sumPref <= mainAvailForSizes) {
		// 全部按偏好
		for (int k = 0; k < nVis; ++k) finalMain[k] = prefMain[k];
	}
	else {
		// 需要收缩
		const long long budgetForAdjustables = std::max(0LL, static_cast<long long>(mainAvailForSizes) - sumFixedPref);
		if (budgetForAdjustables <= 0) {
			// 固定项已吃满甚至溢出：可调项全退到最小
			for (int k = 0; k < nVis; ++k) finalMain[k] = minMain[k];
		}
		else {
			// 线性配比：在 [min, pref] 之间分配总预算
			// 初值设为 min
			long long sumMinAdjustables = 0;
			for (int k = 0; k < nVis; ++k) {
				if (adjustable[k]) sumMinAdjustables += minMain[k];
			}
			long long remaining = std::max(0LL, budgetForAdjustables - sumMinAdjustables);
			// caps = 每个可调的最大可增长量
			long long capsTotal = 0;
			for (int k = 0; k < nVis; ++k) {
				if (adjustable[k]) capsTotal += std::max(0, prefMain[k] - minMain[k]);
			}
			for (int k = 0; k < nVis; ++k) {
				if (!adjustable[k]) {
					finalMain[k] = prefMain[k];
				}
				else {
					const int cap = std::max(0, prefMain[k] - minMain[k]);
					int add = 0;
					if (capsTotal > 0 && remaining > 0 && cap > 0) {
						// 按比例分配
						const double share = static_cast<double>(cap) / static_cast<double>(capsTotal);
						add = static_cast<int>(std::floor(share * static_cast<double>(remaining)));
					}
					finalMain[k] = std::clamp(minMain[k] + add, minMain[k], prefMain[k]);
				}
			}
			// 由于取整，可能有少量剩余，把剩余 +1 逐个补齐
			long long used = 0;
			for (int k = 0; k < nVis; ++k) if (adjustable[k]) used += (finalMain[k] - minMain[k]);
			long long residual = std::max(0LL, remaining - used);
			for (int k = 0; k < nVis && residual > 0; ++k) {
				if (adjustable[k] && finalMain[k] < prefMain[k]) {
					finalMain[k] += 1;
					--residual;
				}
			}
		}
	}

	// 2) 排列：按分配好的主轴尺寸依次放
	int cur = 0;
	for (int k = 0; k < nVis; ++k)
	{
		const auto idx = vis[k];
		const auto& ch = m_children[idx];
		const QSize d = desired[k];
		const int mainSize = std::max(0, finalMain[k]);

		QRect childRect;
		if (isH) {
			// 交叉轴：尽量不拉伸，除非是 Stretch
			const int hFit = std::max(0, std::min(d.height(), area.height()));
			int y;
			switch (ch.crossAlign) {
			case CrossAlign::Center: y = area.center().y() - hFit / 2; break;
			case CrossAlign::End:    y = area.bottom() - hFit;         break;
			case CrossAlign::Stretch:
			default:                 y = area.top();                    break;
			}
			childRect = QRect(area.left() + cur, y, mainSize, (ch.crossAlign == CrossAlign::Stretch ? area.height() : hFit));
		}
		else {
			const int wFit = std::max(0, std::min(d.width(), area.width()));
			int x;
			switch (ch.crossAlign) {
			case CrossAlign::Center: x = area.center().x() - wFit / 2; break;
			case CrossAlign::End:    x = area.right() - wFit;          break;
			case CrossAlign::Stretch:
			default:                 x = area.left();                   break;
			}
			childRect = QRect(x, area.top() + cur, (ch.crossAlign == CrossAlign::Stretch ? area.width() : wFit), mainSize);
		}

		m_childRects[idx] = childRect;
		cur += mainSize;
		if (mainSize > 0 && k < nVis - 1) cur += m_spacing;
	}

	// 3) 下发矩形与子布局
	for (int k = 0; k < nVis; ++k)
	{
		const auto idx = vis[k];
		const auto& ch = m_children[idx];
		const QRect r = m_childRects[idx];

		// 关键：把下发给子项的 viewport 裁剪到父 panel contentRect 内
		const QRect clipped = r.intersected(area);

		if (auto* c = dynamic_cast<IUiContent*>(ch.component)) c->setViewportRect(clipped);
		if (auto* l = dynamic_cast<ILayoutable*>(ch.component)) l->arrange(clipped);
		ch.component->updateLayout(windowSize);
	}
}

void UiPanel::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, const float devicePixelRatio)
{
	m_loader = &loader;
	m_gl = gl;
	m_dpr = std::max(0.5f, devicePixelRatio);
	for (const auto& ch : m_children) if (ch.component) ch.component->updateResourceContext(loader, gl, devicePixelRatio);
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
	for (const auto& ch : m_children) if (ch.visible && ch.component) ch.component->append(fd);
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