#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiRoot.h"

#include <qopenglfunctions.h>
#include <qrect.h>
#include <ranges>
#include <vector>

#include <RenderUtils.hpp>

void UiRoot::add(IUiComponent* c)
{
	if (!c) return;
	if (std::ranges::find(m_children, c) == m_children.end())
		m_children.push_back(c);
}

void UiRoot::remove(IUiComponent* c)
{
	std::erase(m_children, c);
	if (m_pointerCapture == c) m_pointerCapture = nullptr;
}

void UiRoot::clear()
{
	m_children.clear();
	m_pointerCapture = nullptr;
}

void UiRoot::updateLayout(const QSize& windowSize) const
{
	for (auto* c : m_children) c->updateLayout(windowSize);
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