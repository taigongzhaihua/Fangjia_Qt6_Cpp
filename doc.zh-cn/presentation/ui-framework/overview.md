[English](../../doc/presentation/ui-framework/overview.md) | **简体中文**

# UI 框架概览

## 设计理念

Fangjia UI 框架遵循基于组件的架构，具有响应式数据绑定和声明式 API。它强调性能、可维护性和开发者体验，同时提供原生桌面交互感受。

## 核心架构

### 组件层次结构

```
UiRoot (应用根节点)
├── UiPage (屏幕级容器)
│   ├── UiPanel (布局容器)
│   │   ├── UiGrid (网格布局)
│   │   ├── UiContainer (自由形式容器) 
│   │   └── Widgets (交互组件)
│   └── Navigation (NavRail、TabView)
└── TopBar (窗口控件)
```

### 组件生命周期

所有 UI 组件实现 `IUiComponent` 接口，具有标准化的生命周期方法：

```cpp
class IUiComponent {
public:
    virtual ~IUiComponent() = default;

    // 生命周期方法
    virtual void onThemeChanged(bool isDark) {}
    virtual void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float dpr) {}
    virtual void updateLayout(const QSize& containerSize) {}
    
    // 事件处理
    virtual bool onMousePress(const QPoint& pos) { return false; }
    virtual bool onMouseMove(const QPoint& pos) { return false; }
    virtual bool onMouseRelease(const QPoint& pos) { return false; }
    virtual bool onWheel(const QPointF& delta) { return false; }
    virtual bool onKeyPress(QKeyEvent* event) { return false; }
    
    // 渲染
    virtual void append(Render::FrameData& frameData) const = 0;
    virtual bool tick() { return false; }  // 如果动画继续则返回 true
    
    // 属性
    virtual QRect bounds() const = 0;
    virtual bool isVisible() const { return true; }
    virtual bool isEnabled() const { return true; }
};
```

### 基础容器类

#### UiRoot - 应用根容器

```cpp
class UiRoot : public IUiComponent {
public:
    // 子组件管理
    void setContent(std::unique_ptr<IUiComponent> content);
    void setTopBar(std::unique_ptr<IUiComponent> topBar);
    
    // 事件分发
    bool distributeMouseEvent(const QPoint& pos, MouseEventType type);
    bool distributeKeyEvent(QKeyEvent* event);
    bool distributeWheelEvent(const QPointF& delta);
    
    // 主题传播
    void propagateThemeChange(bool isDark);
    
    // 布局管理
    void setViewport(const QSize& viewport);
    
private:
    std::unique_ptr<IUiComponent> m_content;
    std::unique_ptr<IUiComponent> m_topBar;
    QSize m_viewport;
    bool m_isDark = false;
};
```

#### UiPage - 屏幕容器

```cpp
class UiPage : public IUiComponent {
public:
    // 内容区域
    void setNavigation(std::unique_ptr<IUiComponent> nav);
    void setMainContent(std::unique_ptr<IUiComponent> content);
    void setSidebar(std::unique_ptr<IUiComponent> sidebar);
    
    // 布局配置
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

#### UiPanel - 布局容器

```cpp
class UiPanel : public IUiComponent {
public:
    enum class Layout {
        None,      // 手动定位
        Vertical,  // 垂直堆叠
        Horizontal,// 水平堆叠
        Grid       // 网格布局
    };
    
    // 子组件管理
    void addChild(std::unique_ptr<IUiComponent> child);
    void removeChild(IUiComponent* child);
    void clear();
    
