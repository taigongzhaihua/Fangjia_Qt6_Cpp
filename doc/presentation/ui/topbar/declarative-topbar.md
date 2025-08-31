**English** | [简体中文](../../../../doc.zh-cn/presentation/ui/topbar/declarative-topbar.md)

# Declarative TopBar & NavRail Components

This document introduces the new declarative UI components NavRail and TopBar, which provide first-class declarative APIs without requiring `UI::wrap()` wrapping of runtime components.

## Overview

Declarative components maintain consistent behavior with existing `Ui::NavRail` and `UiTopBar` components while providing modern, fluent API configuration interfaces.

### Key Advantages

- **Fluent API**: Chainable configuration interface
- **Type Safety**: Compile-time configuration validation  
- **Performance**: Optimized for reactive rebuilds
- **Consistency**: Unified API patterns across all declarative components
- **Extensibility**: Easy to add new configuration options

## Declarative TopBar

### Basic Usage

```cpp
auto topBar = UI::topBar()
    ->followSystem(m_themeManager->followSystem())
    ->cornerRadius(8.0f)
    ->onThemeToggle([this]() { toggleTheme(); })
    ->onFollowToggle([this]() { toggleFollowSystem(); });
```

### Complete API Reference

```cpp
namespace UI {
    class TopBarBuilder {
    public:
        // Theme configuration
        TopBarBuilder* followSystem(bool follow, bool animate = false);
        TopBarBuilder* cornerRadius(float radius);
        
        // Icon configuration
        TopBarBuilder* svgTheme(const QString& darkIcon, const QString& lightIcon);
        TopBarBuilder* svgFollow(const QString& enabledIcon, const QString& disabledIcon);
        TopBarBuilder* svgSystem(const QString& minimizeIcon, 
                               const QString& maximizeIcon, 
                               const QString& closeIcon);
        
        // Visual styling
        TopBarBuilder* backgroundColor(const QColor& color);
        TopBarBuilder* buttonHoverColor(const QColor& color);
        TopBarBuilder* buttonPressedColor(const QColor& color);
        TopBarBuilder* closeButtonHoverColor(const QColor& color);
        
        // Layout configuration
        TopBarBuilder* height(int height);
        TopBarBuilder* buttonWidth(int width);
        TopBarBuilder* buttonSpacing(int spacing);
        TopBarBuilder* padding(int padding);
        
        // Animation settings
        TopBarBuilder* animationDuration(int milliseconds);
        TopBarBuilder* animationDelay(int milliseconds);
        TopBarBuilder* animationEasing(EasingCurve curve);
        
        // Event handlers
        TopBarBuilder* onThemeToggle(std::function<void()> callback);
        TopBarBuilder* onFollowToggle(std::function<void()> callback);
        TopBarBuilder* onMinimize(std::function<void()> callback);
        TopBarBuilder* onMaximize(std::function<void()> callback);
        TopBarBuilder* onClose(std::function<void()> callback);
        TopBarBuilder* onAnimationComplete(std::function<void()> callback);
        
        // Advanced configuration
        TopBarBuilder* hitTestMargin(int margin);
        TopBarBuilder* doubleClickAction(WindowAction action);
        TopBarBuilder* dragEnabled(bool enabled);
        
        // Build the component
        std::unique_ptr<UiTopBar> build();
        
        // Implicit conversion for convenience
        operator std::unique_ptr<UiTopBar>() { return build(); }
    };
    
    // Factory function
    TopBarBuilder* topBar();
}
```

### Configuration Examples

#### Theme Integration

```cpp
auto createThemedTopBar() {
    return UI::topBar()
        ->followSystem(m_themeManager->followSystem(), true)  // Enable animations
        ->svgTheme(":/icons/theme/sun.svg", ":/icons/theme/moon.svg")
        ->svgFollow(":/icons/theme/auto_on.svg", ":/icons/theme/auto_off.svg")
        ->cornerRadius(m_themeManager->cornerRadius())
        ->backgroundColor(m_themeManager->topBarColor())
        ->buttonHoverColor(m_themeManager->buttonHoverColor())
        ->onThemeToggle([this]() {
            m_themeManager->toggleTheme();
        })
        ->onFollowToggle([this]() {
            m_themeManager->toggleFollowSystem();
        });
}
```

