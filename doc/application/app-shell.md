**English** | [简体中文](../../doc.zh-cn/application/app-shell.md)

# App Shell & Application Assembly

This document introduces the AppShell application shell component in the Fangjia Qt6 C++ framework, including the assembly and interaction coordination mechanisms for navigation, TopBar, and content areas.

## AppShell Overview

### Application Shell Architecture

`AppShell` is the top-level layout container of the application, responsible for assembling and coordinating the main UI areas:

- **TopBar Area**: Window controls, theme switching, system integration
- **Navigation Area**: Side navigation bar with collapse/expand support
- **Content Area**: Main content display area
- **Status Area**: Optional status bar area

```cpp
auto shell = UI::appShell()
    ->topBar(createTopBar())           // Top bar
    ->nav(createNavigation())          // Side navigation
    ->content(createMainContent())     // Main content
    ->statusBar(createStatusBar());    // Status bar (optional)
```

### Layout Structure

AppShell uses Grid layout to manage various areas:

```
┌─────────────────────────────────────┐
│              TopBar                 │  ← Fixed height (usually 40-60px)
├──────────┬──────────────────────────┤
│          │                          │
│   Nav    │        Content           │  ← Flexible height
│  Rail    │                          │
│          │                          │
├──────────┴──────────────────────────┤
│             StatusBar               │  ← Fixed height (optional)
└─────────────────────────────────────┘
```

## AppShell API Design

### Basic Configuration

```cpp
class AppShell : public Widget {
public:
    // Set components for each area
    AppShell* topBar(WidgetPtr topBar);
    AppShell* nav(WidgetPtr navigation);
    AppShell* content(WidgetPtr content);
    AppShell* statusBar(WidgetPtr statusBar);
    
    // Layout configuration
    AppShell* topBarHeight(int height);         // TopBar height
    AppShell* navWidth(int collapsed, int expanded);  // Navigation bar width
    AppShell* statusBarHeight(int height);      // Status bar height
    
    // Interaction configuration
    AppShell* navCollapsible(bool collapsible); // Whether navigation bar is collapsible
    AppShell* navInitialState(bool expanded);   // Navigation bar initial state
    
protected:
    WidgetPtr build() override;
};
```

### Responsive Layout

AppShell supports responsive layout that automatically adjusts based on window size:

```cpp
auto createResponsiveShell() {
    return UI::bindingHost([this]() -> WidgetPtr {
        auto windowSize = getWindowSize();
        
        // Determine navigation bar behavior based on window width
        bool autoCollapse = windowSize.width() < 1200;
        bool navExpanded = !autoCollapse && m_navExpanded;
        
        return UI::appShell()
            ->topBar(createTopBar())
            ->nav(createNavigation()
                ->expanded(navExpanded)
                ->autoCollapse(autoCollapse)
            )
            ->content(createContent())
            ->navCollapsible(true)
            ->navInitialState(navExpanded);
    });
}
```

## Navigation System Integration

### NavRail Integration

```cpp
auto createNavigation() {
    return UI::navRail()
        ->dataProvider(&m_navDataProvider)      // Bind navigation data
        ->widths(60, 240)                       // Collapsed 60px, expanded 240px
        ->iconSize(24)                          // Icon size
        ->itemHeight(56)                        // Navigation item height
        ->onItemSelected([this](int index) {   // Selection callback
            navigateToPage(index);
        })
        ->onToggleExpand([this](bool expanded) { // Expand state change
            m_navExpanded = expanded;
            requestShellRebuild();
        });
}
```

### Navigation Data Provider

```cpp
class AppNavDataProvider : public INavDataProvider {
private:
    struct NavItem {
        QString icon;
        QString label;
        QString route;
        bool enabled = true;
    };
    
    std::vector<NavItem> m_items;
    int m_selectedIndex = 0;
    
public:
    int itemCount() const override { return m_items.size(); }
    
    QString itemIcon(int index) const override {
        return m_items[index].icon;
    }
    
    QString itemLabel(int index) const override {
        return m_items[index].label;
    }
    
    bool itemEnabled(int index) const override {
        return m_items[index].enabled;
    }
    
    int selectedIndex() const override { return m_selectedIndex; }
    
    // Add navigation item
    void addItem(const QString& icon, const QString& label, const QString& route) {
        m_items.push_back({icon, label, route, true});
        notifyDataChanged();
    }
    
    // Set selected item
    void setSelectedIndex(int index) {
        if (m_selectedIndex != index && index >= 0 && index < m_items.size()) {
            m_selectedIndex = index;
            notifySelectionChanged(index);
        }
    }
};
```

## TopBar System Integration

### TopBar Configuration & Interaction

