[English](../../../doc/presentation/components/scroll-view.md) | **简体中文**

# ScrollView 组件

## 概述

ScrollView 组件为超出可见区域的内容提供高效的滚动功能。它支持垂直和水平滚动，具有可自定义的滚动条、平滑滚动动画、惯性滚动以及触摸设备的手势支持。

## 核心特性

- **多方向滚动**: 垂直、水平或双向滚动
- **自定义滚动条**: 带自动隐藏功能的样式化滚动条
- **平滑动画**: 基于惯性的滚动和缓动效果
- **内容裁剪**: 自动内容边界管理
- **手势支持**: 触摸/触控板手势识别
- **性能优化**: 视口剔除和懒加载内容

## 基本用法

### 简单垂直滚动

```cpp
auto scrollView = std::make_unique<UiScrollView>();

// 创建大于容器的内容
auto content = std::make_unique<UiPanel>();
content->setDirection(UiPanel::Direction::Vertical);
content->setSpacing(8);

// 添加多个项目以演示滚动
for (int i = 0; i < 100; ++i) {
    auto item = std::make_unique<UiButton>();
    item->setText(QString("项目 %1").arg(i + 1));
    content->addChild(std::move(item));
}

// 设置内容并配置滚动
scrollView->setContent(std::move(content));
scrollView->setScrollPolicy(Qt::ScrollBarAsNeeded, Qt::ScrollBarAsNeeded);
scrollView->setScrollSpeed(1.5f);
```

### 水平滚动画廊

```cpp
auto galleryScroll = std::make_unique<UiScrollView>();

// 创建水平内容布局
auto gallery = std::make_unique<UiPanel>();
gallery->setDirection(UiPanel::Direction::Horizontal);
gallery->setSpacing(12);

// 添加画廊项目
for (const auto& imagePath : imagePaths) {
    auto imageItem = std::make_unique<ImageComponent>();
    imageItem->setImagePath(imagePath);
    imageItem->setFixedSize(QSize(200, 150));
    gallery->addChild(std::move(imageItem));
}

galleryScroll->setContent(std::move(gallery));
galleryScroll->setScrollPolicy(Qt::ScrollBarAlwaysOff, Qt::ScrollBarAsNeeded);
galleryScroll->setHorizontalScrollEnabled(true);
galleryScroll->setVerticalScrollEnabled(false);
```

## 配置选项

### 滚动策略

```cpp
class UiScrollView {
public:
    // 滚动条可见性
    void setScrollPolicy(Qt::ScrollBarPolicy horizontal, Qt::ScrollBarPolicy vertical);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    
    // 滚动方向
    void setHorizontalScrollEnabled(bool enabled);
    void setVerticalScrollEnabled(bool enabled);
    void setBothDirectionsEnabled(bool enabled);
    
    // 滚动行为
    void setScrollSpeed(float speed);               // 速度倍数（默认：1.0）
    void setScrollAcceleration(float acceleration); // 动画加速度
    void setScrollDeceleration(float deceleration); // 动画减速度
    void setMomentumScrolling(bool enabled);        // 手势后的惯性滚动
    
private:
    Qt::ScrollBarPolicy m_horizontalPolicy = Qt::ScrollBarAsNeeded;
    Qt::ScrollBarPolicy m_verticalPolicy = Qt::ScrollBarAsNeeded;
    float m_scrollSpeed = 1.0f;
    bool m_momentumScrolling = true;
};
```

### 内容管理

```cpp
class UiScrollView {
public:
    // 内容分配
    void setContent(std::unique_ptr<IUiComponent> content);
    IUiComponent* content() const;
    
    // 内容边界
    void setContentSize(const QSize& size);
    QSize contentSize() const;
    void setContentMargins(const QMargins& margins);
    QMargins contentMargins() const;
    
    // 自动尺寸调整
    void setAutoContentSize(bool enabled);          // 调整内容以适应子项
    void setContentSizePolicy(SizePolicy policy);  // 处理内容尺寸调整的方式
    
private:
    std::unique_ptr<IUiComponent> m_content;
    QSize m_contentSize;
    QMargins m_contentMargins{8, 8, 8, 8};
    bool m_autoContentSize = true;
};
```

## 滚动控制

### 程序化滚动

```cpp
class UiScrollView {
public:
    // 位置控制
    void scrollTo(const QPoint& position, bool animated = true);
    void scrollBy(const QPoint& delta, bool animated = true);
    QPoint scrollPosition() const;
    void setScrollPosition(const QPoint& position);
    
    // 边界滚动
    void scrollToTop(bool animated = true);
    void scrollToBottom(bool animated = true);
    void scrollToLeft(bool animated = true);
    void scrollToRight(bool animated = true);
    void scrollToBeginning(bool animated = true);
    void scrollToEnd(bool animated = true);
    
    // 内容对齐
    void scrollToComponent(IUiComponent* component, bool animated = true);
    void centerOnComponent(IUiComponent* component, bool animated = true);
    void ensureComponentVisible(IUiComponent* component, const QMargins& margins = QMargins());
    
    // 相对滚动
    void scrollPageUp();
    void scrollPageDown();
    void scrollPageLeft();
    void scrollPageRight();
    
private:
    QPoint m_scrollPosition;
    QPoint m_targetScrollPosition;
    QPropertyAnimation* m_scrollAnimation = nullptr;
};
```