#### Custom Styling

```cpp
auto createCustomStyledTopBar() {
    return UI::topBar()
        ->height(48)                           // Taller than default
        ->buttonWidth(52)                      // Wider buttons
        ->buttonSpacing(4)                     // Spacing between buttons
        ->cornerRadius(12.0f)                  // More rounded corners
        ->backgroundColor(QColor(30, 30, 30))  // Dark background
        ->buttonHoverColor(QColor(60, 60, 60)) // Hover state
        ->closeButtonHoverColor(QColor(196, 43, 28))  // Red close hover
        ->padding(8);                          // Internal padding
}
```

#### Advanced Animation Control

```cpp
auto createAnimatedTopBar() {
    return UI::topBar()
        ->followSystem(false, true)            // Start disabled, animations enabled
        ->animationDuration(200)               // Slower transitions
        ->animationDelay(50)                   // Longer delay between phases
        ->animationEasing(EasingCurve::OutBack) // Bouncy animation
        ->onAnimationComplete([this]() {
            // Custom logic when animation finishes
            onTopBarAnimationComplete();
        })
        ->onFollowToggle([this]() {
            // Handle follow toggle with custom logic
            handleFollowToggle();
        });
}
```

## Declarative NavRail

### Basic Usage

```cpp
auto navRail = UI::navRail()
    ->dataProvider(&m_navDataProvider)
    ->widths(60, 240)                      // Collapsed/expanded widths
    ->expanded(m_navigationExpanded)
    ->onItemSelected([this](int index) {
        navigateToPage(index);
    })
    ->onToggleExpand([this](bool expanded) {
        m_navigationExpanded = expanded;
        requestRebuild();
    });
```

### Complete API Reference

```cpp
namespace UI {
    class NavRailBuilder {
    public:
        // Data binding
        NavRailBuilder* dataProvider(INavDataProvider* provider);
        NavRailBuilder* items(const std::vector<NavItem>& items);  // Alternative to data provider
        
        // Layout configuration
        NavRailBuilder* widths(int collapsed, int expanded);
        NavRailBuilder* expanded(bool expanded);
        NavRailBuilder* collapsible(bool collapsible);
        NavRailBuilder* autoCollapse(bool autoCollapse);
        NavRailBuilder* autoCollapseWidth(int width);
        
        // Item configuration
        NavRailBuilder* itemHeight(int height);
        NavRailBuilder* iconSize(int size);
        NavRailBuilder* textSize(int size);
        NavRailBuilder* itemPadding(int padding);
        NavRailBuilder* itemSpacing(int spacing);
        
        // Visual styling
        NavRailBuilder* backgroundColor(const QColor& color);
        NavRailBuilder* itemBackgroundColor(const QColor& color);
        NavRailBuilder* selectedItemColor(const QColor& color);
        NavRailBuilder* hoverItemColor(const QColor& color);
        NavRailBuilder* textColor(const QColor& color);
        NavRailBuilder* selectedTextColor(const QColor& color);
        NavRailBuilder* iconColor(const QColor& color);
        NavRailBuilder* selectedIconColor(const QColor& color);
        
        // Border and effects
        NavRailBuilder* borderWidth(int width);
        NavRailBuilder* borderColor(const QColor& color);
        NavRailBuilder* cornerRadius(float radius);
        NavRailBuilder* elevation(int elevation);
        
        // Animation settings
        NavRailBuilder* expandAnimationDuration(int milliseconds);
        NavRailBuilder* selectionAnimationDuration(int milliseconds);
        NavRailBuilder* animationEasing(EasingCurve curve);
        
        // Event handlers
        NavRailBuilder* onItemSelected(std::function<void(int)> callback);
        NavRailBuilder* onItemHover(std::function<void(int)> callback);
        NavRailBuilder* onToggleExpand(std::function<void(bool)> callback);
        NavRailBuilder* onExpandAnimationComplete(std::function<void()> callback);
        
        // Accessibility
        NavRailBuilder* accessibilityLabel(const QString& label);
        NavRailBuilder* keyboardNavigationEnabled(bool enabled);
        
        // Build the component
        std::unique_ptr<UiNavRail> build();
        
        // Implicit conversion for convenience
        operator std::unique_ptr<UiNavRail>() { return build(); }
    };
    
    // Factory function
    NavRailBuilder* navRail();
}
```

