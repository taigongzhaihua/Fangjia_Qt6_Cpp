**English** | [简体中文](../../../doc.zh-cn/presentation/components/nav-rail.md)

# NavRail Component

## Overview

The NavRail component provides vertical navigation with smooth animations, data provider integration, and theme-aware styling. It supports icon/text combinations, active state indicators, and customizable animations synchronized with application state changes.

## Core Features

- **Vertical Navigation**: Icon-based or text+icon navigation items
- **Active Indicators**: Smooth animated indicators showing current selection
- **Data Provider Mode**: Integration with external data sources for dynamic content
- **Animation Synchronization**: Coordinated animations with other UI components
- **Theme Integration**: Automatic color adaptation for light/dark themes
- **Responsive Design**: Adaptive width and item sizing

## Basic Usage

### Simple Navigation Rail

```cpp
auto navRail = std::make_unique<UiNavRail>();

// Add navigation items
navRail->addItem({"home", "Home", ":/icons/home.svg"});
navRail->addItem({"data", "Data", ":/icons/database.svg"});
navRail->addItem({"settings", "Settings", ":/icons/settings.svg"});

// Set active item
navRail->setActiveItem("home");

// Configure appearance
navRail->setWidth(200);
navRail->setItemHeight(48);
navRail->setItemSpacing(4);
```

### Event Handling

```cpp
// Handle item selection
navRail->onItemClick = [](const QString& itemId) {
    qDebug() << "Selected item:" << itemId;
    // Handle navigation logic
};

// Handle active item changes (including programmatic changes)
navRail->onActiveChanged = [](const QString& itemId) {
    qDebug() << "Active item changed to:" << itemId;
    // Update application state
};
```

## DataProvider Integration

### Using External Data Sources

The NavRail can be driven by external data providers for dynamic content:

```cpp
class NavigationDataProvider {
public:
    struct NavItem {
        QString id;
        QString title;
        QString iconPath;
        bool isEnabled = true;
        bool isVisible = true;
        int sortOrder = 0;
        QVariantMap metadata;
    };
    
    virtual QVector<NavItem> getNavigationItems() const = 0;
    virtual QString getActiveItemId() const = 0;
    virtual void setActiveItemId(const QString& id) = 0;
    
signals:
    void itemsChanged();
    void activeItemChanged(const QString& id);
};

// Configure NavRail with data provider
navRail->setDataProvider(dataProvider);
navRail->setAutoUpdate(true); // Automatically update when data changes
```

### Dynamic Updates

```cpp
// Data provider can trigger updates
class DynamicNavigationProvider : public NavigationDataProvider {
public:
    void addItem(const NavItem& item) {
        m_items.append(item);
        emit itemsChanged();
    }
    
    void removeItem(const QString& id) {
        m_items.removeIf([id](const NavItem& item) { return item.id == id; });
        emit itemsChanged();
    }
    
    void updateItemVisibility(const QString& id, bool visible) {
        auto it = std::find_if(m_items.begin(), m_items.end(),
                              [id](const NavItem& item) { return item.id == id; });
        if (it != m_items.end()) {
            it->isVisible = visible;
            emit itemsChanged();
        }
    }
    
private:
    QVector<NavItem> m_items;
};
```

## Advanced Configuration

### Styling & Appearance

```cpp
class UiNavRail {
public:
    // Dimensions
    void setWidth(int width);
    void setItemHeight(int height);
    void setItemSpacing(int spacing);
    void setContentMargins(const QMargins& margins);
    
    // Indicator styling
    void setIndicatorColor(const QColor& color);
    void setIndicatorWidth(int width);
    void setIndicatorRadius(float radius);
    void setIndicatorMargin(int margin);
    
    // Item styling  
    void setItemTextColor(const QColor& color);
    void setItemActiveTextColor(const QColor& color);
    void setItemIconSize(const QSize& size);
    void setItemBackgroundColor(const QColor& color);
    void setItemActiveBackgroundColor(const QColor& color);
    void setItemCornerRadius(float radius);
    
    // Typography
    void setItemFont(const QFont& font);
    void setItemActiveFont(const QFont& font);
    
private:
    int m_width = 200;
    int m_itemHeight = 48;
    int m_itemSpacing = 4;
    QMargins m_contentMargins{8, 8, 8, 8};
};
```

### Color Palettes

```cpp
struct UiNavRail::Palette {
    // Background colors
    QColor backgroundColor;
    QColor itemBackground;
    QColor itemHoverBackground;
    QColor itemActiveBackground;
    
    // Text colors
    QColor textColor;
    QColor textHoverColor;
    QColor textActiveColor;
    QColor textDisabledColor;
    
    // Icon colors
    QColor iconColor;
    QColor iconHoverColor;
    QColor iconActiveColor;
    QColor iconDisabledColor;
    
    // Indicator
    QColor indicatorColor;
    QColor indicatorShadowColor;
    
    // Borders and dividers
    QColor borderColor;
    QColor separatorColor;
};

// Apply custom palette
UiNavRail::Palette customPalette;
customPalette.backgroundColor = QColor(45, 55, 70);
customPalette.textActiveColor = QColor(102, 179, 255);
customPalette.indicatorColor = QColor(102, 179, 255);
navRail->setPalette(customPalette);
```

## Animation System

### Indicator Animation

The NavRail features smooth indicator animations when the active item changes:

```cpp
class UiNavRail {
public:
    // Animation configuration
    void setAnimationDuration(int ms);
    void setAnimationEasing(QEasingCurve::Type easing);
    void setAnimationEnabled(bool enabled);
    
    // Animation synchronization
    void setAnimationSyncTarget(IUiComponent* target);
    void setAnimationDelay(int delayMs);
    
private:
    int m_animationDuration = 300;
    QEasingCurve::Type m_animationEasing = QEasingCurve::OutCubic;
    QPropertyAnimation* m_indicatorAnimation = nullptr;
    
    void animateIndicatorTo(int targetPosition);
};
```

