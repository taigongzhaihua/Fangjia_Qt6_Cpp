#include "UiBoxLayout.h"
#include <algorithm>
#include <IconLoader.h>
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

	// 省略日志...

	if (!content.isValid() || m_children.empty()) {
		return;
	}

	// 统计可见子控件
	std::vector<size_t> visibleIndices;
	float totalWeight = 0.0f;

	for (size_t i = 0; i < m_children.size(); ++i) {
		if (m_children[i].visible) {
			visibleIndices.push_back(i);
			totalWeight += m_children[i].weight;
		}
	}

	if (visibleIndices.empty()) return;

	const bool isHorizontal = (m_direction == Direction::Horizontal);
	const int totalSpacing = m_spacing * std::max(0, static_cast<int>(visibleIndices.size()) - 1);

	// 可用空间
	int availableSize = isHorizontal ? content.width() : content.height();
	availableSize -= totalSpacing;

	// 第一遍：固定大小
	int usedSize = 0;
	for (const size_t idx : visibleIndices) {
		if (m_children[idx].weight == 0.0f) {
			const QRect childBounds = m_children[idx].component->bounds();
			const int preferredSize = isHorizontal ? childBounds.width() : childBounds.height();
			usedSize += preferredSize;
		}
	}

	// 第二遍：分配弹性空间
	const int flexibleSpace = std::max(0, availableSize - usedSize);

	int currentPos = isHorizontal ? content.left() : content.top();

	for (const size_t idx : visibleIndices) {
		const auto& child = m_children[idx];
		const QRect childBounds = child.component ? child.component->bounds() : QRect();
		QRect childRect;

		if (isHorizontal) {
			int width = (child.weight > 0.0f && totalWeight > 0.0f)
				? static_cast<int>(flexibleSpace * (child.weight / totalWeight))
				: childBounds.width();
			width = std::min(width, content.width());

			int height = content.height();
			int y = content.top();

			// 仅在子组件首选高度有效时才收缩
			if (child.alignment != Alignment::Stretch) {
				const int prefH = childBounds.height();
				if (prefH > 0) {
					height = std::min(height, prefH);
				}
				switch (child.alignment) {
				case Alignment::Center:
					y = content.top() + (content.height() - height) / 2;
					break;
				case Alignment::End:
					y = content.bottom() - height;
					break;
				default:
					break;
				}
			}

			childRect = QRect(currentPos, y, width, height);
			currentPos += width + m_spacing;
		}
		else {
			int height = (child.weight > 0.0f && totalWeight > 0.0f)
				? static_cast<int>(flexibleSpace * (child.weight / totalWeight))
				: childBounds.height();
			height = std::min(height, content.height());

			int width = content.width();
			int x = content.left();

			// 仅在子组件首选宽度有效时才收缩
			if (child.alignment != Alignment::Stretch) {
				const int prefW = childBounds.width();
				if (prefW > 0) {
					width = std::min(width, prefW);
				}
				switch (child.alignment) {
				case Alignment::Center:
					x = content.left() + (content.width() - width) / 2;
					break;
				case Alignment::End:
					x = content.right() - width;
					break;
				default:
					break;
				}
			}

			childRect = QRect(x, currentPos, width, height);
			currentPos += height + m_spacing;
		}

		m_childRects[idx] = childRect;
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

	// 绘制背景（如果设置了）
	if (m_bgColor.alpha() > 0) {
		fd.roundedRects.push_back(Render::RoundedRectCmd{
			.rect = QRectF(m_viewport),
			.radiusPx = m_cornerRadius,
			.color = m_bgColor
			});
	}

	// 绘制所有可见子控件
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

	// 可以在这里根据主题调整自己的颜色
	// 例如：updatePalette(isDark);
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
