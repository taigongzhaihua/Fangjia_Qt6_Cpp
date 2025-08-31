# UI Framework Overview

## Design Philosophy

The Fangjia UI framework follows a component-based architecture with reactive data binding and declarative APIs. It emphasizes performance, maintainability, and developer experience while providing native-feeling desktop interactions.

## Core Architecture

### Component Hierarchy

```
UiRoot (Application root)
├── UiPage (Screen-level container)
│   ├── UiPanel (Layout container)
│   │   ├── UiGrid (Grid layout)
│   │   ├── UiContainer (Free-form container) 
│   │   └── Widgets (Interactive components)
│   └── Navigation (NavRail, TabView)
└── TopBar (Window controls)
```

### Component Lifecycle

All UI components implement the `IUiComponent` interface with standardized lifecycle methods:

```cpp
class IUiComponent {
public:
    virtual ~IUiComponent() = default;

    // Lifecycle methods
    virtual void onThemeChanged(bool isDark) {}
    virtual void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float dpr) {}
    virtual void updateLayout(const QSize& containerSize) {}
    
    // Event handling
    virtual bool onMousePress(const QPoint& pos) { return false; }
    virtual bool onMouseMove(const QPoint& pos) { return false; }
    virtual bool onMouseRelease(const QPoint& pos) { return false; }
    virtual bool onWheel(const QPointF& delta) { return false; }
    virtual bool onKeyPress(QKeyEvent* event) { return false; }
    
    // Rendering
    virtual void append(Render::FrameData& frameData) const = 0;
    virtual bool tick() { return false; }  // Returns true if animation continues
    
    // Properties
    virtual QRect bounds() const = 0;
    virtual bool isVisible() const { return true; }
    virtual bool isEnabled() const { return true; }
};
```

### Base Container Classes

#### UiRoot - Application Root Container

```cpp
class UiRoot : public IUiComponent {
public:
    // Child management
    void setContent(std::unique_ptr<IUiComponent> content);
    void setTopBar(std::unique_ptr<IUiComponent> topBar);
    
    // Event distribution
    bool distributeMouseEvent(const QPoint& pos, MouseEventType type);
    bool distributeKeyEvent(QKeyEvent* event);
    bool distributeWheelEvent(const QPointF& delta);
    
    // Theme propagation
    void propagateThemeChange(bool isDark);
    
    // Layout management
    void setViewport(const QSize& viewport);
    
private:
    std::unique_ptr<IUiComponent> m_content;
    std::unique_ptr<IUiComponent> m_topBar;
    QSize m_viewport;
    bool m_isDark = false;
};
```

#### UiPage - Screen Container

```cpp
class UiPage : public IUiComponent {
public:
    // Content areas
    void setNavigation(std::unique_ptr<IUiComponent> nav);
    void setMainContent(std::unique_ptr<IUiComponent> content);
    void setSidebar(std::unique_ptr<IUiComponent> sidebar);
    
    // Layout configuration
    void setNavigationWidth(int width);
    void setSidebarWidth(int width);
    void setContentMargins(const QMargins& margins);
    
private:
    std::unique_ptr<IUiComponent> m_navigation;
    std::unique_ptr<IUiComponent> m_content;
    std::unique_ptr<IUiComponent> m_sidebar;
    int m_navWidth = 200;
    int m_sidebarWidth = 300;
    QMargins m_contentMargins;
};
```

#### UiPanel - Layout Container

```cpp
class UiPanel : public IUiComponent {
public:
    enum class Layout {
        None,      // Manual positioning
        Vertical,  // Vertical stack
        Horizontal,// Horizontal stack
        Grid       // Grid layout
    };
    
    // Child management
    void addChild(std::unique_ptr<IUiComponent> child);
    void removeChild(IUiComponent* child);
    void clear();
    
    // Layout configuration
    void setLayout(Layout layout);
    void setSpacing(int spacing);
    void setPadding(const QMargins& padding);
    void setAlignment(Qt::Alignment alignment);
    
private:
    std::vector<std::unique_ptr<IUiComponent>> m_children;
    Layout m_layout = Layout::None;
    int m_spacing = 8;
    QMargins m_padding;
};
```

## Widget Components

### Interactive Widgets

#### UiButton

