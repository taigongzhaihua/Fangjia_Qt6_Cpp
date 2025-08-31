# 滚动容器详解（中文文档）

本文档详细介绍 UiScrollView 滚动容器的使用方法、行为特性、滚动条渲染、主题适配和与 IScrollContent 的交互机制。

## UiScrollView 概述

`UiScrollView`（位于 `presentation/ui/containers/UiScrollView.h`）是一个垂直滚动容器，通过"移动子项 viewport 顶坐标 + 父级裁剪"的方式实现滚动效果。

### 核心特性

- **单子项容器**: 包装一个可滚动的子组件
- **Width-bounded 测量**: 宽度约束子项，高度允许超出视口
- **Viewport 顶坐标滚动**: 通过调整子项的视口偏移实现滚动
- **滚动条渲染**: 自动显示/隐藏滚动条，支持淡入淡出动画
- **主题自适应**: 滚动条颜色跟随主题变化
- **重复滚动**: 支持滚动按钮的连续滚动操作

## 基本用法

### setChild 设置内容

```cpp
#include "containers/UiScrollView.h"

auto scrollView = std::make_unique<UiScrollView>();

// 设置可滚动的子内容
auto longContent = std::make_unique<UiPanel>();
// ... 添加大量内容到 longContent ...

scrollView->setChild(std::move(longContent));
```

### width-bounded 测量策略

UiScrollView 的测量逻辑：

```cpp
QSizeF UiScrollView::measure(const SizeConstraints& constraints) {
    if (!m_child) return QSizeF(0, 0);
    
    // 子项宽度受约束，高度不受限制（允许内容超出视口）
    SizeConstraints childConstraints = constraints;
    childConstraints.maxSize.setHeight(std::numeric_limits<float>::infinity());
    
    QSizeF childSize = m_child->measure(childConstraints);
    
    // ScrollView 本身的尺寸 = 视口尺寸（可能小于内容尺寸）
    return QSizeF(
        std::min(childSize.width(), constraints.maxSize.width()),
        std::min(childSize.height(), constraints.maxSize.height())
    );
}
```

### viewport 顶坐标滚动策略

```cpp
void UiScrollView::arrange(const QRectF& rect) {
    m_viewportRect = rect;
    
    if (!m_child) return;
    
    // 子项的实际大小（可能大于视口）
    QRectF childRect(0, 0, rect.width(), m_contentHeight);
    m_child->arrange(childRect);
    
    // 设置子项的视口，顶坐标根据滚动偏移调整
    if (auto* content = dynamic_cast<IUiContent*>(m_child.get())) {
        QRectF childViewport = rect;
        childViewport.moveTop(m_scrollOffset);  // 关键：移动视口顶坐标
        content->setViewportRect(childViewport);
    }
}
```

滚动的工作原理：
- **向下滚动**: 增加 `m_scrollOffset`，子项视口顶坐标下移，显示内容的下方部分
- **向上滚动**: 减少 `m_scrollOffset`，子项视口顶坐标上移，显示内容的上方部分
- **裁剪效果**: 父级容器裁剪确保只有视口范围内的内容可见

## 滚动条渲染

### 滚动条组件

UiScrollView 包含垂直滚动条组件：

```cpp
class UiScrollView {
private:
    // 滚动条相关成员
    bool m_showScrollbar;           // 是否显示滚动条
    float m_scrollbarAlpha;         // 滚动条透明度（淡入淡出）
    QRectF m_scrollbarTrack;        // 滚动条轨道矩形
    QRectF m_scrollbarThumb;        // 滚动条拇指矩形
    bool m_scrollbarHovered;        // 滚动条悬停状态
    bool m_scrollbarPressed;        // 滚动条按下状态
};
```

### 轨道和拇指计算

```cpp
void UiScrollView::updateScrollbarGeometry() {
    if (m_contentHeight <= m_viewportRect.height()) {
        m_showScrollbar = false;
        return;
    }
    
    m_showScrollbar = true;
    
    // 滚动条轨道：右侧固定宽度区域
    const float trackWidth = 12.0f;
    m_scrollbarTrack = QRectF(
        m_viewportRect.right() - trackWidth,
        m_viewportRect.top(),
        trackWidth,
        m_viewportRect.height()
    );
    
    // 拇指大小和位置
    float thumbHeight = (m_viewportRect.height() / m_contentHeight) * m_scrollbarTrack.height();
    thumbHeight = std::max(thumbHeight, 20.0f); // 最小拇指高度
    
    float thumbTop = (m_scrollOffset / (m_contentHeight - m_viewportRect.height())) 
                   * (m_scrollbarTrack.height() - thumbHeight);
    
    m_scrollbarThumb = QRectF(
        m_scrollbarTrack.left() + 2,    // 左边距
        m_scrollbarTrack.top() + thumbTop,
        m_scrollbarTrack.width() - 4,   // 左右各2px边距
        thumbHeight
    );
}
```

### 淡入淡出动画

滚动条支持自动淡入淡出：