### Configuration Examples

#### Responsive Navigation

```cpp
auto createResponsiveNavRail() {
    return UI::bindingHost([this]() -> WidgetPtr {
        auto windowSize = getCurrentWindowSize();
        bool autoCollapse = windowSize.width() < 1024;
        bool expanded = !autoCollapse && m_navExpanded;
        
        return UI::navRail()
            ->dataProvider(&m_navDataProvider)
            ->widths(64, 256)
            ->expanded(expanded)
            ->collapsible(!autoCollapse)        // Only allow manual collapse on larger screens
            ->autoCollapse(autoCollapse)
            ->autoCollapseWidth(1024)
            ->onItemSelected([this](int index) {
                handleNavigation(index);
            })
            ->onToggleExpand([this](bool expanded) {
                if (!isAutoCollapsed()) {
                    m_navExpanded = expanded;
                    requestRebuild();
                }
            });
    });
}
```

#### Styled Navigation

```cpp
auto createStyledNavRail() {
    return UI::navRail()
        ->dataProvider(&m_navDataProvider)
        ->widths(72, 280)
        ->itemHeight(64)                       // Taller items
        ->iconSize(28)                         // Larger icons
        ->itemPadding(16)                      // More padding
        ->backgroundColor(QColor(248, 249, 250))  // Light background
        ->selectedItemColor(QColor(59, 130, 246)) // Blue selection
        ->hoverItemColor(QColor(243, 244, 246))   // Light gray hover
        ->cornerRadius(8.0f)
        ->elevation(2)                         // Subtle shadow
        ->expandAnimationDuration(250)         // Smooth expand animation
        ->selectionAnimationDuration(150);    // Quick selection feedback
}
```

#### Custom Navigation Items

```cpp
auto createCustomNavRail() {
    std::vector<NavItem> customItems = {
        {":/icons/nav/dashboard.svg", "Dashboard", "dashboard", true},
        {":/icons/nav/projects.svg", "Projects", "projects", true},
        {":/icons/nav/team.svg", "Team", "team", true},
        {":/icons/nav/settings.svg", "Settings", "settings", true}
    };
    
    return UI::navRail()
        ->items(customItems)                   // Direct item configuration
        ->widths(60, 220)
        ->expanded(true)
        ->iconSize(24)
        ->textSize(14)
        ->onItemSelected([this](int index) {
            auto route = getRouteForIndex(index);
            navigateToRoute(route);
        })
        ->keyboardNavigationEnabled(true)     // Enable keyboard navigation
        ->accessibilityLabel("Main Navigation");
}
```

## Integration Patterns

### Combined App Shell

```cpp
auto createAppShell() {
    return UI::appShell()
        ->topBar(UI::topBar()
            ->followSystem(m_themeManager->followSystem(), true)
            ->svgTheme(":/icons/theme/sun.svg", ":/icons/theme/moon.svg")
            ->svgFollow(":/icons/theme/auto_on.svg", ":/icons/theme/auto_off.svg")
            ->onThemeToggle([this]() { m_themeManager->toggleTheme(); })
            ->onFollowToggle([this]() { m_themeManager->toggleFollowSystem(); })
        )
        ->nav(UI::navRail()
            ->dataProvider(&m_navDataProvider)
            ->widths(64, 240)
            ->expanded(m_navExpanded)
            ->onItemSelected([this](int index) { navigateToPage(index); })
            ->onToggleExpand([this](bool expanded) {
                m_navExpanded = expanded;
                requestRebuild();
            })
        )
        ->content(createPageContent());
}
```

