#include "UiGrid.h"

#include <algorithm>
#include <numeric>
#include <ranges>
#include <RenderUtils.hpp>

void UiGrid::clearChildren() {
	m_children.clear();
	m_childRects.clear();
}

void UiGrid::addChild(IUiComponent* c, int row, int col, int rowSpan, int colSpan,
	const Align hAlign, const Align vAlign) {
	if (!c) return;
	row = std::max(row, 0);
	col = std::max(col, 0);
	rowSpan = std::max(1, rowSpan);
	colSpan = std::max(1, colSpan);
	m_children.push_back(Child{ .component = c, .row = row, .col = col,
		.rowSpan = rowSpan, .colSpan = colSpan,
		.hAlign = hAlign, .vAlign = vAlign, .visible = true
		});
	m_childRects.resize(m_children.size());
}

QRect UiGrid::contentRect() const {
	const QRect r = m_viewport.adjusted(
		m_margins.left() + m_padding.left(),
		m_margins.top() + m_padding.top(),
		-(m_margins.right() + m_padding.right()),
		-(m_margins.bottom() + m_padding.bottom()));
	if (r.width() < 0 || r.height() < 0) return {};
	return r;
}

void UiGrid::ensureTrackSize(const int minRows, const int minCols) const {
	if (static_cast<int>(m_rows.size()) < minRows) {
		m_rows.resize(minRows, TrackDef::Auto());
	}
	if (static_cast<int>(m_cols.size()) < minCols) {
		m_cols.resize(minCols, TrackDef::Auto());
	}
}

// ====================== 测量辅助 ======================
QSize UiGrid::measureChildNatural(IUiComponent* c) const {
	if (!c) return { 0,0 };
	if (auto* l = dynamic_cast<ILayoutable*>(c)) {
		SizeConstraints cs;
		cs.maxW = std::numeric_limits<int>::max() / 4;
		cs.maxH = std::numeric_limits<int>::max() / 4;
		return l->measure(cs);
	}
	return c->bounds().size();
}

QSize UiGrid::measureChildWidthBound(IUiComponent* c, const int maxW) const {
	if (!c) return { 0,0 };
	if (auto* l = dynamic_cast<ILayoutable*>(c)) {
		SizeConstraints cs;
		cs.minW = 0; cs.minH = 0;
		cs.maxW = std::max(0, maxW);
		cs.maxH = std::numeric_limits<int>::max() / 4;
		return l->measure(cs);
	}
	QSize s = c->bounds().size();
	s.setWidth(std::min(std::max(0, s.width()), std::max(0, maxW)));
	return s;
}