    // 布局配置
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

## 小部件组件

### 交互小部件

#### UiButton

```cpp
class UiButton : public IUiComponent {
public:
    // 内容
    void setText(const QString& text);
    void setIcon(const QString& iconPath);
    void setIconSize(const QSize& size);
    
    // 样式
    void setBackgroundColor(const QColor& color);
    void setTextColor(const QColor& color);
    void setCornerRadius(float radius);
    void setBorderWidth(float width);
    void setBorderColor(const QColor& color);
    
    // 交互
    void setEnabled(bool enabled);
    void setCheckable(bool checkable);
    void setChecked(bool checked);
    
    // 事件
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
    // 内容管理
    void setContent(std::unique_ptr<IUiComponent> content);
    
    // 滚动配置
    void setScrollPolicy(Qt::ScrollBarPolicy horizontal, Qt::ScrollBarPolicy vertical);
    void setScrollSpeed(float speed);
    void setScrollBounds(const QRect& bounds);
    
    // 滚动控制
    void scrollTo(const QPoint& position, bool animated = true);
    void scrollBy(const QPoint& delta, bool animated = true);
    QPoint scrollPosition() const;
    
    // 滚动条样式
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

### 导航组件

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
    
    // 项目管理
    void addItem(const NavItem& item);
    void removeItem(const QString& id);
    void setActiveItem(const QString& id);
    QString activeItem() const;
    
    // 样式
    void setWidth(int width);
    void setItemHeight(int height);
    void setItemSpacing(int spacing);
    void setIndicatorColor(const QColor& color);
    void setIndicatorWidth(int width);
    
    // 动画
    void setAnimationDuration(int ms);
    void setAnimationEasing(QEasingCurve::Type easing);
    
    // 事件
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
    
    // 选项卡管理
    void addTab(const Tab& tab);
    void removeTab(const QString& id);
    void setActiveTab(const QString& id);
    QString activeTab() const;
    
    // 选项卡栏配置
    void setTabHeight(int height);
    void setTabMinWidth(int minWidth);
    void setTabMaxWidth(int maxWidth);
    void setTabSpacing(int spacing);
    
    // 事件
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

## 布局系统

### 网格布局

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
    
    // 网格配置
    void setRowCount(int rows);
    void setColumnCount(int columns);
    void setRowHeight(int row, int height);
    void setColumnWidth(int column, int width);
    void setSpacing(int spacing);
    
    // 项目管理
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

### 容器布局

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
    
    // 项目管理
    void addItem(ContainerItem item);
    void removeItem(IUiComponent* component);
    void setItemPosition(IUiComponent* component, const QPoint& position);
    void setItemSize(IUiComponent* component, const QSize& size);
    
    // 容器属性
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

## 动画系统

### 组件动画

```cpp
class AnimationController {
public:
    // 动画类型
    enum class Type {
        FadeIn,
        FadeOut,
        SlideIn,
        SlideOut,
        Scale,
        Rotate
    };
    
    // 动画配置
    void startAnimation(Type type, int duration, QEasingCurve::Type easing = QEasingCurve::OutCubic);
    void stopAnimation();
    bool isAnimating() const;
    
    // 动画属性
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

### 过渡效果

```cpp
class TransitionManager {
public:
    // 页面过渡
    void slideTransition(IUiComponent* from, IUiComponent* to, Direction direction);
    void fadeTransition(IUiComponent* from, IUiComponent* to);
    void scaleTransition(IUiComponent* from, IUiComponent* to);
    
    // 配置
    void setTransitionDuration(int ms);
    void setTransitionEasing(QEasingCurve::Type easing);
    
    // 事件
    std::function<void()> onTransitionStarted;
    std::function<void()> onTransitionFinished;
    
private:
    int m_duration = 300;
    QEasingCurve::Type m_easing = QEasingCurve::OutCubic;
    QParallelAnimationGroup* m_activeTransition = nullptr;
};
```

## 主题集成

### 主题感知组件

```cpp
class ThemeAwareComponent : public IUiComponent {
protected:
    // 主题颜色
    QColor backgroundColor() const;
    QColor foregroundColor() const;
    QColor accentColor() const;
    QColor borderColor() const;
    QColor textColor() const;
    
    // 基于主题的样式
    virtual void applyTheme(bool isDark);
    virtual QColor getThemeColor(const QString& colorRole) const;
    
    // 主题变化处理
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

## 性能优化

### 渲染优化

- **脏矩形跟踪**: 仅重绘变化的区域
- **组件裁剪**: 跳过屏幕外组件的渲染
- **批量渲染**: 合并相似的渲染命令
- **资源缓存**: 缓存昂贵的操作，如文本布局

### 内存管理

- **组件池化**: 尽可能重用组件
- **延迟加载**: 仅在需要时创建组件
- **弱引用**: 避免循环依赖
- **智能清理**: 自动资源清理

## 相关文档

- [布局指南](layouts.md)
- [主题与渲染](theme-and-rendering.md)
- [数据绑定](../binding.md)
- [TopBar 组件](../components/top-bar.md)
- [NavRail 组件](../components/nav-rail.md)