### Reactive State Binding

```cpp
auto createReactiveTopBar() {
    return UI::bindingHost([this]() -> WidgetPtr {
        auto themeManager = getThemeManager();
        
        return UI::topBar()
            ->followSystem(themeManager->followSystem(), true)
            ->backgroundColor(themeManager->topBarColor())
            ->buttonHoverColor(themeManager->buttonHoverColor())
            ->cornerRadius(themeManager->cornerRadius())
            ->onThemeToggle([themeManager]() {
                themeManager->toggleTheme();
            })
            ->onFollowToggle([themeManager]() {
                themeManager->toggleFollowSystem();
            });
    });
}
```

### Performance Optimization

```cpp
class DeclarativeComponentCache {
public:
    std::shared_ptr<UiTopBar> getCachedTopBar(const TopBarConfig& config) {
        auto key = generateConfigHash(config);
        auto it = m_topBarCache.find(key);
        
        if (it != m_topBarCache.end()) {
            return it->second;
        }
        
        auto topBar = buildTopBarFromConfig(config);
        m_topBarCache[key] = topBar;
        return topBar;
    }
    
private:
    std::unordered_map<size_t, std::shared_ptr<UiTopBar>> m_topBarCache;
    std::unordered_map<size_t, std::shared_ptr<UiNavRail>> m_navRailCache;
    
    size_t generateConfigHash(const TopBarConfig& config) {
        // Generate hash based on configuration parameters
        return std::hash<TopBarConfig>{}(config);
    }
    
    std::shared_ptr<UiTopBar> buildTopBarFromConfig(const TopBarConfig& config) {
        return UI::topBar()
            ->followSystem(config.followSystem, config.animate)
            ->cornerRadius(config.cornerRadius)
            ->backgroundColor(config.backgroundColor)
            // ... apply all configuration options
            .build();
    }
};
```

## Migration from Runtime Components

### Before (Runtime Component)

```cpp
// Old approach using runtime wrapping
auto oldTopBar = std::make_unique<UiTopBar>();
oldTopBar->setFollowSystem(true);
oldTopBar->setCornerRadius(8.0f);
oldTopBar->setOnThemeToggleCallback([this]() { toggleTheme(); });

auto wrappedTopBar = UI::wrap(std::move(oldTopBar));
```

### After (Declarative Component)

```cpp
// New declarative approach
auto newTopBar = UI::topBar()
    ->followSystem(true)
    ->cornerRadius(8.0f)
    ->onThemeToggle([this]() { toggleTheme(); });
```

### Migration Utility

```cpp
class ComponentMigrator {
public:
    static WidgetPtr migrateTopBar(const UiTopBar& oldTopBar) {
        return UI::topBar()
            ->followSystem(oldTopBar.followSystem())
            ->cornerRadius(oldTopBar.cornerRadius())
            ->backgroundColor(oldTopBar.backgroundColor())
            ->onThemeToggle(oldTopBar.themeToggleCallback())
            ->onFollowToggle(oldTopBar.followToggleCallback());
    }
    
    static WidgetPtr migrateNavRail(const UiNavRail& oldNavRail) {
        return UI::navRail()
            ->dataProvider(oldNavRail.dataProvider())
            ->widths(oldNavRail.collapsedWidth(), oldNavRail.expandedWidth())
            ->expanded(oldNavRail.isExpanded())
            ->onItemSelected(oldNavRail.itemSelectedCallback())
            ->onToggleExpand(oldNavRail.toggleExpandCallback());
    }
};
```

## Related Documentation

- [TopBar Follow System Animation](animation.md) - Detailed animation behavior
- [TopBar Component](../../../components/top-bar.md) - Core TopBar functionality  
- [NavRail Component](../../../components/nav-rail.md) - Core NavRail functionality
- [App Shell & Application Assembly](../../../application/app-shell.md) - Integration with app shell