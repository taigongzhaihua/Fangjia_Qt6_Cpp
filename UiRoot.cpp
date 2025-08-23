#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiRoot.h"

#include <qopenglfunctions.h>
#include <qrect.h>
#include <vector>

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

void UiRoot::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, const float devicePixelRatio) const
{
	for (auto* c : m_children) c->updateResourceContext(loader, gl, devicePixelRatio);
}

void UiRoot::append(Render::FrameData& fd) const
{
	for (auto* c : m_children) c->append(fd);
}

bool UiRoot::onMousePress(const QPoint& pos)
{
	// 从顶层到底层分发（后添加者优先）
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
		if ((*it)->onMousePress(pos)) {
			m_pointerCapture = *it; // 捕获
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
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
		any = (*it)->onMouseMove(pos) || any;
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
	for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
		if ((*it)->onMouseRelease(pos)) return true;
	}
	return false;
}

bool UiRoot::tick() const
{
	bool any = false;
	for (auto* c : m_children) any = c->tick() || any;
	return any;
}

QRect UiRoot::boundsUnion() const
{
	if (m_children.empty()) return {};
	QRect u = m_children.front()->bounds();
	for (size_t i = 1; i < m_children.size(); ++i) {
		u = u.united(m_children[i]->bounds());
	}
	return u;
}