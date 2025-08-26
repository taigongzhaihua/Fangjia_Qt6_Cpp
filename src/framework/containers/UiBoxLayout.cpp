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

UiBoxLayout::UiBoxLayout(Direction dir) : m_direction(dir) {}

void UiBoxLayout::addChild(IUiComponent* component, float weight, Alignment align)
{
	if (!component) return;
	const auto it = std::ranges::find_if(m_children,
		[component](const ChildItem& item) { return item.component == component; });
	if (it == m_children.end()) {
		m_children.push_back({ component, weight, align, true });
	}
}

void UiBoxLayout::insertChild(size_t index, IUiComponent* component, float weight, Alignment align)
{
	if (!component) return;
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
	if (it != m_children.end()) m_children.erase(it, m_children.end());
}

void UiBoxLayout::removeChildAt(size_t index)
{
	if (index < m_children.size()) m_children.erase(m_children.begin() + index);
}

void UiBoxLayout::clearChildren()
{
	m_children.clear();
	m_childRects.clear();
}

IUiComponent* UiBoxLayout::childAt(size_t index) const
{
	if (index < m_children.size()) return m_children[index].component;
	return nullptr;
}

void UiBoxLayout::setDirection(Direction dir)
{
	if (m_direction != dir) { m_direction = dir; calculateLayout(); }
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
	if (index < m_children.size()) return m_children[index].visible;
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

	const int baseGapCount = std::max<int>(0, static_cast<int>(vis.size()) - 1);
	const bool fixedGapMode =
		(m_mainAlign == MainAlignment::Start ||
			m_mainAlign == MainAlignment::Center ||
			m_mainAlign == MainAlignment::End);
	const int reservedGaps = fixedGapMode ? (m_spacing * baseGapCount) : 0;

	std::vector<int> mainSizes(vis.size(), 0);
	std::vector<int> prefMains(vis.size(), 0);

	// 读取首选尺寸
	for (size_t k = 0; k < vis.size(); ++k) {
		const auto idx = vis[k];
		const QRect pref = m_children[idx].component->bounds();
		prefMains[k] = isH ? pref.width() : pref.height();
	}

	if (m_sizeMode == SizeMode::Natural) {
		// 按首选尺寸顺序排布，不扩展
		for (size_t k = 0; k < vis.size(); ++k) {
			const int s = std::max(0, prefMains[k]); // 可为 0
			mainSizes[k] = s;
		}
	}
	else {
		// Weighted：原行为
		int usedFixed = 0;
		for (size_t k = 0; k < vis.size(); ++k) {
			const auto idx = vis[k];
			if (m_children[idx].weight <= 0.0f) {
				const int s = std::max(0, std::min(prefMains[k], availableMain));
				mainSizes[k] = s;
				usedFixed += s;
			}
		}

		int flexibleSpace = std::max(0, availableMain - usedFixed - reservedGaps);
		float acc = 0.0f, assigned = 0.0f;
		if (totalWeight > 0.0f && flexibleSpace > 0) {
			for (size_t k = 0; k < vis.size(); ++k) {
				const auto idx = vis[k];
				const float w = std::max(0.0f, m_children[idx].weight);
				if (w > 0.0f) {
					acc += w / totalWeight * static_cast<float>(flexibleSpace);
					int s = static_cast<int>(std::floor(acc - assigned));
					s = std::max(0, s);
					mainSizes[k] = s;
					assigned += static_cast<float>(s);
				}
			}
		}
	}

	// 计算用于对齐的总主轴长度（仅子项+固定间距）
	const int sumMain = std::accumulate(mainSizes.begin(), mainSizes.end(), 0);
	int remaining = std::max(0, availableMain - sumMain - reservedGaps);

	// Natural 模式下，SpaceBetween/Around/Evenly 不再“动态分配间距”，统一按固定 spacing
	double gap = static_cast<double>(m_spacing);
	double startOffset = 0.0;
	const int n = static_cast<int>(vis.size());
	switch (m_mainAlign) {
	case MainAlignment::Start: startOffset = 0.0; break;
	case MainAlignment::Center: startOffset = remaining * 0.5; break;
	case MainAlignment::End: startOffset = remaining; break;
	case MainAlignment::SpaceBetween:
		if (m_sizeMode == SizeMode::Weighted) {
			if (n > 1) { gap = static_cast<double>(remaining) / static_cast<double>(n - 1); startOffset = 0.0; }
			else { gap = 0.0; startOffset = remaining * 0.5; }
		}
		break;
	case MainAlignment::SpaceAround:
		if (m_sizeMode == SizeMode::Weighted) {
			if (n > 0) { gap = static_cast<double>(remaining) / static_cast<double>(n); startOffset = gap * 0.5; }
		}
		break;
	case MainAlignment::SpaceEvenly:
		if (m_sizeMode == SizeMode::Weighted) {
			gap = static_cast<double>(remaining) / static_cast<double>(n + 1);
			startOffset = gap;
		}
		break;
	}

	int cur = (isH ? content.left() : content.top()) + static_cast<int>(std::round(startOffset));
	for (size_t k = 0; k < vis.size(); ++k) {
		const auto idx = vis[k];
		const QRect childBounds = m_children[idx].component->bounds(); // 用于交叉轴
		const int mainSize = mainSizes[k];

		QRect childRect;
		if (isH) {
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
			childRect = QRect(cur, y, std::max(0, mainSize), height);
			cur += mainSize + static_cast<int>(std::round(gap));
		}
		else {
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
			childRect = QRect(x, cur, width, std::max(0, mainSize));
			cur += mainSize + static_cast<int>(std::round(gap));
		}

		m_childRects[vis[k]] = childRect;
	}
}

