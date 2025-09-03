**English** | [简体中文](../doc.zh-cn/index.md)

# Fangjia Qt6 C++ Documentation

This documentation is organized by system to provide developers with comprehensive technical reference and development guides.

## Architecture & Design

### System Overview
- **[Architecture Overview](architecture/overview.md)** - System design, component lifecycle, data binding, and threading model

## Build & Development

### Platform-Specific Build Guides
- **[Windows Build Guide](build/windows.md)** - Visual Studio setup, Qt installation, and build configuration for Windows
- **[Linux Build Guide](build/linux.md)** - Package installation, compilation, and development setup for Linux distributions  
- **[macOS Build Guide](build/macos.md)** - Xcode setup, Qt installation, and universal binary builds for macOS

## Infrastructure Systems

### Graphics & Platform
- **[Graphics & Rendering System](infrastructure/gfx.md)** - RenderData, IconCache, coordinate systems, and OpenGL rendering pipeline
- **[Windows Platform Integration](infrastructure/platform-windows.md)** - Native window chrome, hit testing, DPI awareness, and Windows-specific features

## Data Management

### Data Layer Architecture
- **[Data Management Overview](data/overview.md)** - ThemeManager, AppConfig, ViewModel patterns, and reactive data binding

## Presentation Layer

### Architecture & Design
- **[Presentation Architecture](presentation/architecture.md)** - Declarative system overview, core components (UiRoot, BindingHost, RebuildHost), and architectural design principles

### UI Framework
- **[UI Framework Overview](presentation/ui-framework/overview.md)** - Component architecture, lifecycle, containers, and widget systems
- **[Layout System](presentation/ui-framework/layouts.md)** - UiPanel, UiGrid, UiContainer, size policies, and responsive design
- **[Theme & Rendering](presentation/ui-framework/theme-and-rendering.md)** - Theme management, color palettes, transitions, and styling customization

### UI Components
- **[TopBar Component](presentation/components/top-bar.md)** - Window controls, theme switching, system integration, and declarative API
- **[TopBar Animation System](presentation/components/top-bar-animation.md)** - Two-phase animation behavior, state machine, and performance optimization
- **[NavRail Component](presentation/components/nav-rail.md)** - Navigation rail with DataProvider integration, animations, and responsive behavior
- **[TabView Component](presentation/components/tab-view.md)** - Tabbed interface with overflow handling, keyboard navigation, and content management
- **[ScrollView Component](presentation/components/scroll-view.md)** - Smooth scrolling, custom scroll bars, momentum scrolling, and viewport culling

### Data Binding
- **[Binding & Reactive Rebuilding](presentation/binding.md)** - binding/observe/requestRebuild patterns and best practices

## Application Layer

### Application Architecture
- **[App Shell & Assembly](application/app-shell.md)** - AppShell integration, navigation coordination, and TopBar/content area interaction

## Platform Support

### Windows Platform
- **[Windows Platform Integration](infrastructure/platform-windows.md)** - WinWindowChrome window decoration and HitTest region handling

## Navigation Guide

- **Quick Start**: Begin with [Architecture Overview](architecture/overview.md) to understand the overall system design
- **Component Development**: Refer to [UI Framework Overview](presentation/ui-framework/overview.md) to learn about available components
- **Advanced Features**: Dive into [Data Binding System](presentation/binding.md) to master reactive data binding
- **Rendering Optimization**: Explore [Graphics & Rendering System](infrastructure/gfx.md) to understand low-level rendering
- **Platform Features**: Check [Windows Platform Integration](infrastructure/platform-windows.md) for platform-specific functionality

## Contributing & Feedback

Contributions are welcome! Please refer to the project's contribution guidelines and ensure that:

- Documentation changes align with actual code implementation
- Examples are tested and functional
- Cross-references between documents are accurate
- New components follow established documentation patterns

**Documentation Version**: Based on current codebase state  
**Last Updated**: 2024 (according to codebase status)  
**Maintainers**: Fangjia Qt6 C++ Development Team