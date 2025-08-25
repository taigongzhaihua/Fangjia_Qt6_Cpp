#include "UiBoxLayout.h"
#include <algorithm>
#include <cmath>
#include <IconLoader.h>
#include <numeric>
#include <qcolor.h>
#include <qlogging.h>
#include <qmargins.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <vector>

UiBoxLayout::UiBoxLayout(Direction dir)
	: m_direction(dir)
{
}

void UiBoxLayout::addChild(IUiComponent* component, float weight, Alignment align)
{
	if (!component) return;

	// 检查是否已存在
	const auto it = std::ranges::find_if(m_children,
		[component](const ChildItem& item) { return item.component == component; });

	if (it == m_children.end()) {
		m_children.push_back({ component, weight, align, true });
	}
}

void UiBoxLayout::insertChild(size_t index, IUiComponent* component, float weight, Alignment align)
{
	if (!component) return;

	// 检查是否已存在
	const auto it = std::ranges::find_if(m_children,
		[component](const ChildItem& item) { return item.component == component; });

	if (it == m_children.end()) {
		index = std::min(index, m_children.size());
		m_children.insert(m_children.begin() + index, { component, weight, align, true });
	}
}

void UiBoxLayout::removeChild(IUiComponent* component)
{
	const auto it = std::ranges::remove_if(m_children,
		[component](const ChildItem& item) { return item.component == component; }).begin();

	if (it != m_children.end()) {
		m_children.erase(it, m_children.end());
	}
}

void UiBoxLayout::removeChildAt(size_t index)
{
	if (index < m_children.size()) {
		m_children.erase(m_children.begin() + index);
	}
}

void UiBoxLayout::clearChildren()
{
	m_children.clear();
	m_childRects.clear();
}

IUiComponent* UiBoxLayout::childAt(size_t index) const
{
	if (index < m_children.size()) {
		return m_children[index].component;
	}
	return nullptr;
}

void UiBoxLayout::setDirection(Direction dir)
{
	if (m_direction != dir) {
		m_direction = dir;
		calculateLayout();
	}
}

void UiBoxLayout::setChildVisible(size_t index, bool visible)
{
	if (index < m_children.size()) {
		m_children[index].visible = visible;
		calculateLayout();
	}
}

bool UiBoxLayout::isChildVisible(size_t index) const
{
	if (index < m_children.size()) {
		return m_children[index].visible;
	}
	return false;
}

QRect UiBoxLayout::contentRect() const
{
	return m_viewport.adjusted(
		m_margins.left(),
		m_margins.top(),
		-m_margins.right(),
		-m_margins.bottom()
	);
}