// ====================== 列宽/行高求解 ======================
std::vector<int> UiGrid::computeColumnWidths(const int contentW) const {
	const int n = static_cast<int>(m_cols.size());
	if (n <= 0) return {};

	std::vector width(n, 0);
	std::vector starWeight(n, 0.0f);
	std::vector starMin(n, 0);

	// 预填：像素列固定，Star 权重记录
	for (int i = 0; i < n; ++i) {
		const auto& d = m_cols[i];
		if (d.type == TrackDef::Type::Pixel) {
			width[i] = static_cast<int>(std::round(std::max(0.0f, d.value)));
		}
		else if (d.type == TrackDef::Type::Star) {
			starWeight[i] = std::max(0.0f, d.value <= 0.0f ? 1.0f : d.value);
		}
	}

	// 第一轮：单列子项（colSpan==1）填充 Auto 与 Star 的最小内容宽
	for (const auto& ch : m_children) {
		if (!ch.visible || !ch.component) continue;
		if (ch.col < 0 || ch.col >= n) continue;
		if (ch.colSpan != 1) continue;

		const QSize nat = measureChildNatural(ch.component);
		const int col = ch.col;
		const auto& def = m_cols[col];
		if (def.type == TrackDef::Type::Auto) {
			width[col] = std::max(width[col], nat.width());
		}
		else if (def.type == TrackDef::Type::Star) {
			starMin[col] = std::max(starMin[col], nat.width());
		}
	}

	// 第二轮：跨列子项，拉升所跨列的 Auto/Star 最小需求
	for (const auto& ch : m_children) {
		if (!ch.visible || !ch.component) continue;
		if (ch.col < 0 || ch.col >= n) continue;
		const int c0 = ch.col;
		const int c1 = std::min(n, c0 + ch.colSpan) - 1;
		if (c1 < c0) continue;

		const QSize nat = measureChildNatural(ch.component);
		// 现有固定和最小总和
		int sum = 0; float sumStarW = 0.0f; int autoCount = 0; int starCount = 0;
		for (int c = c0; c <= c1; ++c) {
			if (const auto& [type, value] = m_cols[c]; type == TrackDef::Type::Pixel) sum += width[c];
			else if (type == TrackDef::Type::Auto) { sum += width[c]; autoCount++; }
			else { sum += starMin[c]; sumStarW += starWeight[c]; starCount++; }
		}
		// 加上列内间距
		sum += std::max(0, ch.colSpan - 1) * m_colSpacing;

		const int need = nat.width() - sum;
		if (need <= 0) continue;

		if (sumStarW > 0.0f) {
			for (int c = c0; c <= c1; ++c) {
				if (m_cols[c].type == TrackDef::Type::Star) {
					const float w = (starWeight[c] <= 0.0f ? 1.0f : starWeight[c]);
					const int add = static_cast<int>(std::floor(static_cast<float>(need) * (w / sumStarW)));
					starMin[c] += add;
				}
			}
			// 余数补到最后一个 Star
			int distributed = 0;
			for (int c = c0; c <= c1; ++c)
				if (m_cols[c].type == TrackDef::Type::Star)
					distributed += static_cast<int>(std::floor(static_cast<float>(need) * ((starWeight[c] <= 0.0f ? 1.0f : starWeight[c]) / sumStarW)));
			int rem = need - distributed;
			for (int c = c1; c >= c0 && rem > 0; --c) if (m_cols[c].type == TrackDef::Type::Star) { starMin[c] += rem; rem = 0; }
		}
		else if (autoCount > 0) {
			const int each = need / autoCount;
			for (int c = c0; c <= c1; ++c) if (m_cols[c].type == TrackDef::Type::Auto) width[c] += each;
			int rem = need - each * autoCount;
			for (int c = c1; c >= c0 && rem > 0; --c) if (m_cols[c].type == TrackDef::Type::Auto) { width[c] += 1; --rem; }
		}
		else {
			// 只有 Pixel：放到最后一列（避免复杂收缩）
			width[c1] += need;
		}
	}

	// 汇总固定最小宽
	int fixed = 0;
	for (int i = 0; i < n; ++i) {
		fixed += (m_cols[i].type == TrackDef::Type::Star ? starMin[i] : width[i]);
	}
	fixed += std::max(0, n - 1) * m_colSpacing;

	// 分配 Star 剩余
	const int avail = std::max(0, contentW - fixed);
	const float totalStar = std::accumulate(starWeight.begin(), starWeight.end(), 0.0f);
	std::vector out(n, 0);
	for (int i = 0; i < n; ++i) {
		if (m_cols[i].type == TrackDef::Type::Star) {
			int add = (totalStar > 0.0f) ? static_cast<int>(std::floor(avail * (starWeight[i] / totalStar))) : 0;
			out[i] = starMin[i] + add;
		}
		else {
			out[i] = width[i];
		}
	}
	// 处理分配误差
	int sumOut = std::accumulate(out.begin(), out.end(), 0);
	int error = (fixed - std::accumulate(width.begin(), width.end(), 0) - std::accumulate(starMin.begin(), starMin.end(), 0)) + (avail - (sumOut - (fixed - std::max(0, n - 1) * m_colSpacing)));
	// 把误差补到最后一个 Star/Auto
	for (int i = n - 1; i >= 0 && error != 0; --i) {
		if (m_cols[i].type == TrackDef::Type::Star) { out[i] += error; error = 0; break; }
	}

	return out;
}

