# ScrollView Component

## Overview

The ScrollView component provides efficient scrolling for content that exceeds the visible area. It supports both vertical and horizontal scrolling with customizable scroll bars, smooth scrolling animations, momentum scrolling, and gesture support for touch devices.

## Core Features

- **Multi-Directional Scrolling**: Vertical, horizontal, or both
- **Custom Scroll Bars**: Styled scroll bars with auto-hide functionality
- **Smooth Animations**: Momentum-based scrolling with easing
- **Content Clipping**: Automatic content boundary management
- **Gesture Support**: Touch/trackpad gesture recognition
- **Performance Optimization**: Viewport culling and lazy content loading

## Basic Usage

### Simple Vertical Scrolling

```cpp
auto scrollView = std::make_unique<UiScrollView>();

// Create content that's larger than the container
auto content = std::make_unique<UiPanel>();
content->setDirection(UiPanel::Direction::Vertical);
content->setSpacing(8);

// Add many items to demonstrate scrolling
for (int i = 0; i < 100; ++i) {
    auto item = std::make_unique<UiButton>();
    item->setText(QString("Item %1").arg(i + 1));
    content->addChild(std::move(item));
}

// Set content and configure scrolling
scrollView->setContent(std::move(content));
scrollView->setScrollPolicy(Qt::ScrollBarAsNeeded, Qt::ScrollBarAsNeeded);
scrollView->setScrollSpeed(1.5f);
```

### Horizontal Scrolling Gallery

```cpp
auto galleryScroll = std::make_unique<UiScrollView>();

// Create horizontal content layout
auto gallery = std::make_unique<UiPanel>();
gallery->setDirection(UiPanel::Direction::Horizontal);
gallery->setSpacing(12);

// Add gallery items
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

## Configuration

### Scroll Policies

```cpp
class UiScrollView {
public:
    // Scroll bar visibility
    void setScrollPolicy(Qt::ScrollBarPolicy horizontal, Qt::ScrollBarPolicy vertical);
    void setHorizontalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    void setVerticalScrollBarPolicy(Qt::ScrollBarPolicy policy);
    
    // Scroll directions
    void setHorizontalScrollEnabled(bool enabled);
    void setVerticalScrollEnabled(bool enabled);
    void setBothDirectionsEnabled(bool enabled);
    
    // Scroll behavior
    void setScrollSpeed(float speed);               // Speed multiplier (default: 1.0)
    void setScrollAcceleration(float acceleration); // Animation acceleration
    void setScrollDeceleration(float deceleration); // Animation deceleration
    void setMomentumScrolling(bool enabled);        // Momentum after gesture
    
private:
    Qt::ScrollBarPolicy m_horizontalPolicy = Qt::ScrollBarAsNeeded;
    Qt::ScrollBarPolicy m_verticalPolicy = Qt::ScrollBarAsNeeded;
    float m_scrollSpeed = 1.0f;
    bool m_momentumScrolling = true;
};
```

### Content Management

```cpp
class UiScrollView {
public:
    // Content assignment
    void setContent(std::unique_ptr<IUiComponent> content);
    IUiComponent* content() const;
    
    // Content bounds
    void setContentSize(const QSize& size);
    QSize contentSize() const;
    void setContentMargins(const QMargins& margins);
    QMargins contentMargins() const;
    
    // Automatic sizing
    void setAutoContentSize(bool enabled);          // Size content to fit children
    void setContentSizePolicy(SizePolicy policy);  // How to handle content sizing
    
private:
    std::unique_ptr<IUiComponent> m_content;
    QSize m_contentSize;
    QMargins m_contentMargins{8, 8, 8, 8};
    bool m_autoContentSize = true;
};
```

## Scroll Control

### Programmatic Scrolling

```cpp
class UiScrollView {
public:
    // Position control
    void scrollTo(const QPoint& position, bool animated = true);
    void scrollBy(const QPoint& delta, bool animated = true);
    QPoint scrollPosition() const;
    void setScrollPosition(const QPoint& position);
    
    // Boundary scrolling
    void scrollToTop(bool animated = true);
    void scrollToBottom(bool animated = true);
    void scrollToLeft(bool animated = true);
    void scrollToRight(bool animated = true);
    void scrollToBeginning(bool animated = true);
    void scrollToEnd(bool animated = true);
    
    // Content alignment
    void scrollToComponent(IUiComponent* component, bool animated = true);
    void centerOnComponent(IUiComponent* component, bool animated = true);
    void ensureComponentVisible(IUiComponent* component, const QMargins& margins = QMargins());
    