void UiBoxLayout::calculateLayout()
{
	m_childRects.clear();
	m_childRects.resize(m_children.size());

	const QRect content = contentRect();
	if (!content.isValid() || m_children.empty()) return;

	// 收集可见子项
	std::vector<size_t> vis;
	float totalWeight = 0.0f;
	for (size_t i = 0; i < m_children.size(); ++i) {
		if (m_children[i].visible && m_children[i].component) {
			vis.push_back(i);
			totalWeight += std::max(0.0f, m_children[i].weight);
		}
	}
	if (vis.empty()) return;

	const bool isH = (m_direction == Direction::Horizontal);
	const int availableMain = isH ? content.width() : content.height();
	const int availableCross = isH ? content.height() : content.width();

	// 预计算“固定尺寸”占用和每个子项最终主轴尺寸
	std::vector<int> mainSizes(vis.size(), 0);
	int usedFixed = 0;
	for (size_t k = 0; k < vis.size(); ++k) {
		const auto idx = vis[k];
		const QRect pref = m_children[idx].component->bounds();
		const int prefMain = isH ? pref.width() : pref.height();
		if (m_children[idx].weight <= 0.0f) {
			const int s = std::max(0, std::min(prefMain, availableMain));
			mainSizes[k] = s;
			usedFixed += s;
		}
	}

	// 为“有权重”的子项分配空间
	const int baseGapCount = std::max<int>(0, static_cast<int>(vis.size()) - 1);
	// Start/Center/End 模式下，预留固定间距；Space* 模式下，间距由剩余空间决定
	const bool fixedGapMode =
		(m_mainAlign == MainAlignment::Start ||
			m_mainAlign == MainAlignment::Center ||
			m_mainAlign == MainAlignment::End);

	const int reservedGaps = fixedGapMode ? (m_spacing * baseGapCount) : 0;
	int flexibleSpace = std::max(0, availableMain - usedFixed - reservedGaps);

	// 按权重比例分配
	float acc = 0.0f;
	float assigned = 0.0f;
	for (size_t k = 0; k < vis.size(); ++k) {
		const auto idx = vis[k];
		const float w = std::max(0.0f, m_children[idx].weight);
		if (w > 0.0f && totalWeight > 0.0f) {
			acc += w / totalWeight * static_cast<float>(flexibleSpace);
			int s = static_cast<int>(std::floor(acc - assigned));
			s = std::max(0, s);
			mainSizes[k] = s;
			assigned += static_cast<float>(s);
		}
	}

	// 现在我们有每个子项的主轴尺寸，计算主轴剩余空间以做整体对齐/动态间距
	const int sumMain = std::accumulate(mainSizes.begin(), mainSizes.end(), 0);
	int remaining = std::max(0, availableMain - sumMain - reservedGaps);

	// 计算起始偏移与间距
	double gap = static_cast<double>(m_spacing);
	double startOffset = 0.0;
	const int n = static_cast<int>(vis.size());
	switch (m_mainAlign) {
	case MainAlignment::Start:
		startOffset = 0.0;
		gap = static_cast<double>(m_spacing);
		break;
	case MainAlignment::Center:
		startOffset = remaining * 0.5;
		gap = static_cast<double>(m_spacing);
		break;
	case MainAlignment::End:
		startOffset = remaining;
		gap = static_cast<double>(m_spacing);
		break;
	case MainAlignment::SpaceBetween:
		if (n > 1) {
			gap = static_cast<double>(remaining) / static_cast<double>(n - 1);
			startOffset = 0.0;
		}
		else {
			gap = 0.0;
			startOffset = remaining * 0.5;
		}
		break;
	case MainAlignment::SpaceAround:
		if (n > 0) {
			gap = static_cast<double>(remaining) / static_cast<double>(n);
			startOffset = gap * 0.5;
		}
		break;
	case MainAlignment::SpaceEvenly:
		gap = static_cast<double>(remaining) / static_cast<double>(n + 1);
		startOffset = gap;
		break;
	}

	// 放置子项：主轴按 mainSizes[] 和 gap 排列；交叉轴按每个子项的 Alignment 处理
	int cur = (isH ? content.left() : content.top()) + static_cast<int>(std::round(startOffset));
	for (size_t k = 0; k < vis.size(); ++k) {
		const auto idx = vis[k];
		const QRect childBounds = m_children[idx].component->bounds(); // 仅用于交叉轴“首选尺寸”
		const int mainSize = mainSizes[k];

		QRect childRect;
		if (isH) {
			// 交叉轴（垂直）的高度 + 对齐
			int height = availableCross;
			int y = content.top();
			if (m_children[idx].alignment != Alignment::Stretch) {
				const int prefH = childBounds.height();
				if (prefH > 0) height = std::min(height, prefH);
				switch (m_children[idx].alignment) {
				case Alignment::Center: y = content.top() + (availableCross - height) / 2; break;
				case Alignment::End:    y = content.bottom() - height; break;
				default: break;
				}
			}
			childRect = QRect(cur, y, std::min(mainSize, availableMain), height);
			cur += mainSize + static_cast<int>(std::round(gap));
		}
		else {
			// 交叉轴（水平）的宽度 + 对齐
			int width = availableCross;
			int x = content.left();
			if (m_children[idx].alignment != Alignment::Stretch) {
				const int prefW = childBounds.width();
				if (prefW > 0) width = std::min(width, prefW);
				switch (m_children[idx].alignment) {
				case Alignment::Center: x = content.left() + (availableCross - width) / 2; break;
				case Alignment::End:    x = content.right() - width; break;
				default: break;
				}
			}
			childRect = QRect(x, cur, width, std::min(mainSize, availableMain));
			cur += mainSize + static_cast<int>(std::round(gap));
		}

		m_childRects[vis[k]] = childRect;
	}
}

void UiBoxLayout::setViewportRect(const QRect& r)
{
	m_viewport = r;
	calculateLayout();  // 重新计算布局

	// 关键：在这里就把子项的 viewport 下发，不用等到 updateLayout
	if (!m_childRects.empty()) {
		for (size_t i = 0; i < m_children.size(); ++i) {
			if (!m_children[i].visible || !m_children[i].component) continue;
			if (auto* content = dynamic_cast<IUiContent*>(m_children[i].component)) {
				const QRect cr = (i < m_childRects.size() ? m_childRects[i] : QRect());
				content->setViewportRect(cr);
			}
		}
	}
}