```cpp
// 动画常量
static constexpr int SCROLLBAR_FADE_DELAY_MS = 1000;    // 停止滚动后延时
static constexpr int SCROLLBAR_FADE_DURATION_MS = 300;   // 淡出时长
static constexpr float BASE_ALPHA = 0.7f;               // 基础透明度

bool UiScrollView::tick() {
    bool needsRedraw = false;
    
    // 处理滚动条淡入淡出动画
    if (m_showScrollbar) {
        const auto now = std::chrono::steady_clock::now();
        
        if (m_scrolling) {
            // 正在滚动：完全显示
            m_scrollbarAlpha = BASE_ALPHA;
            m_lastScrollTime = now;
        } else {
            // 停止滚动：延时后淡出
            auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
                now - m_lastScrollTime).count();
            
            if (elapsed > SCROLLBAR_FADE_DELAY_MS) {
                // 开始淡出
                float fadeProgress = std::min(1.0f, 
                    (elapsed - SCROLLBAR_FADE_DELAY_MS) / float(SCROLLBAR_FADE_DURATION_MS));
                m_scrollbarAlpha = BASE_ALPHA * (1.0f - fadeProgress);
                needsRedraw = true;
            }
        }
    }
    
    return needsRedraw;
}
```

### 滚动条渲染

```cpp
void UiScrollView::append(Render::FrameData& fd) const {
    // 渲染子内容
    if (m_child) {
        m_child->append(fd);
    }
    
    // 渲染滚动条
    if (m_showScrollbar && m_scrollbarAlpha > 0.01f) {
        // 轨道背景
        QColor trackColor = m_isDark ? QColor(255, 255, 255, 30) : QColor(0, 0, 0, 30);
        trackColor.setAlphaF(trackColor.alphaF() * m_scrollbarAlpha);
        fd.addRoundedRect(m_scrollbarTrack, 6.0f, trackColor);
        
        // 拇指
        QColor thumbColor = m_isDark ? QColor(255, 255, 255, 120) : QColor(0, 0, 0, 120);
        if (m_scrollbarHovered) thumbColor.setAlpha(150);
        if (m_scrollbarPressed) thumbColor.setAlpha(180);
        thumbColor.setAlphaF(thumbColor.alphaF() * m_scrollbarAlpha);
        
        fd.addRoundedRect(m_scrollbarThumb, 4.0f, thumbColor);
    }
}
```

## 重复滚动按钮

UiScrollView 支持滚动区域端部的重复滚动操作：

### 重复滚动常量

```cpp
static constexpr int REPEAT_INITIAL_DELAY_MS = 500;  // 初始延时
static constexpr int REPEAT_INTERVAL_MS = 50;       // 重复间隔
static constexpr float REPEAT_STEP_SIZE = 30.0f;    // 每次滚动步长
```

### 重复滚动逻辑

```cpp
bool UiScrollView::onMousePress(const QPointF& localPos) {
    if (hitTestScrollbar(localPos)) {
        if (localPos.y() < m_scrollbarThumb.top()) {
            // 点击拇指上方：向上滚动
            startRepeatScroll(-REPEAT_STEP_SIZE);
        } else if (localPos.y() > m_scrollbarThumb.bottom()) {
            // 点击拇指下方：向下滚动
            startRepeatScroll(REPEAT_STEP_SIZE);
        } else {
            // 点击拇指：开始拖拽
            startThumbDrag(localPos);
        }
        return true;
    }
    return false;
}

void UiScrollView::startRepeatScroll(float step) {
    m_repeatScrollStep = step;
    m_repeatScrollActive = true;
    m_repeatScrollTimer.start();
    
    // 立即执行第一次滚动
    scroll(step);
}

bool UiScrollView::tick() {
    // ... 淡入淡出动画 ...
    
    // 处理重复滚动
    if (m_repeatScrollActive) {
        auto elapsed = m_repeatScrollTimer.elapsed();
        
        // 初始延时后开始重复
        if (elapsed > REPEAT_INITIAL_DELAY_MS) {
            auto sinceLastRepeat = elapsed - m_lastRepeatTime;
            if (sinceLastRepeat >= REPEAT_INTERVAL_MS) {
                scroll(m_repeatScrollStep);
                m_lastRepeatTime = elapsed;
            }
        }
        needsRedraw = true;
    }
    
    return needsRedraw;
}
```

## 主题自适应配色

### applyTheme 流程

```cpp
void UiScrollView::onThemeChanged(bool isDark) {
    m_isDark = isDark;
    
    // 更新滚动条颜色（在下次渲染时生效）
    updateScrollbarColors();
    
    // 传播主题到子组件
    if (m_child) {
        m_child->onThemeChanged(isDark);
    }
}

void UiScrollView::updateScrollbarColors() {
    if (m_isDark) {
        m_trackColor = QColor(255, 255, 255, 30);     // 暗色主题：浅色轨道
        m_thumbColor = QColor(255, 255, 255, 120);    // 暗色主题：浅色拇指
    } else {
        m_trackColor = QColor(0, 0, 0, 30);           // 亮色主题：深色轨道
        m_thumbColor = QColor(0, 0, 0, 120);          // 亮色主题：深色拇指
    }
}
```

