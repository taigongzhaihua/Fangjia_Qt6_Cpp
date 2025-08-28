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
    m_animClock.start(); // 启动动画计时器
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
    
    if (!scrollbarRect.isValid() || m_thumbAlpha <= 0.0f) return;

    // 渲染滚动条轨道（透明，Fluent 风格）
    QColor trackColor = m_trackColor;
    trackColor.setAlphaF(trackColor.alphaF() * m_thumbAlpha);
    
    fd.roundedRects.push_back(Render::RoundedRectCmd{
        .rect = QRectF(scrollbarRect),
        .radiusPx = static_cast<float>(THUMB_RADIUS),  // 使用圆角
        .color = trackColor,
        .clipRect = QRectF(m_viewport)
    });

    // 渲染滚动条 thumb
    if (thumbRect.isValid()) {
        QColor thumbColor = m_thumbColor;
        if (m_thumbPressed) {
            thumbColor = m_thumbPressColor;
        } else if (m_thumbHovered) {
            thumbColor = m_thumbHoverColor;
        }
        
        // 应用淡入淡出透明度
        thumbColor.setAlphaF(thumbColor.alphaF() * m_thumbAlpha);

        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(thumbRect),
            .radiusPx = static_cast<float>(THUMB_RADIUS),  // 使用圆角
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
    showScrollbar(); // 显示滚动条
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
    showScrollbar(); // 显示滚动条
}

void UiScrollView::startContentDrag(const QPoint& pos) {
    m_dragMode = DragMode::Content;
    m_dragStartPos = pos;
    m_dragStartScrollY = m_scrollY;
    showScrollbar(); // 显示滚动条
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

bool UiScrollView::onWheel(const QPoint& pos, const QPoint& angleDelta) {
    // 检查位置是否在当前组件边界内
    if (!bounds().contains(pos)) {
        return false;
    }
    
    // 计算滚动步长：基于 angleDelta.y()，默认 48px/刻度（120单位）
    const int wheelStep = 48;
    const int deltaY = angleDelta.y();
    if (deltaY == 0) {
        return false;
    }
    
    // 计算滚动偏移（向上滚动为负值，向下滚动为正值）
    const int scrollDelta = -(deltaY * wheelStep) / 120;
    const int newScrollY = m_scrollY + scrollDelta;
    
    // 设置新的滚动位置（内部会进行范围限制）
    setScrollY(newScrollY);
    
    // 显示滚动条（立即淡入）
    showScrollbar();
    
    // 如果有滚动内容，则消费此事件
    return maxScrollY() > 0;
}

bool UiScrollView::tick() {
    bool any = false;
    if (m_child) {
        any = m_child->tick();
    }
    
    // 处理滚动条淡出动画
    if (m_animActive) {
        const qint64 now = m_animClock.elapsed();
        const qint64 timeSinceInteract = now - m_lastInteractMs;
        
        if (timeSinceInteract > FADE_DELAY_MS) {
            // 开始淡出
            const qint64 fadeElapsed = timeSinceInteract - FADE_DELAY_MS;
            if (fadeElapsed >= FADE_DURATION_MS) {
                // 淡出完成
                m_thumbAlpha = 0.0f;
                m_animActive = false;
            } else {
                // 淡出进行中
                const float t = static_cast<float>(fadeElapsed) / static_cast<float>(FADE_DURATION_MS);
                m_thumbAlpha = 1.0f - t;
                any = true; // 需要继续动画
            }
        } else {
            // 延时期间，保持完全显示
            m_thumbAlpha = 1.0f;
            any = true; // 需要继续动画
        }
    }
    
    return any;
}

void UiScrollView::showScrollbar() {
    m_thumbAlpha = 1.0f;
    m_animActive = true;
    m_lastInteractMs = m_animClock.elapsed();
}

void UiScrollView::applyTheme(bool isDark) {
    if (isDark) {
        // 深色主题 - Fluent 风格
        m_trackColor = QColor(255, 255, 255, 25);       // 半透明白色轨道
        m_thumbColor = QColor(255, 255, 255, 120);      // 半透明白色拇指
        m_thumbHoverColor = QColor(255, 255, 255, 160); // 悬停时更明显
        m_thumbPressColor = QColor(255, 255, 255, 200); // 按下时最明显
    } else {
        // 浅色主题 - Fluent 风格  
        m_trackColor = QColor(0, 0, 0, 25);             // 半透明黑色轨道
        m_thumbColor = QColor(0, 0, 0, 120);            // 半透明黑色拇指
        m_thumbHoverColor = QColor(0, 0, 0, 160);       // 悬停时更明显
        m_thumbPressColor = QColor(0, 0, 0, 200);       // 按下时最明显
    }

    // 传递主题变化给子组件
    if (m_child) {
        m_child->onThemeChanged(isDark);
    }
}