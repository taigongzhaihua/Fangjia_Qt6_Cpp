#include "UiScrollView.h"
#include <algorithm>
#include <ILayoutable.hpp>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <RenderUtils.hpp>

UiScrollView::UiScrollView() {
    applyTheme(false); // 初始化为浅色主题
}

void UiScrollView::setScrollY(int scrollY) {
    const int oldScrollY = m_scrollY;
    m_scrollY = scrollY;
    clampScrollY();
    
    if (m_scrollY != oldScrollY) {
        updateChildLayout();
    }
}

int UiScrollView::maxScrollY() const noexcept {
    const int viewportHeight = m_viewport.height();
    return std::max(0, m_contentHeight - viewportHeight);
}

void UiScrollView::setViewportRect(const QRect& r) {
    m_viewport = r;
    clampScrollY();
    updateChildLayout();
}

QSize UiScrollView::measure(const SizeConstraints& cs) {
    if (!m_child) {
        return QSize(std::clamp(0, cs.minW, cs.maxW),
                    std::clamp(0, cs.minH, cs.maxH));
    }

    // 为了得到内容高度，使用 widthBounded 方式测量子组件
    QSize childSize(0, 0);
    if (auto* layoutable = dynamic_cast<ILayoutable*>(m_child)) {
        // 给子组件提供宽度约束（减去滚动条宽度）
        SizeConstraints childCs = cs;
        childCs.maxW = std::max(0, cs.maxW - SCROLLBAR_WIDTH);
        childSize = layoutable->measure(childCs);
    } else {
        childSize = m_child->bounds().size();
    }

    // 缓存内容高度
    m_contentHeight = childSize.height();

    // 返回容器期望尺寸（宽度可能需要包含滚动条）
    int desiredWidth = childSize.width();
    if (m_contentHeight > cs.maxH) {
        desiredWidth += SCROLLBAR_WIDTH; // 需要滚动条时加上其宽度
    }

    return QSize(std::clamp(desiredWidth, cs.minW, cs.maxW),
                std::clamp(childSize.height(), cs.minH, cs.maxH));
}

void UiScrollView::arrange(const QRect& finalRect) {
    m_viewport = finalRect;
    clampScrollY();
    updateChildLayout();
}

void UiScrollView::updateLayout(const QSize& windowSize) {
    measureContent();
    updateChildLayout();
}

void UiScrollView::measureContent() {
    if (!m_child) {
        m_contentHeight = 0;
        return;
    }

    if (auto* layoutable = dynamic_cast<ILayoutable*>(m_child)) {
        // 给子组件提供宽度约束
        SizeConstraints cs = SizeConstraints::widthBounded(
            m_viewport.width() - (isScrollbarVisible() ? SCROLLBAR_WIDTH : 0)
        );
        const QSize childSize = layoutable->measure(cs);
        m_contentHeight = childSize.height();
    } else {
        m_contentHeight = m_child->bounds().height();
    }
}

void UiScrollView::updateChildLayout() {
    if (!m_child) return;

    const QRect childViewport = getChildViewport();

    // 设置子组件视口
    if (auto* content = dynamic_cast<IUiContent*>(m_child)) {
        content->setViewportRect(childViewport);
    }

    // 安排子组件布局
    if (auto* layoutable = dynamic_cast<ILayoutable*>(m_child)) {
        layoutable->arrange(childViewport);
    }

    // 传递布局更新
    if (m_viewport.isValid()) {
        m_child->updateLayout(m_viewport.size());
    }
}

QRect UiScrollView::getChildViewport() const {
    if (!m_viewport.isValid()) return QRect();

    const int contentWidth = m_viewport.width() - (isScrollbarVisible() ? SCROLLBAR_WIDTH : 0);
    return QRect(
        m_viewport.left(),
        m_viewport.top() - m_scrollY,  // 关键：通过调整顶部坐标实现滚动
        contentWidth,
        m_contentHeight
    );
}

QRect UiScrollView::getScrollbarRect() const {
    if (!isScrollbarVisible()) return QRect();
    
    return QRect(
        m_viewport.right() - SCROLLBAR_WIDTH + 1,
        m_viewport.top(),
        SCROLLBAR_WIDTH,
        m_viewport.height()
    );
}