### 主题色动态计算

颜色根据交互状态动态调整：

```cpp
QColor UiScrollView::getThumbColor() const {
    QColor color = m_thumbColor;
    
    // 交互状态调整
    if (m_scrollbarPressed) {
        color.setAlpha(std::min(255, color.alpha() + 60));  // 按下时更不透明
    } else if (m_scrollbarHovered) {
        color.setAlpha(std::min(255, color.alpha() + 30));  // 悬停时稍微不透明
    }
    
    // 应用淡入淡出透明度
    color.setAlphaF(color.alphaF() * m_scrollbarAlpha);
    
    return color;
}
```

## 与 IScrollContent 交互

### IScrollContent 接口

子组件可以实现 `IScrollContent` 接口以支持滚动通知：

```cpp
class IScrollContent {
public:
    // 滚动位置变化通知
    virtual void onScrollChanged(float offset, float maxOffset) = 0;
    
    // 视口大小变化通知
    virtual void onViewportChanged(const QSizeF& viewportSize) = 0;
};
```

### 滚动事件通知

```cpp
void UiScrollView::setScrollOffset(float offset) {
    float oldOffset = m_scrollOffset;
    m_scrollOffset = std::clamp(offset, 0.0f, getMaxScrollOffset());
    
    if (std::abs(m_scrollOffset - oldOffset) > 0.1f) {
        // 通知支持滚动的子组件
        if (auto* scrollContent = dynamic_cast<IScrollContent*>(m_child.get())) {
            scrollContent->onScrollChanged(m_scrollOffset, getMaxScrollOffset());
        }
        
        // 更新滚动条几何
        updateScrollbarGeometry();
        
        // 标记需要重新安排子组件
        invalidateArrangement();
    }
}
```

### 实际应用示例

```cpp
// 长列表组件实现 IScrollContent
class UiListView : public IUiComponent, public IScrollContent {
public:
    void onScrollChanged(float offset, float maxOffset) override {
        // 更新可见项目范围，实现虚拟化
        updateVisibleRange(offset);
        
        // 触发滚动事件给外部监听器
        if (m_onScroll) {
            m_onScroll(offset / maxOffset); // 归一化滚动位置
        }
    }
    
    void onViewportChanged(const QSizeF& viewportSize) override {
        // 重新计算可见项目数量
        m_visibleItemCount = static_cast<int>(viewportSize.height() / m_itemHeight);
        updateVisibleRange(m_currentScrollOffset);
    }
};

// 在 ScrollView 中使用
auto scrollView = std::make_unique<UiScrollView>();
auto listView = std::make_unique<UiListView>();
scrollView->setChild(std::move(listView));
```

## 声明式 ScrollView API

声明式封装位于 `presentation/ui/declarative/ScrollView.h`：

```cpp
// 声明式 ScrollView 使用
auto scrollableContent = UI::scrollView(
    UI::panel()
        ->direction(UI::Panel::Direction::Vertical)
        ->spacing(8)
        ->children({
            UI::text("Item 1"),
            UI::text("Item 2"),
            UI::text("Item 3"),
            // ... 更多内容 ...
        })
)->showScrollbar(true)
 ->scrollbarWidth(12)
 ->padding(16);
```

### 声明式配置选项

```cpp
class ScrollView : public Widget {
public:
    // 滚动条控制
    std::shared_ptr<ScrollView> showScrollbar(bool show);
    std::shared_ptr<ScrollView> scrollbarWidth(float width);
    std::shared_ptr<ScrollView> autoHide(bool enable);
    
    // 滚动行为
    std::shared_ptr<ScrollView> scrollStep(float step);
    std::shared_ptr<ScrollView> wheelSensitivity(float sensitivity);
    
    // 事件回调
    std::shared_ptr<ScrollView> onScroll(std::function<void(float)> callback);
    std::shared_ptr<ScrollView> onScrollEnd(std::function<void()> callback);
};
```

## 性能优化

### 渲染优化

- **裁剪传播**: 只渲染视口内的内容
- **滚动条缓存**: 滚动条几何信息缓存，避免重复计算
- **动画优化**: 使用高效的透明度动画，避免不必要的重绘

### 内存管理

- **延迟加载**: 子组件可实现虚拟化以处理大量数据
- **视口外释放**: 滚动到视口外的内容可以被释放
- **滚动位置持久化**: 保存和恢复滚动位置

## 相关文档

- [UI 架构](./UI_ARCHITECTURE.zh-CN.md) - IUiComponent 生命周期和 IUiContent 接口
- [布局系统](./LAYOUTS.zh-CN.md) - 布局容器与 UiScrollView 的协作
- [声明式概览](./DECLARATIVE_OVERVIEW.zh-CN.md) - 声明式 ScrollView API 详解
- [声明式 TopBar](./DECLARATIVE_NAV_TOPBAR.zh-CN.md) - AppShell 中的滚动视图集成