void UiBoxLayout::setViewportRect(const QRect& r)
{
	m_viewport = r;
	calculateLayout();

	// 将子项 viewport 裁切到内容区域，避免溢出绘制
	const QRect content = contentRect();
	if (!m_childRects.empty()) {
		for (size_t i = 0; i < m_children.size(); ++i) {
			if (!m_children[i].visible || !m_children[i].component) continue;
			if (auto* c = dynamic_cast<IUiContent*>(m_children[i].component)) {
				const QRect cr = (i < m_childRects.size() ? m_childRects[i] : QRect());
				const QRect clipped = cr.intersected(content);
				c->setViewportRect(clipped);
			}
		}
	}
}

void UiBoxLayout::updateLayout(const QSize& windowSize)
{
	qDebug() << "UiBoxLayout::updateLayout, viewport:" << m_viewport
		<< "children:" << m_children.size();

	calculateLayout();

	for (size_t i = 0; i < m_children.size(); ++i) {
		if (m_children[i].visible && m_children[i].component) {
			const QRect content = contentRect();
			const QRect cr = (i < m_childRects.size() ? m_childRects[i] : QRect());
			if (auto* contentIf = dynamic_cast<IUiContent*>(m_children[i].component)) {
				contentIf->setViewportRect(cr.intersected(content));
			}
			m_children[i].component->updateLayout(windowSize);
		}
	}
}

void UiBoxLayout::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio)
{
	for (const auto& child : m_children) {
		if (child.component) {
			child.component->updateResourceContext(loader, gl, devicePixelRatio);
		}
	}
}

void UiBoxLayout::append(Render::FrameData& fd) const
{
	if (!m_viewport.isValid()) {
		qDebug() << "UiBoxLayout::append - invalid viewport! Size:" << m_viewport.size()
			<< "Pos:" << m_viewport.topLeft();
		return;
	}

	if (m_bgColor.alpha() > 0) {
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = QRectF(m_viewport),
			.radiusPx = m_cornerRadius,
			.color = m_bgColor,
			.clipRect = QRectF(m_viewport)
			});
	}

	for (const auto& childItem : m_children) {
		if (childItem.visible && childItem.component) {
			childItem.component->append(fd);
		}
	}
}

bool UiBoxLayout::onMousePress(const QPoint& pos)
{
	if (!m_viewport.contains(pos)) return false;
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
	if (m_capturedChild) return m_capturedChild->onMouseMove(pos);

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
			if (it->component->onMouseRelease(pos)) return true;
		}
	}
	return false;
}

bool UiBoxLayout::tick()
{
	bool active = false;
	for (const auto& child : m_children) {
		if (child.component) active = child.component->tick() || active;
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
	if (index < m_children.size()) return m_children[index].weight;
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
	for (const auto& child : m_children) {
		if (child.component) child.component->onThemeChanged(isDark);
	}
}

UiBoxLayout& UiBoxLayout::withSpacing(int spacing) { setSpacing(spacing); return *this; }
UiBoxLayout& UiBoxLayout::withMargins(const QMargins& margins) { setMargins(margins); return *this; }
UiBoxLayout& UiBoxLayout::withBackground(const QColor& color, float radius) { setBackgroundColor(color); setCornerRadius(radius); return *this; }