QRect UiScrollView::getScrollbarThumbRect() const {
    const QRect scrollbarRect = getScrollbarRect();
    if (!scrollbarRect.isValid()) return QRect();

    const int viewportHeight = m_viewport.height();
    const int maxScroll = maxScrollY();
    
    if (maxScroll <= 0) return QRect(); // 不需要滚动

    // 计算thumb高度（按内容比例）
    const float ratio = static_cast<float>(viewportHeight) / static_cast<float>(m_contentHeight);
    const int thumbHeight = std::max(THUMB_MIN_HEIGHT, 
                                   static_cast<int>(scrollbarRect.height() * ratio));

    // 计算thumb位置
    const float scrollRatio = static_cast<float>(m_scrollY) / static_cast<float>(maxScroll);
    const int availableTrackHeight = scrollbarRect.height() - thumbHeight;
    const int thumbTop = scrollbarRect.top() + static_cast<int>(availableTrackHeight * scrollRatio);

    return QRect(scrollbarRect.left(), thumbTop, scrollbarRect.width(), thumbHeight);
}

bool UiScrollView::isScrollbarVisible() const {
    return m_contentHeight > m_viewport.height();
}

bool UiScrollView::isPointInScrollbar(const QPoint& pos) const {
    return getScrollbarRect().contains(pos);
}

bool UiScrollView::isPointInThumb(const QPoint& pos) const {
    return getScrollbarThumbRect().contains(pos);
}

void UiScrollView::clampScrollY() {
    const int maxScroll = maxScrollY();
    m_scrollY = std::clamp(m_scrollY, 0, maxScroll);
}

void UiScrollView::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) {
    m_cache = &cache;
    m_gl = gl;
    m_dpr = devicePixelRatio;

    // 传递给子组件
    if (m_child) {
        m_child->updateResourceContext(cache, gl, devicePixelRatio);
    }
}

void UiScrollView::append(Render::FrameData& fd) const {
    if (!m_viewport.isValid()) return;

    // 记录当前命令数量，用于应用裁剪
    const int rr0 = static_cast<int>(fd.roundedRects.size());
    const int im0 = static_cast<int>(fd.images.size());

    // 先添加子组件的渲染命令
    if (m_child) {
        m_child->append(fd);
    }

    // 将子组件的渲染命令裁剪到容器视口
    RenderUtils::applyParentClip(fd, rr0, im0, QRectF(m_viewport));

    // 渲染滚动条
    if (isScrollbarVisible()) {
        renderScrollbar(fd);
    }
}

void UiScrollView::renderScrollbar(Render::FrameData& fd) const {
    const QRect scrollbarRect = getScrollbarRect();
    const QRect thumbRect = getScrollbarThumbRect();
    
    if (!scrollbarRect.isValid()) return;

    // 渲染滚动条轨道
    fd.roundedRects.push_back(Render::RoundedRectCmd{
        .rect = QRectF(scrollbarRect),
        .radiusPx = 2.0f,
        .color = m_trackColor,
        .clipRect = QRectF(m_viewport)
    });

    // 渲染滚动条thumb
    if (thumbRect.isValid()) {
        QColor thumbColor = m_thumbColor;
        if (m_thumbPressed) {
            thumbColor = m_thumbPressColor;
        } else if (m_thumbHovered) {
            thumbColor = m_thumbHoverColor;
        }

        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(thumbRect),
            .radiusPx = 3.0f,
            .color = thumbColor,
            .clipRect = QRectF(m_viewport)
        });
    }
}

bool UiScrollView::onMousePress(const QPoint& pos) {
    if (!m_viewport.contains(pos)) return false;

    // 检查是否点击在滚动条区域
    if (isScrollbarVisible() && isPointInScrollbar(pos)) {
        if (isPointInThumb(pos)) {
            // 开始拖拽thumb
            startThumbDrag(pos);
        } else {
            // 点击轨道，快速定位
            handleTrackClick(pos);
        }
        return true;
    }

    // 否则开始内容拖拽
    startContentDrag(pos);
    
    // 继续传递给子组件
    if (m_child) {
        return m_child->onMousePress(pos);
    }
    
    return false;
}