std::vector<int> UiGrid::computeRowHeights(const int contentH, const std::vector<int>& colW) const {
	const int rN = static_cast<int>(m_rows.size());
	const int cN = static_cast<int>(m_cols.size());
	if (rN <= 0 || cN <= 0) return {};

	std::vector height(rN, 0);
	std::vector starWeight(rN, 0.0f);
	std::vector starMin(rN, 0);

	for (int r = 0; r < rN; ++r) {
		const auto& d = m_rows[r];
		if (d.type == TrackDef::Type::Pixel) height[r] = static_cast<int>(std::round(std::max(0.0f, d.value)));
		else if (d.type == TrackDef::Type::Star) starWeight[r] = std::max(0.0f, d.value <= 0.0f ? 1.0f : d.value);
	}

	auto spanWidth = [&](const int col, const int colSpan) -> int {
		if (col < 0 || col >= cN) return 0;
		const int c1 = std::min(cN, col + std::max(1, colSpan)) - 1;
		int w = 0;
		for (int c = col; c <= c1; ++c) w += (c >= 0 && c < (int)colW.size() ? colW[c] : 0);
		w += std::max(0, c1 - col) * m_colSpacing;
		return w;
		};

	// 单行子项：Auto/Star 记录最小高度（根据已知跨列宽测量）
	for (const auto& ch : m_children) {
		if (!ch.visible || !ch.component) continue;
		if (ch.row < 0 || ch.row >= rN) continue;
		if (ch.rowSpan != 1) continue;
		const int maxW = spanWidth(ch.col, ch.colSpan);
		const QSize d = measureChildWidthBound(ch.component, maxW);

		const auto& def = m_rows[ch.row];
		if (def.type == TrackDef::Type::Auto) height[ch.row] = std::max(height[ch.row], d.height());
		else if (def.type == TrackDef::Type::Star) starMin[ch.row] = std::max(starMin[ch.row], d.height());
	}

	// 跨行子项：填充 Star/Auto 最小
	for (const auto& ch : m_children) {
		if (!ch.visible || !ch.component) continue;
		if (ch.row < 0 || ch.row >= rN) continue;
		const int r0 = ch.row;
		const int r1 = std::min(rN, r0 + ch.rowSpan) - 1;
		if (r1 < r0) continue;

		const int maxW = spanWidth(ch.col, ch.colSpan);
		const QSize d = measureChildWidthBound(ch.component, maxW);

		int sum = 0; float sumStarW = 0.0f; int autoCount = 0;
		for (int r = r0; r <= r1; ++r) {
			const auto& def = m_rows[r];
			if (def.type == TrackDef::Type::Pixel) sum += height[r];
			else if (def.type == TrackDef::Type::Auto) { sum += height[r]; autoCount++; }
			else { sum += starMin[r]; sumStarW += starWeight[r]; }
		}
		sum += std::max(0, ch.rowSpan - 1) * m_rowSpacing;

		int need = d.height() - sum;
		if (need <= 0) continue;

		if (sumStarW > 0.0f) {
			for (int r = r0; r <= r1; ++r) if (m_rows[r].type == TrackDef::Type::Star) {
				const float w = (starWeight[r] <= 0.0f ? 1.0f : starWeight[r]);
				const int add = static_cast<int>(std::floor(static_cast<float>(need) * (w / sumStarW)));
				starMin[r] += add;
			}
			int distributed = 0;
			for (int r = r0; r <= r1; ++r)
				if (m_rows[r].type == TrackDef::Type::Star)
					distributed += static_cast<int>(std::floor(static_cast<float>(need) * ((starWeight[r] <= 0.0f ? 1.0f : starWeight[r]) / sumStarW)));
			int rem = need - distributed;
			for (int r = r1; r >= r0 && rem > 0; --r) if (m_rows[r].type == TrackDef::Type::Star) { starMin[r] += rem; rem = 0; }
		}
		else if (autoCount > 0) {
			const int each = need / autoCount;
			for (int r = r0; r <= r1; ++r) if (m_rows[r].type == TrackDef::Type::Auto) height[r] += each;
			int rem = need - each * autoCount;
			for (int r = r1; r >= r0 && rem > 0; --r) if (m_rows[r].type == TrackDef::Type::Auto) { height[r] += 1; --rem; }
		}
		else {
			height[r1] += need; // 只有 Pixel：补到最后一行
		}
	}

	int fixed = 0;
	for (int r = 0; r < rN; ++r) fixed += (m_rows[r].type == TrackDef::Type::Star ? starMin[r] : height[r]);
	fixed += std::max(0, rN - 1) * m_rowSpacing;

	const int avail = std::max(0, contentH - fixed);
	const float totalStar = std::accumulate(starWeight.begin(), starWeight.end(), 0.0f);

	std::vector out(rN, 0);
	for (int r = 0; r < rN; ++r) {
		if (m_rows[r].type == TrackDef::Type::Star) {
			int add = (totalStar > 0.0f) ? static_cast<int>(std::floor(avail * (starWeight[r] / totalStar))) : 0;
			out[r] = starMin[r] + add;
		}
		else {
			out[r] = height[r];
		}
	}
	// 误差补偿
	int sumOut = std::accumulate(out.begin(), out.end(), 0);
	int error = (fixed - std::accumulate(height.begin(), height.end(), 0) - std::accumulate(starMin.begin(), starMin.end(), 0)) + (avail - (sumOut - (fixed - std::max(0, rN - 1) * m_rowSpacing)));
	for (int r = rN - 1; r >= 0 && error != 0; --r) {
		if (m_rows[r].type == TrackDef::Type::Star) { out[r] += error; error = 0; break; }
	}
	return out;
}