    // Relative scrolling
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

### Scroll Bounds

```cpp
class UiScrollView {
public:
    // Scroll limits
    QRect scrollBounds() const;
    QPoint minimumScrollPosition() const;
    QPoint maximumScrollPosition() const;
    
    // Boundary behavior
    void setBounceEnabled(bool enabled);            // Allow over-scroll with bounce-back
    void setBounceStrength(float strength);         // Bounce resistance
    void setOverscrollEnabled(bool enabled);        // Allow scrolling past boundaries
    void setOverscrollDamping(float damping);       // Over-scroll resistance
    
    // Scroll validation
    QPoint clampScrollPosition(const QPoint& position) const;
    bool isValidScrollPosition(const QPoint& position) const;
    
private:
    bool m_bounceEnabled = true;
    float m_bounceStrength = 0.3f;
    bool m_overscrollEnabled = false;
    float m_overscrollDamping = 0.5f;
    
    QPoint clampToBounds(const QPoint& position) const {
        QRect bounds = scrollBounds();
        return QPoint(
            qBound(bounds.left(), position.x(), bounds.right()),
            qBound(bounds.top(), position.y(), bounds.bottom())
        );
    }
};
```

## Scroll Bar Customization

### Scroll Bar Appearance

```cpp
class UiScrollView {
public:
    // Scroll bar dimensions
    void setScrollBarWidth(int width);
    void setScrollBarMinLength(int minLength);
    void setScrollBarMargins(const QMargins& margins);
    void setScrollBarSpacing(int spacing);          // Space between bars
    
    // Scroll bar styling
    void setScrollBarColor(const QColor& color);
    void setScrollBarBackgroundColor(const QColor& color);
    void setScrollBarHoverColor(const QColor& color);
    void setScrollBarActiveColor(const QColor& color);
    void setScrollBarRadius(float radius);
    void setScrollBarOpacity(float opacity);
    
    // Auto-hide behavior
    void setScrollBarAutoHide(bool enabled);
    void setScrollBarHideDelay(int delayMs);        // Delay before hiding
    void setScrollBarFadeSpeed(int fadeMs);         // Fade animation duration
    
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

### Scroll Bar Interaction

```cpp
class UiScrollView {
private:
    bool handleScrollBarMousePress(const QPoint& pos) {
        // Check vertical scroll bar
        if (m_verticalScrollBar.bounds.contains(pos)) {
            if (m_verticalScrollBar.handleBounds.contains(pos)) {
                // Start dragging handle
                m_verticalScrollBar.isDragging = true;
                m_dragStartPos = pos;
                m_dragStartScroll = m_scrollPosition.y();
                return true;
            } else {
                // Click in track - jump to position
                jumpToScrollPosition(pos, Qt::Vertical);
                return true;
            }
        }
        
        // Check horizontal scroll bar
        if (m_horizontalScrollBar.bounds.contains(pos)) {
            if (m_horizontalScrollBar.handleBounds.contains(pos)) {
                m_horizontalScrollBar.isDragging = true;
                m_dragStartPos = pos;
                m_dragStartScroll = m_scrollPosition.x();
                return true;
            } else {
                jumpToScrollPosition(pos, Qt::Horizontal);
                return true;
            }
        }
        
        return false;
    }
    
    void jumpToScrollPosition(const QPoint& clickPos, Qt::Orientation orientation) {
        if (orientation == Qt::Vertical) {
            float ratio = static_cast<float>(clickPos.y() - m_verticalScrollBar.bounds.top()) / 
                         m_verticalScrollBar.bounds.height();
            int targetY = static_cast<int>(ratio * scrollBounds().height());
            scrollTo(QPoint(m_scrollPosition.x(), targetY), true);
        } else {
            float ratio = static_cast<float>(clickPos.x() - m_horizontalScrollBar.bounds.left()) / 
                         m_horizontalScrollBar.bounds.width();
            int targetX = static_cast<int>(ratio * scrollBounds().width());
            scrollTo(QPoint(targetX, m_scrollPosition.y()), true);
        }
    }
};
```

## Animation System

### Smooth Scrolling

```cpp
class UiScrollView {
public:
    // Animation configuration
    void setScrollAnimationEnabled(bool enabled);
    void setScrollAnimationDuration(int ms);
    void setScrollAnimationEasing(QEasingCurve::Type easing);
    void setScrollAnimationCurve(const QEasingCurve& curve);
    