## 滚动条自定义

### 滚动条外观

```cpp
class UiScrollView {
public:
    // 滚动条尺寸
    void setScrollBarWidth(int width);
    void setScrollBarMinLength(int minLength);
    void setScrollBarMargins(const QMargins& margins);
    void setScrollBarSpacing(int spacing);          // 滚动条之间的间距
    
    // 滚动条样式
    void setScrollBarColor(const QColor& color);
    void setScrollBarBackgroundColor(const QColor& color);
    void setScrollBarHoverColor(const QColor& color);
    void setScrollBarActiveColor(const QColor& color);
    void setScrollBarRadius(float radius);
    void setScrollBarOpacity(float opacity);
    
    // 自动隐藏行为
    void setScrollBarAutoHide(bool enabled);
    void setScrollBarHideDelay(int delayMs);        // 隐藏前的延迟
    void setScrollBarFadeSpeed(int fadeMs);         // 淡出动画持续时间
    
private:
    struct ScrollBar {
        QRect bounds;
        QRect handleBounds;
        QColor color;
        QColor backgroundColor;
        float opacity = 1.0f;
        bool isVisible = true;
        bool isHovered = false;
        bool isDragging = false;
        QTimer* hideTimer = nullptr;
        QPropertyAnimation* fadeAnimation = nullptr;
    };
    
    ScrollBar m_verticalScrollBar;
    ScrollBar m_horizontalScrollBar;
    int m_scrollBarWidth = 12;
    bool m_scrollBarAutoHide = true;
    int m_scrollBarHideDelay = 1000;
};
```

## 动画系统

### 平滑滚动

```cpp
class UiScrollView {
public:
    // 动画配置
    void setScrollAnimationEnabled(bool enabled);
    void setScrollAnimationDuration(int ms);
    void setScrollAnimationEasing(QEasingCurve::Type easing);
    void setScrollAnimationCurve(const QEasingCurve& curve);
    
    // 自定义动画
    void setCustomScrollAnimation(std::function<void(QPoint, QPoint, int)> animator);
    
private:
    bool m_scrollAnimationEnabled = true;
    int m_scrollAnimationDuration = 250;
    QEasingCurve::Type m_scrollAnimationEasing = QEasingCurve::OutCubic;
};
```

### 惯性滚动

```cpp
class UiScrollView {
private:
    struct MomentumState {
        QPoint velocity;
        QPoint lastPosition;
        QElapsedTimer timer;
        bool isActive = false;
        float deceleration = 0.95f;
        float minVelocity = 0.5f;
    } m_momentum;
    
    void startMomentumScrolling(const QPoint& velocity) {
        if (!m_momentumScrolling) return;
        
        m_momentum.velocity = velocity;
        m_momentum.isActive = true;
        m_momentum.timer.start();
        
        // 开始惯性动画
        QTimer::singleShot(16, this, &UiScrollView::updateMomentumScrolling);
    }
};
```

## 事件处理

### 鼠标和滚轮事件

```cpp
bool UiScrollView::onMousePress(const QPoint& pos) {
    // 停止任何正在进行的惯性滚动
    m_momentum.isActive = false;
    
    // 首先检查滚动条交互
    if (handleScrollBarMousePress(pos)) {
        return true;
    }
    
    // 开始潜在的拖拽滚动
    m_dragScrolling = true;
    m_dragStartPos = pos;
    m_dragStartScroll = m_scrollPosition;
    m_dragVelocityTracker.clear();
    m_dragVelocityTracker.addPoint(pos, QTime::currentTime());
    
    return true;
}

bool UiScrollView::onWheel(const QPointF& delta) {
    // 计算滚动增量
    QPoint scrollDelta = QPoint(
        static_cast<int>(delta.x() * m_scrollSpeed * 20),
        static_cast<int>(delta.y() * m_scrollSpeed * 20)
    );
    
    // 应用滚动
    scrollBy(scrollDelta, false);
    
    // 临时显示滚动条
    showScrollBarsTemporarily();
    
    return true;
}
```

## 性能优化

### 视口剔除

```cpp
class UiScrollView {
private:
    struct ViewportCulling {
        bool enabled = true;
        QRect visibleRect;
        QMargins cullingMargins{50, 50, 50, 50}; // 用于平滑滚动的额外边距
        QSet<IUiComponent*> visibleComponents;
        QSet<IUiComponent*> culledComponents;
    } m_culling;
    
    void updateViewportCulling() {
        if (!m_culling.enabled || !m_content) return;
        
        // 计算可见矩形
        m_culling.visibleRect = QRect(m_scrollPosition, size()).marginsAdded(m_culling.cullingMargins);
        
        // 更新组件可见性
        updateComponentCulling(m_content.get());
    }
};
```

## 相关文档

- [UI 框架概览](../ui-framework/overview.md)
- [布局系统](../ui-framework/layouts.md)
- [主题系统](../ui-framework/theme-and-rendering.md)
- [TabView 组件](tab-view.md)
- [NavRail 组件](nav-rail.md)