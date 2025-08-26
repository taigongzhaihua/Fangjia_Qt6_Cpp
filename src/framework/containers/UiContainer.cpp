#include "UiContainer.h"
#include <algorithm>
#include <limits>

QSize UiContainer::measure(const SizeConstraints& cs)
{
    if (!m_child) {
        return QSize(clampDim(0, cs.minW, cs.maxW), clampDim(0, cs.minH, cs.maxH));
    }

    QSize inner(0, 0);
    if (auto* l = dynamic_cast<ILayoutable*>(m_child)) {
        inner = l->measure(cs);
    } else {
        inner = m_child->bounds().size();
        inner.setWidth(std::clamp(inner.width(), cs.minW, cs.maxW));
        inner.setHeight(std::clamp(inner.height(), cs.minH, cs.maxH));
    }
    // 容器自身不添加额外尺寸（padding/background 由 DecoratedBox 承担）
    return inner;
}

void UiContainer::arrange(const QRect& finalRect)
{
    setViewportRect(finalRect);
    if (!m_child || !finalRect.isValid()) return;

    // 在当前可用区域下，获取子项“期望尺寸”，便于非 Stretch 对齐时放置自然大小
    QSize desired(0, 0);
    if (auto* l = dynamic_cast<ILayoutable*>(m_child)) {
        SizeConstraints cs{};
        cs.minW = 0; cs.minH = 0;
        cs.maxW = std::max(0, finalRect.width());
        cs.maxH = std::max(0, finalRect.height());
        desired = l->measure(cs);
    } else {
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

QRect UiContainer::placeChildRect(const QRect& area, const QSize& desired) const
{
    const int availW = std::max(0, area.width());
    const int availH = std::max(0, area.height());

    int w = desired.width();
    int h = desired.height();

    // Stretch 维度直接占满
    if (m_hAlign == Align::Stretch) w = availW; else w = std::min(w, availW);
    if (m_vAlign == Align::Stretch) h = availH; else h = std::min(h, availH);

    // 水平放置
    int x = area.left();
    switch (m_hAlign) {
    case Align::Start: x = area.left(); break;
    case Align::Center: x = area.left() + (availW - w) / 2; break;
    case Align::End: x = area.right() - w; break;
    case Align::Stretch: x = area.left(); break;
    }
    // 垂直放置
    int y = area.top();
    switch (m_vAlign) {
    case Align::Start: y = area.top(); break;
    case Align::Center: y = area.top() + (availH - h) / 2; break;
    case Align::End: y = area.bottom() - h; break;
    case Align::Stretch: y = area.top(); break;
    }

    return QRect(x, y, std::max(0, w), std::max(0, h));
}

void UiContainer::setViewportRect(const QRect& r)
{
    m_viewport = r;
    // 注意：不立即安排子项，arrange() 会完成
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
    // 自身不绘制任何内容（背景交给 DecoratedBox）
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