    // Custom animation
    void setCustomScrollAnimation(std::function<void(QPoint, QPoint, int)> animator);
    
private:
    void animateScrollTo(const QPoint& targetPosition) {
        if (!m_scrollAnimationEnabled) {
            setScrollPosition(targetPosition);
            return;
        }
        
        if (m_scrollAnimation) {
            m_scrollAnimation->stop();
        }
        
        m_scrollAnimation = new QPropertyAnimation(this, "scrollPosition");
        m_scrollAnimation->setDuration(m_scrollAnimationDuration);
        m_scrollAnimation->setStartValue(m_scrollPosition);
        m_scrollAnimation->setEndValue(targetPosition);
        m_scrollAnimation->setEasingCurve(m_scrollAnimationEasing);
        
        connect(m_scrollAnimation, &QPropertyAnimation::finished, [this]() {
            m_scrollAnimation->deleteLater();
            m_scrollAnimation = nullptr;
            emit scrollAnimationFinished();
        });
        
        m_scrollAnimation->start();
    }
    
    bool m_scrollAnimationEnabled = true;
    int m_scrollAnimationDuration = 250;
    QEasingCurve::Type m_scrollAnimationEasing = QEasingCurve::OutCubic;
};
```

### Momentum Scrolling

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
        
        // Start momentum animation
        QTimer::singleShot(16, this, &UiScrollView::updateMomentumScrolling);
    }
    
    void updateMomentumScrolling() {
        if (!m_momentum.isActive) return;
        
        // Apply deceleration
        m_momentum.velocity *= m_momentum.deceleration;
        
        // Check if velocity is too low to continue
        float speed = std::sqrt(m_momentum.velocity.x() * m_momentum.velocity.x() + 
                               m_momentum.velocity.y() * m_momentum.velocity.y());
        if (speed < m_momentum.minVelocity) {
            m_momentum.isActive = false;
            return;
        }
        
        // Apply velocity to scroll position
        QPoint newPosition = m_scrollPosition + m_momentum.velocity;
        newPosition = clampToBounds(newPosition);
        
        // Check for boundary collision
        if (newPosition != m_scrollPosition + m_momentum.velocity) {
            // Hit boundary - reduce velocity
            if (newPosition.x() != m_scrollPosition.x() + m_momentum.velocity.x()) {
                m_momentum.velocity.setX(m_momentum.velocity.x() * -0.3f);
            }
            if (newPosition.y() != m_scrollPosition.y() + m_momentum.velocity.y()) {
                m_momentum.velocity.setY(m_momentum.velocity.y() * -0.3f);
            }
        }
        
        setScrollPosition(newPosition);
        
        // Continue momentum
        QTimer::singleShot(16, this, &UiScrollView::updateMomentumScrolling);
    }
};
```

## Event Handling

### Mouse & Wheel Events

```cpp
bool UiScrollView::onMousePress(const QPoint& pos) {
    // Stop any ongoing momentum
    m_momentum.isActive = false;
    
    // Check scroll bar interaction first
    if (handleScrollBarMousePress(pos)) {
        return true;
    }
    
    // Start potential drag scrolling
    m_dragScrolling = true;
    m_dragStartPos = pos;
    m_dragStartScroll = m_scrollPosition;
    m_dragVelocityTracker.clear();
    m_dragVelocityTracker.addPoint(pos, QTime::currentTime());
    
    return true;
}

bool UiScrollView::onMouseMove(const QPoint& pos) {
    if (m_dragScrolling) {
        // Calculate drag delta
        QPoint delta = m_dragStartPos - pos;
        QPoint newScrollPos = m_dragStartScroll + delta;
        
        // Apply scroll position
        setScrollPosition(clampToBounds(newScrollPos));
        
        // Track velocity for momentum
        m_dragVelocityTracker.addPoint(pos, QTime::currentTime());
        
        return true;
    }
    
    // Update scroll bar hover states
    updateScrollBarHover(pos);
    
    return false;
}

bool UiScrollView::onWheel(const QPointF& delta) {
    // Calculate scroll delta
    QPoint scrollDelta = QPoint(
        static_cast<int>(delta.x() * m_scrollSpeed * 20),
        static_cast<int>(delta.y() * m_scrollSpeed * 20)
    );
    
    // Apply scroll
    scrollBy(scrollDelta, false);
    
    // Show scroll bars temporarily
    showScrollBarsTemporarily();
    
    return true;
}
```

### Touch & Gesture Support

```cpp
class UiScrollView {
public:
    // Touch/gesture configuration
    void setTouchScrollEnabled(bool enabled);
    void setGestureRecognitionEnabled(bool enabled);
    void setPinchZoomEnabled(bool enabled);
    void setSwipeThreshold(float threshold);
    
protected:
    bool event(QEvent* event) override {
        switch (event->type()) {
        case QEvent::TouchBegin:
            return handleTouchBegin(static_cast<QTouchEvent*>(event));
        case QEvent::TouchUpdate:
            return handleTouchUpdate(static_cast<QTouchEvent*>(event));
        case QEvent::TouchEnd:
            return handleTouchEnd(static_cast<QTouchEvent*>(event));
        case QEvent::Gesture:
            return handleGesture(static_cast<QGestureEvent*>(event));
        default:
            return IUiComponent::event(event);
        }
    }
    
private:
    bool handleTouchBegin(QTouchEvent* event) {
        const auto& touchPoints = event->touchPoints();
        if (touchPoints.size() == 1) {
            // Single finger - start scroll tracking
            m_touchScrolling = true;
            m_touchStartPos = touchPoints.first().pos().toPoint();
            m_touchStartScroll = m_scrollPosition;
        }
        return true;
    }
    
    bool handleTouchUpdate(QTouchEvent* event) {
        const auto& touchPoints = event->touchPoints();
        if (touchPoints.size() == 1 && m_touchScrolling) {
            // Single finger scroll
            QPoint currentPos = touchPoints.first().pos().toPoint();
            QPoint delta = m_touchStartPos - currentPos;
            setScrollPosition(clampToBounds(m_touchStartScroll + delta));
        }
        return true;
    }
    
    bool m_touchScrollEnabled = true;
    bool m_touchScrolling = false;
    QPoint m_touchStartPos;
    QPoint m_touchStartScroll;
};
```

## Performance Optimization

### Viewport Culling

```cpp
class UiScrollView {
private:
    struct ViewportCulling {
        bool enabled = true;
        QRect visibleRect;
        QMargins cullingMargins{50, 50, 50, 50}; // Extra margin for smooth scrolling
        QSet<IUiComponent*> visibleComponents;
        QSet<IUiComponent*> culledComponents;
    } m_culling;
    
    void updateViewportCulling() {
        if (!m_culling.enabled || !m_content) return;
        
        // Calculate visible rectangle
        m_culling.visibleRect = QRect(m_scrollPosition, size()).marginsAdded(m_culling.cullingMargins);
        
        // Update component visibility
        updateComponentCulling(m_content.get());
    }
    
    void updateComponentCulling(IUiComponent* component) {
        QRect componentBounds = component->bounds();
        bool isVisible = m_culling.visibleRect.intersects(componentBounds);
        
        if (isVisible) {
            if (m_culling.culledComponents.contains(component)) {
                // Component became visible
                m_culling.culledComponents.remove(component);
                m_culling.visibleComponents.insert(component);
                component->setVisible(true);
            }
        } else {
            if (m_culling.visibleComponents.contains(component)) {
                // Component became hidden
                m_culling.visibleComponents.remove(component);
                m_culling.culledComponents.insert(component);
                component->setVisible(false);
            }
        }
        
        // Recursively check child components
        if (auto container = dynamic_cast<IUiContainer*>(component)) {
            for (auto& child : container->children()) {
                updateComponentCulling(child.get());
            }
        }
    }
};
```

### Memory Management

```cpp
class UiScrollView {
private:
    // Efficient velocity tracking
    struct VelocityTracker {
        static constexpr int MAX_SAMPLES = 5;
        
        struct Sample {
            QPoint position;
            QTime timestamp;
        };
        
        QVector<Sample> samples;
        
        void addPoint(const QPoint& pos, const QTime& time) {
            samples.append({pos, time});
            if (samples.size() > MAX_SAMPLES) {
                samples.removeFirst();
            }
        }
        
        QPoint calculateVelocity() const {
            if (samples.size() < 2) return QPoint(0, 0);
            
            const Sample& latest = samples.last();
            const Sample& previous = samples.first();
            
            int deltaTime = previous.timestamp.msecsTo(latest.timestamp);
            if (deltaTime <= 0) return QPoint(0, 0);
            
            QPoint deltaPos = latest.position - previous.position;
            return QPoint(
                deltaPos.x() * 1000 / deltaTime,
                deltaPos.y() * 1000 / deltaTime
            );
        }
        
        void clear() { samples.clear(); }
    };
    
    VelocityTracker m_dragVelocityTracker;
};
```

## Related Documentation

- [UI Framework Overview](../ui-framework/overview.md)
- [Layout System](../ui-framework/layouts.md)
- [Theme System](../ui-framework/theme-and-rendering.md)
- [TabView Component](tab-view.md)
- [NavRail Component](nav-rail.md)