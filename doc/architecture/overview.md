**English** | [简体中文](../../doc.zh-cn/architecture/overview.md)

# Architecture Overview

## System Design

Fangjia Qt6 C++ follows a layered architecture pattern with clear separation of concerns across multiple subsystems:

- **Rendering Subsystem**: Renderer/RenderData/IconCache for high-performance graphics
- **UI Framework**: UiRoot/UiPage/UiPanel/UiGrid and component system for layout and interaction
- **Business Views**: Domain-specific pages like Home/Data/Settings
- **Models & Configuration**: ThemeManager/AppConfig for theme management and persistence

## Rendering Pipeline

The rendering system is built on OpenGL with optimized command-based rendering:

- **Vertex Generation**: Rectangle-to-NDC vertex buffer transformation
- **Fragment Shading**: Rounded rectangle SDF rendering, texture sampling with tint
- **Clipping**: Logical pixel clipping → device pixel clipping rectangle → glScissor

## Event & Animation System

- **UiRoot**: Unified mouse/wheel event distribution and capture
- **Component Animation**: Internal component animation state (NavRail/UiTabView etc.)
- **Main Loop**: QTimer-driven ~60fps tick() calls from the main window

## Theme Propagation

- **ThemeManager**: Derives effective color scheme (follow system/force light/dark)
- **UiRoot**: Recursive onThemeChanged() propagation down the component tree

## Threading Model

- **OpenGL Context**: All OpenGL calls must be on the UI thread with valid context
- **Data Models**: Usually modified on UI thread; async operations require thread safety
- **Resource Management**: Icon/texture loading coordinated with UI thread

## Component Lifecycle

All UI components implement the IUiComponent interface with standardized lifecycle:

1. **Construction**: Component initialization and default state setup
2. **Theme Application**: onThemeChanged() called when theme changes
3. **Resource Context**: updateResourceContext() for icon cache and GL functions
4. **Layout Update**: updateLayout() for size and positioning
5. **Event Handling**: onMousePress/Move/Release for user interaction
6. **Rendering**: append() to add render commands to FrameData
7. **Animation**: tick() for time-based updates and animations

## Data Binding System

The framework provides reactive data binding through:

- **BindingHost**: Central binding management and change propagation
- **RebuildHost**: Declarative UI rebuilding with environment synchronization
- **Observer Pattern**: Components subscribe to model changes for automatic updates

For detailed information on specific subsystems, see:
- [UI Framework Overview](../presentation/ui-framework/overview.md)
- [Graphics & Rendering](../infrastructure/gfx.md)
- [Data Binding](../presentation/binding.md)