```cpp
class UiButton : public IUiComponent {
public:
    // Content
    void setText(const QString& text);
    void setIcon(const QString& iconPath);
    void setIconSize(const QSize& size);
    
    // Styling
    void setBackgroundColor(const QColor& color);
    void setTextColor(const QColor& color);
    void setCornerRadius(float radius);
    void setBorderWidth(float width);
    void setBorderColor(const QColor& color);
    
    // Interaction
    void setEnabled(bool enabled);
    void setCheckable(bool checkable);
    void setChecked(bool checked);
    
    // Events
    std::function<void()> onClick;
    std::function<void(bool)> onToggle;
    
private:
    QString m_text;
    QString m_iconPath;
    QSize m_iconSize{16, 16};
    QColor m_backgroundColor;
    QColor m_textColor;
    bool m_isPressed = false;
    bool m_isHovered = false;
    bool m_isEnabled = true;
};
```

#### UiScrollView

```cpp
class UiScrollView : public IUiComponent {
public:
    // Content management
    void setContent(std::unique_ptr<IUiComponent> content);
    
    // Scroll configuration
    void setScrollPolicy(Qt::ScrollBarPolicy horizontal, Qt::ScrollBarPolicy vertical);
    void setScrollSpeed(float speed);
    void setScrollBounds(const QRect& bounds);
    
    // Scroll control
    void scrollTo(const QPoint& position, bool animated = true);
    void scrollBy(const QPoint& delta, bool animated = true);
    QPoint scrollPosition() const;
    
    // Scroll bar styling
    void setScrollBarWidth(int width);
    void setScrollBarColor(const QColor& color);
    void setScrollBarRadius(float radius);
    
private:
    std::unique_ptr<IUiComponent> m_content;
    QPoint m_scrollPosition;
    QRect m_contentBounds;
    QRect m_visibleBounds;
    bool m_isScrolling = false;
    QPropertyAnimation* m_scrollAnimation;
};
```

### Navigation Components

#### UiNavRail

```cpp
class UiNavRail : public IUiComponent {
public:
    struct NavItem {
        QString id;
        QString title;
        QString iconPath;
        bool isEnabled = true;
        bool isVisible = true;
    };
    
    // Item management
    void addItem(const NavItem& item);
    void removeItem(const QString& id);
    void setActiveItem(const QString& id);
    QString activeItem() const;
    
    // Styling
    void setWidth(int width);
    void setItemHeight(int height);
    void setItemSpacing(int spacing);
    void setIndicatorColor(const QColor& color);
    void setIndicatorWidth(int width);
    
    // Animation
    void setAnimationDuration(int ms);
    void setAnimationEasing(QEasingCurve::Type easing);
    
    // Events
    std::function<void(const QString&)> onItemClick;
    std::function<void(const QString&)> onActiveChanged;
    
private:
    std::vector<NavItem> m_items;
    QString m_activeItemId;
    int m_width = 200;
    int m_itemHeight = 48;
    int m_indicatorPosition = 0;
    QPropertyAnimation* m_indicatorAnimation;
};
```

#### UiTabView

```cpp
class UiTabView : public IUiComponent {
public:
    struct Tab {
        QString id;
        QString title;
        QString iconPath;
        std::unique_ptr<IUiComponent> content;
        bool isClosable = true;
    };
    
    // Tab management
    void addTab(const Tab& tab);
    void removeTab(const QString& id);
    void setActiveTab(const QString& id);
    QString activeTab() const;
    
    // Tab bar configuration
    void setTabHeight(int height);
    void setTabMinWidth(int minWidth);
    void setTabMaxWidth(int maxWidth);
    void setTabSpacing(int spacing);
    
    // Events
    std::function<void(const QString&)> onTabClick;
    std::function<void(const QString&)> onTabClose;
    std::function<void(const QString&)> onActiveChanged;
    
private:
    std::vector<Tab> m_tabs;
    QString m_activeTabId;
    int m_tabBarHeight = 40;
    int m_tabMinWidth = 100;
    int m_tabMaxWidth = 200;
};
```

## Layout System

### Grid Layout

```cpp
class UiGrid : public IUiComponent {
public:
    struct GridItem {
        std::unique_ptr<IUiComponent> component;
        int row = 0;
        int column = 0;
        int rowSpan = 1;
        int columnSpan = 1;
        Qt::Alignment alignment = Qt::AlignCenter;
    };
    
    // Grid configuration
    void setRowCount(int rows);
    void setColumnCount(int columns);
    void setRowHeight(int row, int height);
    void setColumnWidth(int column, int width);
    void setSpacing(int spacing);
    
    // Item management
    void addItem(GridItem item);
    void removeItem(int row, int column);
    void clear();
    
private:
    std::vector<GridItem> m_items;
    std::vector<int> m_rowHeights;
    std::vector<int> m_columnWidths;
    int m_rowCount = 0;
    int m_columnCount = 0;
    int m_spacing = 8;
};
```