// ====================== ILayoutable ======================
QSize UiGrid::measure(const SizeConstraints& cs) {
	// 估算可用宽高（无上限时给个合理默认，用于推导 Star 分配）
	int maxW = cs.maxW, maxH = cs.maxH;
	if (maxW >= std::numeric_limits<int>::max() / 4) {
		// 给出一个温和默认：累计 Pixel + 其余每列 120，再加间距
		int pxSum = 0, others = 0;
		for (const auto& c : m_cols) {
			if (c.type == TrackDef::Type::Pixel) pxSum += static_cast<int>(std::round(c.value));
			else others++;
		}
		maxW = m_margins.left() + m_margins.right() + m_padding.left() + m_padding.right()
			+ pxSum + others * 120 + std::max(0, (int)m_cols.size() - 1) * m_colSpacing;
	}
	if (maxH >= std::numeric_limits<int>::max() / 4) {
		int pxSum = 0, others = 0;
		for (const auto& r : m_rows) {
			if (r.type == TrackDef::Type::Pixel) pxSum += static_cast<int>(std::round(r.value));
			else others++;
		}
		maxH = m_margins.top() + m_margins.bottom() + m_padding.top() + m_padding.bottom()
			+ pxSum + others * 40 + std::max(0, (int)m_rows.size() - 1) * m_rowSpacing;
	}

	// 确保行列足够覆盖子项
	int needRows = 0, needCols = 0;
	for (const auto& ch : m_children) {
		needRows = std::max(needRows, ch.row + ch.rowSpan);
		needCols = std::max(needCols, ch.col + ch.colSpan);
	}
	ensureTrackSize(needRows, needCols);

	const int padW = m_margins.left() + m_margins.right() + m_padding.left() + m_padding.right();
	const int padH = m_margins.top() + m_margins.bottom() + m_padding.top() + m_padding.bottom();

	const int contentW = std::max(0, maxW - padW);
	const std::vector<int> colW = computeColumnWidths(contentW);

	// 行高基于列宽再测一轮
	const int contentH = std::max(0, maxH - padH);
	const std::vector<int> rowH = computeRowHeights(contentH, colW);

	const int totalW = padW + std::accumulate(colW.begin(), colW.end(), 0) + std::max(0, (int)colW.size() - 1) * m_colSpacing;
	const int totalH = padH + std::accumulate(rowH.begin(), rowH.end(), 0) + std::max(0, (int)rowH.size() - 1) * m_rowSpacing;

	const int outW = std::clamp(totalW, cs.minW, cs.maxW);
	const int outH = std::clamp(totalH, cs.minH, cs.maxH);
	return { outW, outH };
}

void UiGrid::updateLayout(const QSize& windowSize) {
	const QRect area = contentRect();
	m_childRects.assign(m_children.size(), QRect());

	// 保障行列大小
	int needRows = 0, needCols = 0;
	for (const auto& ch : m_children) {
		needRows = std::max(needRows, ch.row + ch.rowSpan);
		needCols = std::max(needCols, ch.col + ch.colSpan);
	}
	ensureTrackSize(needRows, needCols);

	if (!area.isValid() || m_rows.empty() || m_cols.empty()) {
		for (const auto& ch : m_children) {
			if (auto* c = dynamic_cast<IUiContent*>(ch.component)) c->setViewportRect(QRect());
		}
		return;
	}

	const int contentW = area.width();
	const int contentH = area.height();

	const std::vector<int> colW = computeColumnWidths(contentW);
	const std::vector<int> rowH = computeRowHeights(contentH, colW);

	// 生成列/行偏移
	const int nC = static_cast<int>(colW.size());
	const int nR = static_cast<int>(rowH.size());

	std::vector<int> x(nC, 0), y(nR, 0);
	int acc = area.left();
	for (int c = 0; c < nC; ++c) {
		x[c] = acc;
		acc += colW[c];
		if (c + 1 < nC) acc += m_colSpacing;
	}
	acc = area.top();
	for (int r = 0; r < nR; ++r) {
		y[r] = acc;
		acc += rowH[r];
		if (r + 1 < nR) acc += m_rowSpacing;
	}

	auto spanSize = [&](const int start, const int span, const std::vector<int>& arr, const int gap) {
		if (start < 0 || start >= (int)arr.size() || span <= 0) return 0;
		const int end = std::min(static_cast<int>(arr.size()), start + span) - 1;
		int s = 0;
		for (int i = start; i <= end; ++i) s += arr[i];
		s += std::max(0, end - start) * gap;
		return s;
		};

	for (size_t i = 0; i < m_children.size(); ++i) {
		const auto& ch = m_children[i];
		if (!ch.visible || !ch.component) continue;
		if (ch.row < 0 || ch.row >= nR || ch.col < 0 || ch.col >= nC) continue;

		const int cw = spanSize(ch.col, ch.colSpan, colW, m_colSpacing);
		const int chh = spanSize(ch.row, ch.rowSpan, rowH, m_rowSpacing);

		const int cellX = x[ch.col];
		const int cellY = y[ch.row];

		const QRect cell(cellX, cellY, cw, chh);

		// 再测一次期望尺寸（结合跨列宽度约束）
		const QSize desired = measureChildWidthBound(ch.component, cw);

		const QRect r = placeInCell(cell, desired, ch.hAlign, ch.vAlign);
		m_childRects[i] = r;

		if (auto* c = dynamic_cast<IUiContent*>(ch.component)) c->setViewportRect(r);
		if (auto* l = dynamic_cast<ILayoutable*>(ch.component)) l->arrange(r);
		ch.component->updateLayout(windowSize);
	}
}

