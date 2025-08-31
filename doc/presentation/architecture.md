**English** | [简体中文](../../doc.zh-cn/presentation/architecture.md)

# Presentation Architecture & Declarative System Overview

This document introduces the core mechanisms of the Fangjia Qt6 C++ project's presentation layer architecture, including the responsibilities and collaboration of BindingHost, RebuildHost, and UiRoot, as well as the design philosophy of the declarative UI system.

## Core Component Overview

### UiRoot - Root Container & Event Coordinator

`UiRoot` is the root container of the entire UI system, responsible for:

- **Unified Event Dispatch**: Managing mouse events (press/move/release/wheel) and pointer capture
- **Layout Coordination**: Coordinating `updateLayout()` and `updateResourceContext()` calls for all top-level components
- **Render Coordination**: Collecting rendering commands from all components into `Render::FrameData`
- **Theme Propagation**: Distributing theme changes to the entire component tree via `propagateThemeChange(isDark)`

### BindingHost - Reactive Rebuild Container

`BindingHost` provides reactive UI construction mechanisms:

```cpp
m_shellHost = UI::bindingHost([this]() -> WidgetPtr {
    const bool follow = m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem;
    return appShell()
        ->nav(wrap(&m_nav))
        ->topBar(UI::topBar()->followSystem(follow, m_animateFollowChange))
        ->content([this]{ return wrap(m_pageHost.get()); });
});
```

**Responsibilities**:
- **Dependency Tracking**: Automatically detects external state accessed in build functions
- **Change Response**: Triggers rebuilds when dependent state changes
- **Lifecycle Management**: Manages Widget instances produced by rebuilds

### RebuildHost - Rebuild Order Management

`RebuildHost` ensures correct order during rebuilds, preventing visual flicker during theme switches:

```
requestRebuild() execution order:
1. Set viewport (for IUiContent/ILayoutable)
2. Call onThemeChanged(isDark)
3. Update resource context updateResourceContext(...)
4. Call updateLayout(...)
```

This order design ensures components complete theme adaptation and resource updates before receiving new layout dimensions.

## Declarative UI System

### Widget Base Class & Decorator Pattern

The declarative system centers around the `Widget` base class, supporting fluent configuration:

```cpp
auto decoratedPanel = UI::panel()
    ->padding(16)                        // Padding decorator
    ->margin(8, 12)                     // Margin decorator
    ->background(QColor(240, 240, 240)) // Background decorator
    ->border(QColor(200, 200, 200), 1.0f); // Border decorator
```

**Decorator Features**:
- **Composable**: Multiple decorators can be combined arbitrarily
- **Order-Independent**: Decorator application order doesn't affect final result
- **Type-Safe**: Compile-time checking of decorator-component compatibility

### Factory Functions & Type Deduction

The UI namespace provides convenient factory functions:

```cpp
using namespace UI;

// Container components
auto panel = panel();           // UiPanel container
auto grid = grid();             // UiGrid grid layout
auto scroll = scrollView();     // UiScrollView scroll container

// Navigation components  
auto nav = navRail();           // NavRail navigation bar
auto top = topBar();            // TopBar top bar

// Application components
auto shell = appShell();        // AppShell application shell
```

## Component Lifecycle

### IUiComponent Interface

All UI components implement the `IUiComponent` interface, called in the following order:

1. **`updateLayout(const QSize&)`** - Layout calculation based on window logical size
2. **`updateResourceContext(IconCache&, QOpenGLFunctions*, float dpr)`** - Update texture/GL context (DPR-related)
3. **`append(Render::FrameData&) const`** - Generate drawing commands
4. **`tick()`** - Advance animations, return whether redraw is still needed

### Theme Change Flow

Theme changes propagate through the following flow:

1. **Trigger Source**: User action or system theme change
2. **UiRoot Coordination**: Call `propagateThemeChange(isDark)`
3. **Component Response**: Each component's `onThemeChanged(bool)` is called
4. **Resource Update**: Components update color palettes, icon cache keys, etc.
5. **Re-render**: Next frame uses new theme resources

## Integration with MainOpenGlWindow

The main window class is responsible for:

- **OpenGL Context Management**: Initialize renderer and graphics resources
- **Event Bridging**: Forward Qt events to UiRoot
- **Theme Management**: Integrate ThemeManager, respond to system theme changes
- **Page Routing**: Manage navigation and switching between different pages

### Typical Integration Pattern

```cpp
class MainOpenGlWindow : public QOpenGLWidget {
private:
    std::unique_ptr<UiRoot> m_uiRoot;
    std::unique_ptr<UI::BindingHost> m_shellHost;
    
    void setupUI() {
        m_uiRoot = std::make_unique<UiRoot>();
        m_shellHost = UI::bindingHost([this]() {
            return createAppShell();  // Build application shell
        });
        m_uiRoot->setContent(m_shellHost.get());
    }
};
```

## Advantages & Design Goals

### Advantages of Declarative Programming

- **Readability**: Code structure intuitively reflects UI hierarchy
- **Maintainability**: State changes automatically trigger UI updates, reducing manual synchronization
- **Testability**: Component configuration is separated from business logic, facilitating unit testing
- **Performance Optimization**: Dependency tracking avoids unnecessary rebuilds

### Architecture Design Goals

- **Separation of Concerns**: Presentation layer focuses on UI rendering, business logic handled in domain layer
- **Extensibility**: New components can easily integrate into the existing system
- **Cross-Platform Consistency**: Abstract platform differences, provide unified programming interface
- **Performance-Oriented**: Batch rendering and state caching optimize performance

## Related Documentation

- [UI Framework Overview](ui-framework/overview.md) - Component architecture, lifecycle, containers, and widget systems
- [Binding & Reactive Rebuild](binding.md) - binding/observe/requestRebuild patterns and best practices
- [TopBar Component](components/top-bar.md) - Window controls, theme switching, system integration, and declarative API
- [Graphics & Rendering System](../infrastructure/gfx.md) - RenderData, IconCache, coordinate systems, and OpenGL rendering pipeline