void UiBoxLayout::updateLayout(const QSize& windowSize)
{
	qDebug() << "UiBoxLayout::updateLayout, viewport:" << m_viewport
		<< "children:" << m_children.size();

	calculateLayout();

	// 更新所有子控件的布局
	for (size_t i = 0; i < m_children.size(); ++i) {
		if (m_children[i].visible && m_children[i].component) {
			qDebug() << "UiBoxLayout updating child" << i
				<< "rect:" << m_childRects[i];

			// 如果子控件实现了 IUiContent，设置其视口
			if (auto* content = dynamic_cast<IUiContent*>(m_children[i].component)) {
				content->setViewportRect(m_childRects[i]);
			}
			m_children[i].component->updateLayout(windowSize);
		}
	}
}

void UiBoxLayout::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio)
{
	// 传递给所有子控件
	for (const auto& child : m_children) {
		if (child.component) {
			child.component->updateResourceContext(loader, gl, devicePixelRatio);
		}
	}
}

void UiBoxLayout::append(Render::FrameData& fd) const
{
	// 检查视口有效性
	if (!m_viewport.isValid()) {
		qDebug() << "UiBoxLayout::append - invalid viewport! Size:"
			<< m_viewport.size() << "Pos:" << m_viewport.topLeft();
		return;
	}

	// 背景（如果设置了）
	if (m_bgColor.alpha() > 0) {
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = QRectF(m_viewport),
			.radiusPx = m_cornerRadius,
			.color = m_bgColor
			});
	}

	// 子控件
	for (const auto& childItem : m_children)
	{
		if (childItem.visible && childItem.component) {
			childItem.component->append(fd);
		}
	}
}

bool UiBoxLayout::onMousePress(const QPoint& pos)
{
	if (!m_viewport.contains(pos)) return false;

	// 从后向前遍历（上层优先）
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
		if (it->visible && it->component) {
			if (it->component->onMousePress(pos)) {
				m_capturedChild = it->component;
				return true;
			}
		}
	}
	return false;
}

bool UiBoxLayout::onMouseMove(const QPoint& pos)
{
	if (m_capturedChild) {
		return m_capturedChild->onMouseMove(pos);
	}

	bool handled = false;
	for (const auto& child : m_children) {
		if (child.visible && child.component) {
			handled = child.component->onMouseMove(pos) || handled;
		}
	}
	return handled;
}

bool UiBoxLayout::onMouseRelease(const QPoint& pos)
{
	if (m_capturedChild) {
		const bool result = m_capturedChild->onMouseRelease(pos);
		m_capturedChild = nullptr;
		return result;
	}

	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
		if (it->visible && it->component) {
			if (it->component->onMouseRelease(pos)) {
				return true;
			}
		}
	}
	return false;
}

bool UiBoxLayout::tick()
{
	bool active = false;
	for (const auto& child : m_children) {
		if (child.component) {
			active = child.component->tick() || active;
		}
	}
	return active;
}

void UiBoxLayout::setChildWeight(size_t index, float weight)
{
	if (index < m_children.size()) {
		m_children[index].weight = std::max(0.0f, weight);
		calculateLayout();
	}
}

float UiBoxLayout::childWeight(size_t index) const
{
	if (index < m_children.size()) {
		return m_children[index].weight;
	}
	return 0.0f;
}

void UiBoxLayout::setChildAlignment(size_t index, Alignment align)
{
	if (index < m_children.size()) {
		m_children[index].alignment = align;
		calculateLayout();
	}
}
void UiBoxLayout::onThemeChanged(bool isDark)
{
	m_isDark = isDark;

	// 传播主题变化到所有子控件
	for (const auto& child : m_children) {
		if (child.component) {
			child.component->onThemeChanged(isDark);
		}
	}
}

UiBoxLayout::Alignment UiBoxLayout::childAlignment(size_t index) const
{
	if (index < m_children.size()) {
		return m_children[index].alignment;
	}
	return Alignment::Start;
}

// 添加便捷的构建器模式
UiBoxLayout& UiBoxLayout::withSpacing(int spacing)
{
	setSpacing(spacing);
	return *this;
}

UiBoxLayout& UiBoxLayout::withMargins(const QMargins& margins)
{
	setMargins(margins);
	return *this;
}

UiBoxLayout& UiBoxLayout::withBackground(const QColor& color, float radius)
{
	setBackgroundColor(color);
	setCornerRadius(radius);
	return *this;
}