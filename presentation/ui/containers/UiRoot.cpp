#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "ILayoutable.hpp"
#include "IFocusContainer.hpp"
#include "UiRoot.h"

#include <qopenglfunctions.h>
#include <qrect.h>
#include <ranges>
#include <vector>

#include <RenderUtils.hpp>

void UiRoot::add(IUiComponent* c)
{
	if (!c) return;
	if (std::ranges::find(m_children, c) == m_children.end()) {
		m_children.push_back(c);
		m_focusOrderDirty = true; // 标记焦点顺序需要重建
	}
}

void UiRoot::remove(IUiComponent* c)
{
	std::erase(m_children, c);
	if (m_pointerCapture == c) m_pointerCapture = nullptr;
	if (m_focusedComponent == c) m_focusedComponent = nullptr;
	m_focusOrderDirty = true; // 标记焦点顺序需要重建
}

void UiRoot::clear()
{
	m_children.clear();
	m_pointerCapture = nullptr;
	m_focusedComponent = nullptr;
	m_focusOrderDirty = true; // 标记焦点顺序需要重建
}

void UiRoot::updateLayout(const QSize& windowSize) const
{
	// Reordered to fix content overflow: set viewport and arrange first, then updateLayout
	const QRect fullWindowRect(0, 0, windowSize.width(), windowSize.height());
	
	for (auto* c : m_children) {
		// 1) Set viewport for IUiContent children first
		if (auto* content = dynamic_cast<IUiContent*>(c)) {
			content->setViewportRect(fullWindowRect);
		}
		// 2) Call arrange for ILayoutable children
		if (auto* layoutable = dynamic_cast<ILayoutable*>(c)) {
			layoutable->arrange(fullWindowRect);
		}
		// 3) Then update layout with valid viewport in place
		c->updateLayout(windowSize);
	}
}

void UiRoot::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float devicePixelRatio) const
{
	for (auto* c : m_children) c->updateResourceContext(cache, gl, devicePixelRatio);
}

void UiRoot::append(Render::FrameData& fd) const
{
	for (const auto* c : m_children) {
		if (!c) continue;

		const int rr0 = static_cast<int>(fd.roundedRects.size());
		const int im0 = static_cast<int>(fd.images.size());

		c->append(fd);

		const auto clip = QRectF(c->bounds());
		RenderUtils::applyParentClip(fd, rr0, im0, clip);
	}
}

bool UiRoot::onMousePress(const QPoint& pos)
{
	for (const auto& it : std::ranges::reverse_view(m_children))
	{
		if (it->onMousePress(pos)) {
			m_pointerCapture = it; // 捕获
			
			// 如果点击的组件可以获得焦点，则自动设置焦点
			if (auto* focusable = dynamic_cast<IFocusable*>(it)) {
				if (focusable->canFocus()) {
					setFocus(it);
				}
			}
			
			return true;
		}
	}
	m_pointerCapture = nullptr;
	return false;
}

bool UiRoot::onMouseMove(const QPoint& pos)
{
	if (m_pointerCapture) {
		// 捕获期间仅路由给捕获者
		return m_pointerCapture->onMouseMove(pos);
	}
	bool any = false;
	for (const auto& it : std::ranges::reverse_view(m_children))
	{
		any = it->onMouseMove(pos) || any;
	}
	return any;
}

bool UiRoot::onMouseRelease(const QPoint& pos)
{
	if (m_pointerCapture) {
		// 将释放事件交还给捕获者，并解除捕获
		const bool handled = m_pointerCapture->onMouseRelease(pos);
		m_pointerCapture = nullptr;
		return handled;
	}
	for (const auto& it : std::ranges::reverse_view(m_children))
	{
		if (it->onMouseRelease(pos)) return true;
	}
	return false;
}

bool UiRoot::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
	// 按 Z 序（倒序）将滚轮事件分发给命中的子组件
	for (const auto& it : std::ranges::reverse_view(m_children))
	{
		if (it->onWheel(pos, angleDelta)) return true;
	}
	return false;
}

bool UiRoot::tick() const
{
	bool any = false;
	for (auto* c : m_children) any = c->tick() || any;
	return any;
}

void UiRoot::propagateThemeChange(const bool isDark) const
{
	for (auto* c : m_children) {
		if (c) {
			c->onThemeChanged(isDark);
		}
	}
}

bool UiRoot::onKeyPress(int key, Qt::KeyboardModifiers modifiers)
{
	// 处理Tab键导航
	if (key == Qt::Key_Tab) {
		if (modifiers & Qt::ShiftModifier) {
			focusPrevious();
		} else {
			focusNext();
		}
		return true;
	}
	
	// 只有有焦点的组件才能响应键盘输入
	if (m_focusedComponent) {
		// 尝试转换为IKeyInput接口
		if (auto* keyInput = dynamic_cast<IKeyInput*>(m_focusedComponent)) {
			return keyInput->onKeyPress(key, modifiers);
		}
	}
	return false;
}