void UiScrollView::startThumbDrag(const QPoint& pos) {
    m_dragMode = DragMode::Thumb;
    m_dragStartPos = pos;
    m_dragStartScrollY = m_scrollY;
    m_dragStartThumbY = getScrollbarThumbRect().top();
    m_thumbPressed = true;
}

void UiScrollView::handleTrackClick(const QPoint& pos) {
    const QRect scrollbarRect = getScrollbarRect();
    const QRect thumbRect = getScrollbarThumbRect();
    
    if (!scrollbarRect.isValid()) return;

    // 计算点击位置应该对应的滚动位置
    // 将点击位置置于thumb中心
    const int thumbHeight = thumbRect.height();
    const int targetThumbCenter = pos.y();
    const int targetThumbTop = targetThumbCenter - thumbHeight / 2;
    
    // 将thumb位置转换为滚动偏移
    const int availableTrackHeight = scrollbarRect.height() - thumbHeight;
    const int thumbOffset = targetThumbTop - scrollbarRect.top();
    const float scrollRatio = static_cast<float>(thumbOffset) / static_cast<float>(availableTrackHeight);
    
    const int targetScrollY = static_cast<int>(maxScrollY() * scrollRatio);
    setScrollY(targetScrollY);
}

void UiScrollView::startContentDrag(const QPoint& pos) {
    m_dragMode = DragMode::Content;
    m_dragStartPos = pos;
    m_dragStartScrollY = m_scrollY;
}

bool UiScrollView::onMouseMove(const QPoint& pos) {
    // 更新悬停状态
    const bool wasHovered = m_thumbHovered;
    m_thumbHovered = isScrollbarVisible() && isPointInThumb(pos);
    
    // 处理拖拽
    if (m_dragMode == DragMode::Thumb) {
        const QRect scrollbarRect = getScrollbarRect();
        const QRect thumbRect = getScrollbarThumbRect();
        
        if (scrollbarRect.isValid() && thumbRect.isValid()) {
            const int deltaY = pos.y() - m_dragStartPos.y();
            const int availableTrackHeight = scrollbarRect.height() - thumbRect.height();
            
            if (availableTrackHeight > 0) {
                const float scrollRatio = static_cast<float>(deltaY) / static_cast<float>(availableTrackHeight);
                const int deltaScrollY = static_cast<int>(maxScrollY() * scrollRatio);
                setScrollY(m_dragStartScrollY + deltaScrollY);
            }
        }
        return true;
    } else if (m_dragMode == DragMode::Content) {
        const int deltaY = m_dragStartPos.y() - pos.y(); // 注意方向：向上拖拽是正值
        setScrollY(m_dragStartScrollY + deltaY);
        return true;
    }

    // 传递给子组件
    if (m_child) {
        return m_child->onMouseMove(pos);
    }

    return wasHovered != m_thumbHovered; // 如果悬停状态改变，则需要重绘
}

bool UiScrollView::onMouseRelease(const QPoint& pos) {
    const bool wasDragging = (m_dragMode != DragMode::None);
    m_dragMode = DragMode::None;
    m_thumbPressed = false;

    // 传递给子组件
    if (m_child && !wasDragging) {
        return m_child->onMouseRelease(pos);
    }

    return wasDragging;
}

bool UiScrollView::tick() {
    bool any = false;
    if (m_child) {
        any = m_child->tick();
    }
    return any;
}

void UiScrollView::applyTheme(bool isDark) {
    if (isDark) {
        // 深色主题
        m_trackColor = QColor(40, 40, 40, 180);
        m_thumbColor = QColor(120, 120, 120, 200);
        m_thumbHoverColor = QColor(140, 140, 140, 220);
        m_thumbPressColor = QColor(160, 160, 160, 240);
    } else {
        // 浅色主题  
        m_trackColor = QColor(240, 240, 240, 180);
        m_thumbColor = QColor(180, 180, 180, 200);
        m_thumbHoverColor = QColor(150, 150, 150, 220);
        m_thumbPressColor = QColor(120, 120, 120, 240);
    }

    // 传递主题变化给子组件
    if (m_child) {
        m_child->onThemeChanged(isDark);
    }
}