### Animation Implementation

```cpp
void UiNavRail::setActiveItem(const QString& id) {
    if (m_activeItemId == id) return;
    
    // Find target item position
    int targetIndex = findItemIndex(id);
    if (targetIndex < 0) return;
    
    // Calculate target indicator position
    int targetPosition = calculateIndicatorPosition(targetIndex);
    
    // Animate indicator
    if (m_animationEnabled && m_indicatorAnimation) {
        m_indicatorAnimation->stop();
        m_indicatorAnimation->setDuration(m_animationDuration);
        m_indicatorAnimation->setStartValue(m_currentIndicatorPosition);
        m_indicatorAnimation->setEndValue(targetPosition);
        m_indicatorAnimation->setEasingCurve(m_animationEasing);
        m_indicatorAnimation->start();
    } else {
        m_currentIndicatorPosition = targetPosition;
    }
    
    // Update state
    m_activeItemId = id;
    emit activeItemChanged(id);
}
```

### Synchronized Animations

NavRail can synchronize with other components for coordinated UI transitions:

```cpp
// Sync with content area transitions
navRail->setAnimationSyncTarget(contentArea);
navRail->setAnimationDelay(50); // Slight delay for staggered effect

// Custom sync callback
navRail->onAnimationStart = [](const QString& fromId, const QString& toId) {
    // Coordinate with other UI elements
    contentArea->startTransition(fromId, toId);
    topBar->updateBreadcrumb(toId);
};
```

## Layout Integration

### Responsive Behavior

```cpp
class ResponsiveNavRail : public UiNavRail {
public:
    // Responsive configuration
    void setCollapsedWidth(int width);
    void setExpandedWidth(int width);
    void setAutoCollapse(bool enabled);
    void setCollapseBreakpoint(int width);
    
    // State management
    void setCollapsed(bool collapsed, bool animated = true);
    bool isCollapsed() const;
    
protected:
    void updateLayout(const QSize& containerSize) override {
        if (m_autoCollapse) {
            bool shouldCollapse = containerSize.width() < m_collapseBreakpoint;
            if (shouldCollapse != m_isCollapsed) {
                setCollapsed(shouldCollapse, true);
            }
        }
        UiNavRail::updateLayout(containerSize);
    }
    
private:
    int m_collapsedWidth = 60;
    int m_expandedWidth = 200;
    int m_collapseBreakpoint = 768;
    bool m_autoCollapse = false;
    bool m_isCollapsed = false;
};
```

### Container Integration

```cpp
// Integration with main layout
auto mainLayout = std::make_unique<UiPanel>();
mainLayout->setDirection(UiPanel::Direction::Horizontal);
mainLayout->setSpacing(0);

// Add navigation rail
mainLayout->addChild(std::move(navRail));

// Add content area with flex grow
auto contentArea = std::make_unique<UiContainer>();
mainLayout->addChild(std::move(contentArea));
mainLayout->setChildStretch(contentArea.get(), 1); // Take remaining space
```

## Theme Integration

### Automatic Theme Adaptation

```cpp
void UiNavRail::onThemeChanged(bool isDark) {
    // Update palette based on theme
    if (isDark) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
    
    // Update all item visuals
    updateItemAppearance();
    
    // Refresh cached resources
    invalidateIconCache();
    
    // Request visual update
    requestRepaint();
}

void UiNavRail::applyDarkTheme() {
    m_palette.backgroundColor = QColor(33, 37, 41);
    m_palette.textColor = QColor(248, 249, 250);
    m_palette.textActiveColor = QColor(102, 179, 255);
    m_palette.indicatorColor = QColor(102, 179, 255);
    m_palette.itemActiveBackground = QColor(102, 179, 255, 30);
}
```

### Custom Theme Support

```cpp
// Register custom theme variant
ThemeManager::instance().registerCustomPalette("navRail", customNavRailPalette);

// Apply custom theme
navRail->setUseCustomTheme(true);
navRail->setCustomThemeName("navRail");
```

## Performance Optimization

### Efficient Rendering

- **Batched Updates**: Multiple item changes are batched into single render update
- **Viewport Culling**: Only visible items are rendered
- **Icon Caching**: Icons are cached and reused across theme changes
- **Animation Optimization**: Hardware-accelerated animations when available

### Memory Management

```cpp
class UiNavRail {
private:
    // Efficient item storage
    std::vector<NavItem> m_items;
    
    // Cache frequently accessed data
    mutable QHash<QString, int> m_itemIndexCache;
    mutable QHash<int, QRect> m_itemBoundsCache;
    
    // Resource management
    void invalidateItemCache() {
        m_itemIndexCache.clear();
        m_itemBoundsCache.clear();
    }
    
    int findItemIndex(const QString& id) const {
        auto it = m_itemIndexCache.find(id);
        if (it != m_itemIndexCache.end()) {
            return it.value();
        }
        
        // Calculate and cache
        for (int i = 0; i < m_items.size(); ++i) {
            if (m_items[i].id == id) {
                m_itemIndexCache[id] = i;
                return i;
            }
        }
        return -1;
    }
};
```

## Related Documentation

- [UI Framework Overview](../ui-framework/overview.md)
- [Layout System](../ui-framework/layouts.md)
- [Theme System](../ui-framework/theme-and-rendering.md)
- [TopBar Component](top-bar.md)
- [TabView Component](tab-view.md)