void UiGrid::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, const float devicePixelRatio) {
	m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
	for (const auto& ch : m_children) if (ch.component) ch.component->updateResourceContext(loader, gl, devicePixelRatio);
}

void UiGrid::append(Render::FrameData& fd) const {
	const QRectF parentClip = QRectF(contentRect());

	for (const auto& ch : m_children) {
		if (!ch.visible || !ch.component) continue;

		const int rr0 = static_cast<int>(fd.roundedRects.size());
		const int im0 = static_cast<int>(fd.images.size());

		ch.component->append(fd);

		RenderUtils::applyParentClip(fd, rr0, im0, parentClip);
	}
}

bool UiGrid::onMousePress(const QPoint& pos) {
	if (!m_viewport.contains(pos)) return false;
	for (const auto& it : std::ranges::reverse_view(m_children)) {
		if (!it.visible || !it.component) continue;
		if (it.component->onMousePress(pos)) { m_capture = it.component; return true; }
	}
	return false;
}
bool UiGrid::onMouseMove(const QPoint& pos) {
	if (m_capture) return m_capture->onMouseMove(pos);
	bool any = false;
	for (const auto& it : std::ranges::reverse_view(m_children)) {
		if (!it.visible || !it.component) continue;
		any = it.component->onMouseMove(pos) || any;
	}
	return any;
}
bool UiGrid::onMouseRelease(const QPoint& pos) {
	if (m_capture) {
		const bool h = m_capture->onMouseRelease(pos);
		m_capture = nullptr;
		return h;
	}
	for (const auto& it : std::ranges::reverse_view(m_children)) {
		if (!it.visible || !it.component) continue;
		if (it.component->onMouseRelease(pos)) return true;
	}
	return false;
}

bool UiGrid::tick() {
	bool any = false;
	for (const auto& ch : m_children) if (ch.component) any = ch.component->tick() || any;
	return any;
}

void UiGrid::onThemeChanged(const bool isDark) {
	for (const auto& ch : m_children) if (ch.component) ch.component->onThemeChanged(isDark);
}

QRect UiGrid::placeInCell(const QRect& cell, const QSize& desired, const Align h, const Align v) const {
	const int availW = std::max(0, cell.width());
	const int availH = std::max(0, cell.height());

	int w = std::max(0, desired.width());
	int hgt = std::max(0, desired.height());

	if (h == Align::Stretch) w = availW; else w = std::min(w, availW);
	if (v == Align::Stretch) hgt = availH; else hgt = std::min(hgt, availH);

	int x = cell.left();
	switch (h) {
	case Align::Start:   x = cell.left(); break;
	case Align::Center:  x = cell.left() + (availW - w) / 2; break;
	case Align::End:     x = cell.right() - w; break;
	case Align::Stretch: x = cell.left(); break;
	}

	int y = cell.top();
	switch (v) {
	case Align::Start:   y = cell.top(); break;
	case Align::Center:  y = cell.top() + (availH - hgt) / 2; break;
	case Align::End:     y = cell.bottom() - hgt; break;
	case Align::Stretch: y = cell.top(); break;
	}
	return QRect(x, y, std::max(0, w), std::max(0, hgt));
}