bool UiRoot::onKeyRelease(int key, Qt::KeyboardModifiers modifiers)
{
	// 只有有焦点的组件才能响应键盘输入
	if (m_focusedComponent) {
		// 尝试转换为IKeyInput接口
		if (auto* keyInput = dynamic_cast<IKeyInput*>(m_focusedComponent)) {
			return keyInput->onKeyRelease(key, modifiers);
		}
	}
	return false;
}

void UiRoot::setFocus(IUiComponent* component)
{
	// 清除当前焦点
	if (m_focusedComponent) {
		if (auto* focusable = dynamic_cast<IFocusable*>(m_focusedComponent)) {
			focusable->setFocused(false);
		}
	}
	
	// 设置新焦点
	m_focusedComponent = component;
	if (m_focusedComponent) {
		if (auto* focusable = dynamic_cast<IFocusable*>(m_focusedComponent)) {
			if (focusable->canFocus()) {
				focusable->setFocused(true);
			} else {
				m_focusedComponent = nullptr; // 不能获得焦点则清空
			}
		}
	}
}

void UiRoot::clearFocus()
{
	if (m_focusedComponent) {
		if (auto* focusable = dynamic_cast<IFocusable*>(m_focusedComponent)) {
			focusable->setFocused(false);
		}
		m_focusedComponent = nullptr;
	}
}

void UiRoot::focusNext()
{
	rebuildFocusOrder();
	
	if (m_focusOrder.empty()) {
		clearFocus();
		return;
	}
	
	// 如果当前没有焦点，设置焦点到第一个组件
	if (!m_focusedComponent) {
		if (auto* comp = dynamic_cast<IUiComponent*>(m_focusOrder[0])) {
			setFocus(comp);
		}
		return;
	}
	
	// 查找当前焦点组件在列表中的位置
	const int currentIndex = findFocusIndex(m_focusedComponent);
	if (currentIndex >= 0) {
		// 移动到下一个组件（循环到开头）
		const int nextIndex = (currentIndex + 1) % static_cast<int>(m_focusOrder.size());
		if (auto* comp = dynamic_cast<IUiComponent*>(m_focusOrder[nextIndex])) {
			setFocus(comp);
		}
	} else {
		// 当前组件不在列表中，设置焦点到第一个组件
		if (auto* comp = dynamic_cast<IUiComponent*>(m_focusOrder[0])) {
			setFocus(comp);
		}
	}
}

void UiRoot::focusPrevious()
{
	rebuildFocusOrder();
	
	if (m_focusOrder.empty()) {
		clearFocus();
		return;
	}
	
	// 如果当前没有焦点，设置焦点到最后一个组件
	if (!m_focusedComponent) {
		if (auto* comp = dynamic_cast<IUiComponent*>(m_focusOrder.back())) {
			setFocus(comp);
		}
		return;
	}
	
	// 查找当前焦点组件在列表中的位置
	const int currentIndex = findFocusIndex(m_focusedComponent);
	if (currentIndex >= 0) {
		// 移动到上一个组件（循环到末尾）
		const int prevIndex = (currentIndex - 1 + static_cast<int>(m_focusOrder.size())) % static_cast<int>(m_focusOrder.size());
		if (auto* comp = dynamic_cast<IUiComponent*>(m_focusOrder[prevIndex])) {
			setFocus(comp);
		}
	} else {
		// 当前组件不在列表中，设置焦点到最后一个组件
		if (auto* comp = dynamic_cast<IUiComponent*>(m_focusOrder.back())) {
			setFocus(comp);
		}
	}
}

void UiRoot::rebuildFocusOrder()
{
	if (!m_focusOrderDirty) {
		return;
	}
	
	m_focusOrder.clear();
	
	// 遍历所有子组件，收集可焦点组件
	for (auto* child : m_children) {
		if (!child) continue;
		
		// 如果子组件本身可以获得焦点，添加它
		if (auto* focusable = dynamic_cast<IFocusable*>(child)) {
			if (focusable->canFocus()) {
				m_focusOrder.push_back(focusable);
			}
		}
		
		// 如果子组件是容器，递归枚举其可焦点子组件
		if (auto* container = dynamic_cast<IFocusContainer*>(child)) {
			container->enumerateFocusables(m_focusOrder);
		}
	}
	
	m_focusOrderDirty = false;
}

int UiRoot::findFocusIndex(IUiComponent* component) const
{
	if (!component) return -1;
	
	auto* focusable = dynamic_cast<IFocusable*>(component);
	if (!focusable) return -1;
	
	for (int i = 0; i < static_cast<int>(m_focusOrder.size()); ++i) {
		if (m_focusOrder[i] == focusable) {
			return i;
		}
	}
	
	return -1;
}