### Container Layout

```cpp
class UiContainer : public IUiComponent {
public:
    struct ContainerItem {
        std::unique_ptr<IUiComponent> component;
        QPoint position;
        QSize size;
        Qt::Alignment alignment = Qt::AlignTopLeft;
        bool autoSize = false;
    };
    
    // Item management
    void addItem(ContainerItem item);
    void removeItem(IUiComponent* component);
    void setItemPosition(IUiComponent* component, const QPoint& position);
    void setItemSize(IUiComponent* component, const QSize& size);
    
    // Container properties
    void setClipContents(bool clip);
    void setBackgroundColor(const QColor& color);
    void setBorder(const QColor& color, float width);
    void setCornerRadius(float radius);
    
private:
    std::vector<ContainerItem> m_items;
    bool m_clipContents = true;
    QColor m_backgroundColor;
    QColor m_borderColor;
    float m_borderWidth = 0.0f;
    float m_cornerRadius = 0.0f;
};
```

## Animation System

### Component Animation

```cpp
class AnimationController {
public:
    // Animation types
    enum class Type {
        FadeIn,
        FadeOut,
        SlideIn,
        SlideOut,
        Scale,
        Rotate
    };
    
    // Animation configuration
    void startAnimation(Type type, int duration, QEasingCurve::Type easing = QEasingCurve::OutCubic);
    void stopAnimation();
    bool isAnimating() const;
    
    // Animation properties
    void setOpacity(float opacity);
    void setTransform(const QTransform& transform);
    void setPosition(const QPointF& position);
    void setScale(float scale);
    
private:
    QPropertyAnimation* m_currentAnimation = nullptr;
    float m_opacity = 1.0f;
    QTransform m_transform;
    QPointF m_position;
    float m_scale = 1.0f;
};
```

### Transition Effects

```cpp
class TransitionManager {
public:
    // Page transitions
    void slideTransition(IUiComponent* from, IUiComponent* to, Direction direction);
    void fadeTransition(IUiComponent* from, IUiComponent* to);
    void scaleTransition(IUiComponent* from, IUiComponent* to);
    
    // Configuration
    void setTransitionDuration(int ms);
    void setTransitionEasing(QEasingCurve::Type easing);
    
    // Events
    std::function<void()> onTransitionStarted;
    std::function<void()> onTransitionFinished;
    
private:
    int m_duration = 300;
    QEasingCurve::Type m_easing = QEasingCurve::OutCubic;
    QParallelAnimationGroup* m_activeTransition = nullptr;
};
```

## Theme Integration

### Theme-Aware Components

```cpp
class ThemeAwareComponent : public IUiComponent {
protected:
    // Theme colors
    QColor backgroundColor() const;
    QColor foregroundColor() const;
    QColor accentColor() const;
    QColor borderColor() const;
    QColor textColor() const;
    
    // Theme-based styling
    virtual void applyTheme(bool isDark);
    virtual QColor getThemeColor(const QString& colorRole) const;
    
    // Theme change handling
    void onThemeChanged(bool isDark) override {
        applyTheme(isDark);
        requestRepaint();
    }
    
private:
    bool m_isDark = false;
    QHash<QString, QColor> m_lightColors;
    QHash<QString, QColor> m_darkColors;
};
```

## Performance Optimizations

### Render Optimization

- **Dirty Rectangle Tracking**: Only redraw changed regions
- **Component Culling**: Skip rendering for off-screen components
- **Batch Rendering**: Merge similar render commands
- **Resource Caching**: Cache expensive operations like text layout

### Memory Management

- **Component Pooling**: Reuse components when possible
- **Lazy Loading**: Create components only when needed
- **Weak References**: Avoid circular dependencies
- **Smart Cleanup**: Automatic resource cleanup

## Related Documentation

- [Layouts Guide](layouts.md)
- [Theme & Rendering](theme-and-rendering.md)
- [Data Binding](../binding.md)
- [TopBar Component](../components/top-bar.md)
- [NavRail Component](../components/nav-rail.md)