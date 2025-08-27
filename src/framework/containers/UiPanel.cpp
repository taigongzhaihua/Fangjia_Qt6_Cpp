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
	const int mainAvail = isH ? area.width() : area.height();

	// 可见索引
	std::vector<size_t> vis;
	vis.reserve(m_children.size());
	for (size_t i = 0; i < m_children.size(); ++i) {
		if (m_children[i].visible && m_children[i].component) vis.push_back(i);
	}
	const int n = static_cast<int>(vis.size());
	if (n == 0) return;

	// 简便工具：在给定主轴上限下测量一个子项
	auto measureWithMainMax = [&](IUiComponent* c, int crossMax, int mainMax) -> QSize {
		if (auto* l = dynamic_cast<ILayoutable*>(c)) {
			SizeConstraints cs{};
			cs.minW = 0; cs.minH = 0;
			if (isH) {
				cs.maxW = (mainMax >= 0 ? std::max(0, mainMax) : std::numeric_limits<int>::max() / 2);
				cs.maxH = std::max(0, crossMax);
			}
			else {
				cs.maxW = std::max(0, crossMax);
				cs.maxH = (mainMax >= 0 ? std::max(0, mainMax) : std::numeric_limits<int>::max() / 2);
			}
			return l->measure(cs);
		}
		// 非 ILayoutable 视为固定：只在交叉轴裁剪
		QSize s = c->bounds().size();
		if (isH) s.setHeight(std::min(std::max(0, s.height()), std::max(0, crossMax)));
		else     s.setWidth(std::min(std::max(0, s.width()), std::max(0, crossMax)));
		return s;
		};

	// 1) 首次测量：偏好尺寸（主轴无限制，交叉轴受限）
	std::vector<QSize> desired(n, QSize(0, 0));
	std::vector<int>   prefMain(n, 0);
	std::vector<int>   prefCross(n, 0);
	std::vector<bool>  adjustable(n, false);

	for (int k = 0; k < n; ++k) {
		const auto idx = vis[k];
		IUiComponent* comp = m_children[idx].component;
		const QSize d = measureWithMainMax(comp, crossAvail, -1);
		desired[k] = d;
		prefMain[k] = std::max(0, isH ? d.width() : d.height());
		prefCross[k] = std::max(0, isH ? d.height() : d.width());
		adjustable[k] = (dynamic_cast<ILayoutable*>(comp) != nullptr);
	}

	const int spacingTotal = std::max(0, n - 1) * m_spacing;
	long long sumPref = spacingTotal;
	for (int k = 0; k < n; ++k) sumPref += prefMain[k];

	// 若偏好总长不超，直接放置
	if (sumPref <= mainAvail) {
		int cur = 0;
		for (int k = 0; k < n; ++k) {
			const auto idx = vis[k];
			const auto& ch = m_children[idx];
			const QSize d = desired[k];
			QRect r;
			if (isH) {
				const int hFit = std::max(0, std::min(d.height(), area.height()));
				int y = (ch.crossAlign == CrossAlign::Center) ? area.center().y() - hFit / 2 :
					(ch.crossAlign == CrossAlign::End) ? area.bottom() - hFit : area.top();
				r = QRect(area.left() + cur, y, d.width(), (ch.crossAlign == CrossAlign::Stretch ? area.height() : hFit));
				cur += d.width();
			}
			else {
				const int wFit = std::max(0, std::min(d.width(), area.width()));
				int x = (ch.crossAlign == CrossAlign::Center) ? area.center().x() - wFit / 2 :
					(ch.crossAlign == CrossAlign::End) ? area.right() - wFit : area.left();
				r = QRect(x, area.top() + cur, (ch.crossAlign == CrossAlign::Stretch ? area.width() : wFit), d.height());
				cur += d.height();
			}
			m_childRects[idx] = r;
			if (k < n - 1) cur += m_spacing;
		}
		// 下发（带裁剪）
		for (int k = 0; k < n; ++k) {
			const auto idx = vis[k];
			const auto& ch = m_children[idx];
			const QRect r = m_childRects[idx];
			const QRect clipped = r.intersected(area);
			if (auto* c = dynamic_cast<IUiContent*>(ch.component)) c->setViewportRect(clipped);
			if (auto* l = dynamic_cast<ILayoutable*>(ch.component)) l->arrange(clipped);
			ch.component->updateLayout(windowSize);
		}
		return;
	}

	// 2) 需要收缩：为每个子项求“最小主轴尺寸”能力（在当前交叉轴可用 crossAvail 下）
	std::vector<int> minMain(n, 0);
	for (int k = 0; k < n; ++k) {
		if (!adjustable[k]) {
			minMain[k] = prefMain[k]; // 不可调：最小即偏好
			continue;
		}
		IUiComponent* comp = m_children[vis[k]].component;
		// 二分搜索可行的最小主轴长度：高度不超过 crossAvail
		int lo = 1;
		int hi = std::max(1, prefMain[k]);
		int best = prefMain[k];
		// 若 crossAvail 为 0，则无法通过增高换宽，最小就是当前测到的宽度（不可缩）
		if (crossAvail <= 0) {
			minMain[k] = prefMain[k];
			continue;
		}
		for (int it = 0; it < 20 && lo <= hi; ++it) { // 20 次足够
			const int mid = (lo + hi) / 2;
			const QSize d = measureWithMainMax(comp, crossAvail, mid);
			const int usedMain = std::max(0, isH ? d.width() : d.height());
			const int usedCross = std::max(0, isH ? d.height() : d.width());
			// 在当前交叉轴限制下，是否能“把主轴压到 mid 以内”
			if (usedCross <= crossAvail && usedMain <= mid) {
				best = std::max(1, usedMain); // 记下可行的更小主轴
				hi = mid - 1;
			}
			else {
				lo = mid + 1;
			}
		}
		minMain[k] = std::clamp(best, 1, prefMain[k]);
	}

	// 3) 计算是否即便所有可调子项都缩到最小仍然越界
	long long sumMin = spacingTotal;
	for (int k = 0; k < n; ++k) sumMin += minMain[k];

	// 4) 在 [minMain, prefMain] 内做“水位填充”，分配最终主轴长度
	std::vector<int> finalMain(n, 0);
	if (sumMin >= mainAvail) {
		// 无法完全容纳：按最小放置，其余溢出部分后续用裁剪处理
		for (int k = 0; k < n; ++k) finalMain[k] = minMain[k];
	}
	else {
		// 可容纳：把剩余长度按“扩张能力”比例分配
		const long long capacityTotal = [&]() {
			long long s = 0;
			for (int k = 0; k < n; ++k) s += std::max(0, prefMain[k] - minMain[k]);
			return s;
			}();
		const long long budget = static_cast<long long>(mainAvail) - sumMin;

		// 初值 = min
		for (int k = 0; k < n; ++k) finalMain[k] = minMain[k];

		if (capacityTotal <= 0) {
			// 没有扩张能力，final 就是 min
		}
		else {
			long long used = 0;
			for (int k = 0; k < n; ++k) {
				const int cap = std::max(0, prefMain[k] - minMain[k]);
				if (cap <= 0) continue;
				const double share = static_cast<double>(cap) / static_cast<double>(capacityTotal);
				const int add = static_cast<int>(std::floor(share * static_cast<double>(budget)));
				finalMain[k] = std::min(prefMain[k], finalMain[k] + add);
				used += add;
			}
			// 由于取整，可能还有少量剩余，逐个 +1 补齐
			long long residual = std::max(0LL, budget - used);
			for (int k = 0; k < n && residual > 0; ++k) {
				if (finalMain[k] < prefMain[k]) { finalMain[k] += 1; --residual; }
			}
		}
	}

	// 5) 放置：按分配好的 main 尺寸依次放；交叉轴按对齐策略
	int cur = 0;
	for (int k = 0; k < n; ++k) {
		const auto idx = vis[k];
		const auto& ch = m_children[idx];

		// 为了更准确的交叉轴“需求”，在已分配 main 下再测一遍
		const QSize d2 = measureWithMainMax(ch.component, crossAvail, finalMain[k]);

		QRect r;
		if (isH) {
			const int w = std::max(0, std::min(finalMain[k], std::max(0, area.width() - cur)));
			const int hFit = std::max(0, std::min(d2.height(), area.height()));
			int y = (ch.crossAlign == CrossAlign::Center) ? area.center().y() - hFit / 2 :
				(ch.crossAlign == CrossAlign::End) ? area.bottom() - hFit : area.top();
			r = QRect(area.left() + cur, y, w, (ch.crossAlign == CrossAlign::Stretch ? area.height() : hFit));
			cur += finalMain[k];
		}
		else {
			const int h = std::max(0, std::min(finalMain[k], std::max(0, area.height() - cur)));
			const int wFit = std::max(0, std::min(d2.width(), area.width()));
			int x = (ch.crossAlign == CrossAlign::Center) ? area.center().x() - wFit / 2 :
				(ch.crossAlign == CrossAlign::End) ? area.right() - wFit : area.left();
			r = QRect(x, area.top() + cur, (ch.crossAlign == CrossAlign::Stretch ? area.width() : wFit), h);
			cur += finalMain[k];
		}
		m_childRects[idx] = r;
		if (k < n - 1) cur += m_spacing;
	}

	// 6) 下发：viewport 一律裁剪到父 contentRect 内；安排与布局更新
	for (int k = 0; k < n; ++k) {
		const auto idx = vis[k];
		const auto& ch = m_children[idx];
		const QRect r = m_childRects[idx];
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