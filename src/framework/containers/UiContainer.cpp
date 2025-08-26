#include "UiContainer.h"
#include <algorithm>
#include <ILayoutable.hpp>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>
#include <UiComponent.hpp>
#include <UiContent.hpp>

QSize UiContainer::measure(const SizeConstraints& cs)
{
	if (!m_child) {
		return QSize(std::clamp(0, cs.minW, cs.maxW),
			std::clamp(0, cs.minH, cs.maxH));
	}

	QSize inner(0, 0);
	if (auto* l = dynamic_cast<ILayoutable*>(m_child)) {
		inner = l->measure(cs);
	}
	else {
		inner = m_child->bounds().size();
		inner.setWidth(std::clamp(inner.width(), cs.minW, cs.maxW));
		inner.setHeight(std::clamp(inner.height(), cs.minH, cs.maxH));
	}
	return inner;
}

void UiContainer::doArrange(const QRect& finalRect)
{
	if (!m_child || !finalRect.isValid()) return;

	// 在当前区域内拿到子项期望尺寸（便于非 Stretch 对齐）
	QSize desired(0, 0);
	if (auto* l = dynamic_cast<ILayoutable*>(m_child)) {
		SizeConstraints cs{};
		cs.minW = 0; cs.minH = 0;
		cs.maxW = std::max(0, finalRect.width());
		cs.maxH = std::max(0, finalRect.height());
		desired = l->measure(cs);
	}
	else {
		desired = m_child->bounds().size();
	}

	const QRect childRect = placeChildRect(finalRect, desired);

	if (auto* c = dynamic_cast<IUiContent*>(m_child)) {
		c->setViewportRect(childRect);
	}
	if (auto* l = dynamic_cast<ILayoutable*>(m_child)) {
		l->arrange(childRect);
	}
}

void UiContainer::arrange(const QRect& finalRect)
{
	m_viewport = finalRect;
	doArrange(finalRect);
}

QRect UiContainer::placeChildRect(const QRect& area, const QSize& desired) const
{
	const int availW = std::max(0, area.width());
	const int availH = std::max(0, area.height());

	int w = std::max(0, desired.width());
	int h = std::max(0, desired.height());

	if (m_hAlign == Align::Stretch) w = availW; else w = std::min(w, availW);
	if (m_vAlign == Align::Stretch) h = availH; else h = std::min(h, availH);

	int x = area.left();
	switch (m_hAlign) {
	case Align::Start:   x = area.left(); break;
	case Align::Center:  x = area.left() + (availW - w) / 2; break;
	case Align::End:     x = area.right() - w; break;
	case Align::Stretch: x = area.left(); break;
	}

	int y = area.top();
	switch (m_vAlign) {
	case Align::Start:   y = area.top(); break;
	case Align::Center:  y = area.top() + (availH - h) / 2; break;
	case Align::End:     y = area.bottom() - h; break;
	case Align::Stretch: y = area.top(); break;
	}

	return QRect(x, y, std::max(0, w), std::max(0, h));
}

void UiContainer::setViewportRect(const QRect& r)
{
	m_viewport = r;
	// 关键：即便父级只是下发 viewport，也完成一次放置
	doArrange(r);
}

void UiContainer::updateLayout(const QSize& windowSize)
{
	if (m_child) m_child->updateLayout(windowSize);
}

void UiContainer::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio)
{
	m_loader = &loader; m_gl = gl; m_dpr = devicePixelRatio;
	if (m_child) m_child->updateResourceContext(loader, gl, devicePixelRatio);
}

void UiContainer::append(Render::FrameData& fd) const
{
	if (m_child) m_child->append(fd);
}

bool UiContainer::onMousePress(const QPoint& pos)
{
	return m_child ? m_child->onMousePress(pos) : false;
}
bool UiContainer::onMouseMove(const QPoint& pos)
{
	return m_child ? m_child->onMouseMove(pos) : false;
}
bool UiContainer::onMouseRelease(const QPoint& pos)
{
	return m_child ? m_child->onMouseRelease(pos) : false;
}

bool UiContainer::tick()
{
	return m_child ? m_child->tick() : false;
}

void UiContainer::onThemeChanged(bool isDark)
{
	if (m_child) m_child->onThemeChanged(isDark);
}