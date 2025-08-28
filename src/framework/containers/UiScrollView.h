#pragma once
#include "ILayoutable.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qcolor.h>
#include <RenderData.hpp>

class IconCache;
class QOpenGLFunctions;

// 垂直滚动容器：支持鼠标拖拽和滚动条操作
class UiScrollView final : public IUiComponent, public IUiContent, public ILayoutable {
public:
    UiScrollView();
    ~UiScrollView() override = default;

    // 子组件管理
    void setChild(IUiComponent* child) { m_child = child; }
    IUiComponent* child() const noexcept { return m_child; }

    // 滚动控制
    void setScrollY(int scrollY);
    int scrollY() const noexcept { return m_scrollY; }
    int maxScrollY() const noexcept;
    int contentHeight() const noexcept { return m_contentHeight; }

    // IUiContent
    void setViewportRect(const QRect& r) override;

    // ILayoutable
    QSize measure(const SizeConstraints& cs) override;
    void arrange(const QRect& finalRect) override;

    // IUiComponent
    void updateLayout(const QSize& windowSize) override;
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
    void append(Render::FrameData& fd) const override;
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    bool tick() override;
    QRect bounds() const override { return m_viewport; }
    void applyTheme(bool isDark) override;

private:
    // 布局计算
    void updateChildLayout();
    void measureContent();
    QRect getChildViewport() const;
    QRect getScrollbarRect() const;
    QRect getScrollbarThumbRect() const;

    // 滚动条交互
    bool isScrollbarVisible() const;
    bool isPointInScrollbar(const QPoint& pos) const;
    bool isPointInThumb(const QPoint& pos) const;
    void startThumbDrag(const QPoint& pos);
    void handleTrackClick(const QPoint& pos);
    
    // 内容拖拽
    void startContentDrag(const QPoint& pos);

    // 滚动条渲染
    void renderScrollbar(Render::FrameData& fd) const;

    // 滚动限制
    void clampScrollY();

private:
    // 子组件
    IUiComponent* m_child{ nullptr }; // 非拥有

    // 视口与布局
    QRect m_viewport;
    int m_contentHeight{ 0 };
    int m_scrollY{ 0 };

    // 交互状态
    enum class DragMode { None, Content, Thumb };
    DragMode m_dragMode{ DragMode::None };
    QPoint m_dragStartPos;
    int m_dragStartScrollY{ 0 };
    int m_dragStartThumbY{ 0 };

    // 悬停状态
    bool m_thumbHovered{ false };
    bool m_thumbPressed{ false };

    // 滚动条配置
    static constexpr int SCROLLBAR_WIDTH = 12;
    static constexpr int THUMB_MIN_HEIGHT = 20;

    // 主题颜色
    QColor m_trackColor;
    QColor m_thumbColor;
    QColor m_thumbHoverColor;
    QColor m_thumbPressColor;

    // 资源上下文
    IconCache* m_cache{ nullptr };
    QOpenGLFunctions* m_gl{ nullptr };
    float m_dpr{ 1.0f };
};