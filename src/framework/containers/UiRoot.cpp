#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiRoot.h"

#include <algorithm>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <ranges>
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
	// 根级可选：对子项命令叠加“子项自身边界”的裁剪，避免顶层越界
	for (const auto* c : m_children) {
		if (!c) continue;

		const int rr0 = static_cast<int>(fd.roundedRects.size());
		const int im0 = static_cast<int>(fd.images.size());

		c->append(fd);

		const QRectF clip = QRectF(c->bounds());
		if (clip.width() <= 0.0 || clip.height() <= 0.0) continue;

		for (int i = rr0; i < static_cast<int>(fd.roundedRects.size()); ++i) {
			auto& cmd = fd.roundedRects[i];
			if (cmd.clipRect.width() > 0.0 && cmd.clipRect.height() > 0.0) {
				cmd.clipRect = cmd.clipRect.intersected(clip);
			}
			else {
				cmd.clipRect = clip;
			}
		}
		for (int i = im0; i < static_cast<int>(fd.images.size()); ++i) {
			auto& cmd = fd.images[i];
			if (cmd.clipRect.width() > 0.0 && cmd.clipRect.height() > 0.0) {
				cmd.clipRect = cmd.clipRect.intersected(clip);
			}
			else {
				cmd.clipRect = clip;
			}
		}
	}
}

bool UiRoot::onMousePress(const QPoint& pos)
{
	// 从顶层到底层分发（后添加者优先）
	for (const auto& it : std::ranges::reverse_view(m_children))
	{
		if (it->onMousePress(pos)) {
			m_pointerCapture = it; // 捕获
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

void UiRoot::propagateThemeChange(bool isDark) const
{
	for (auto* c : m_children) {
		if (c) {
			c->onThemeChanged(isDark);
		}
	}
}