```cpp
auto createTopBar() {
    return UI::topBar()
        ->followSystem(m_themeManager->followSystem(), m_animateThemeChange)
        ->cornerRadius(8.0f)
        ->svgTheme(":/icons/theme/sun.svg", ":/icons/theme/moon.svg")
        ->svgFollow(":/icons/theme/auto_on.svg", ":/icons/theme/auto_off.svg")
        ->svgSystem(":/icons/window/minimize.svg", 
                   ":/icons/window/maximize.svg", 
                   ":/icons/window/close.svg")
        ->onThemeToggle([this]() {
            toggleTheme();
        })
        ->onFollowToggle([this]() {
            toggleFollowSystem();
        })
        ->onMinimize([this]() {
            minimizeWindow();
        })
        ->onMaximize([this]() {
            toggleMaximizeWindow();
        })
        ->onClose([this]() {
            closeApplication();
        });
}
```

## Content Area Management

### Page Routing System

```cpp
class PageHost : public IUiContent {
private:
    std::unordered_map<QString, std::unique_ptr<IPage>> m_pages;
    QString m_currentPageRoute;
    IPage* m_currentPage = nullptr;
    
public:
    // Register page
    void registerPage(const QString& route, std::unique_ptr<IPage> page) {
        m_pages[route] = std::move(page);
    }
    
    // Navigate to page
    void navigateTo(const QString& route) {
        auto it = m_pages.find(route);
        if (it != m_pages.end() && m_currentPageRoute != route) {
            // Clean up current page
            if (m_currentPage) {
                m_currentPage->onDeactivated();
            }
            
            // Activate new page
            m_currentPageRoute = route;
            m_currentPage = it->second.get();
            m_currentPage->onActivated();
            
            // Notify route change
            notifyRouteChanged(route);
        }
    }
    
    // IUiContent implementation
    WidgetPtr createContent() override {
        if (m_currentPage) {
            return m_currentPage->createContent();
        }
        return UI::label()->text("Page not found");
    }
    
    void updateLayout(const QSize& size) override {
        if (m_currentPage) {
            m_currentPage->updateLayout(size);
        }
    }
};
```

## State Management & Synchronization

### Application State Coordination

```cpp
class AppShellViewModel : public INotifyPropertyChanged {
private:
    ThemeManager* m_themeManager;
    NavigationModel* m_navigationModel;
    PageHost* m_pageHost;
    bool m_navExpanded = true;
    
public:
    // Theme state
    bool isDarkMode() const {
        notifyAccess("isDarkMode");
        return m_themeManager->isDarkMode();
    }
    
    bool followSystem() const {
        notifyAccess("followSystem");
        return m_themeManager->followSystem();
    }
    
    // Navigation state
    bool navExpanded() const {
        notifyAccess("navExpanded");
        return m_navExpanded;
    }
    
    QString currentRoute() const {
        notifyAccess("currentRoute");
        return m_pageHost->currentRoute();
    }
    
    // State change methods
    void toggleTheme() {
        m_themeManager->setDarkMode(!m_themeManager->isDarkMode());
    }
    
    void toggleFollowSystem() {
        m_themeManager->setFollowSystem(!m_themeManager->followSystem());
    }
    
    void setNavExpanded(bool expanded) {
        if (m_navExpanded != expanded) {
            m_navExpanded = expanded;
            notifyChanged("navExpanded");
        }
    }
    
    void navigateToRoute(const QString& route) {
        m_pageHost->navigateTo(route);
        // currentRoute change will be notified through PageHost
    }
};
```

## Common Usage Patterns

### Simple Application Shell

```cpp
auto createBasicShell() {
    return UI::appShell()
        ->topBar(UI::topBar()->followSystem(true))
        ->nav(UI::navRail()->widths(60, 200))
        ->content(UI::label()->text("Main content area"));
}
```

### Complete Shell with Status Bar

```cpp
auto createFullShell() {
    return UI::appShell()
        ->topBar(createCustomTopBar())
        ->nav(createCustomNavigation())
        ->content(createPageContent())
        ->statusBar(createStatusBar())
        ->topBarHeight(50)
        ->navWidth(64, 240)
        ->statusBarHeight(30);
}
```

### Responsive Shell

```cpp
auto createResponsiveShell() {
    return UI::bindingHost([this]() -> WidgetPtr {
        auto windowSize = getCurrentWindowSize();
        bool isCompact = windowSize.width() < 768;
        
        auto shell = UI::appShell();
        
        if (isCompact) {
            // Compact layout: hide navigation bar, use bottom navigation
            shell->nav(nullptr)
                 ->bottomNav(createBottomNavigation());
        } else {
            // Standard layout: show side navigation bar
            shell->nav(createSideNavigation())
                 ->bottomNav(nullptr);
        }
        
        return shell->topBar(createTopBar())
                   ->content(createMainContent());
    });
}
```

## Related Documentation

- [Presentation Architecture Overview](../presentation/architecture.md) - AppShell's position in the overall architecture
- [TopBar Component](../presentation/components/top-bar.md) - Specific TopBar component configuration
- [Binding & Reactive Rebuild](../presentation/binding.md) - Reactive mechanisms in AppShell