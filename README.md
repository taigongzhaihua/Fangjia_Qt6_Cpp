# Fangjia Qt6 C++

## Project Overview

Fangjia Qt6 C++ is a modern desktop application framework built on Qt 6 and OpenGL, showcasing enterprise-grade C++ desktop application best practices. The project features a layered architecture design with comprehensive UI components, declarative programming interfaces, theme management, and cross-platform support.

## Key Features

- **Custom Rendering Pipeline** - High-performance OpenGL-based rendering system (Renderer, RenderData, IconCache)
- **Declarative UI System** - Modern chain-based APIs and component decorator system
- **Rich UI Components** - UiRoot, UiPanel, UiGrid, UiTopBar, NavRail, ScrollView, and more
- **Reactive Data Binding** - BindingHost/RebuildHost mechanism with automatic UI updates
- **Theme Management System** - Light/dark theme switching with system theme following
- **Cross-Platform Support** - Windows platform window decoration and interaction optimization

## Project Structure

```
├─ presentation/        # Presentation layer (UI framework, declarative components, data binding)
├─ infrastructure/      # Infrastructure (rendering, graphics, platform abstraction)
├─ domain/             # Business domain (models, services)
├─ apps/               # Applications (main window, page routing)
├─ resources/          # Qt resource files
├─ doc/                # Project documentation
└─ examples/           # Code examples
```

## Build & Run

### Requirements

- **Qt 6 Components**: Core, Gui, Widgets, OpenGL, Svg
- **Build Tools**: CMake ≥ 3.16
- **Compiler**: Modern C++23 compatible compiler

### Build Steps

```bash
# Linux/macOS
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Windows (in Qt Developer Command Prompt)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

### Run Demo

After successful build, run the executable:
- Linux/macOS: `./build/apps/fangjia/fangjia`
- Windows: `.\build\apps\fangjia\Debug\FangJia.exe`

The application demonstrates declarative UI components, theme switching, navigation interactions, and other core features.

## Quick Start

1. **Clone the repository and build the project**
2. **Run the sample application** to explore UI components and interaction patterns
3. **Review code examples** (`examples/` directory) to understand declarative API usage
4. **Read architecture documentation** to understand the framework design philosophy

## Documentation

Comprehensive technical documentation is available in the **[doc/](doc/index.md)** directory:

### Getting Started
- **[Architecture Overview](doc/architecture/overview.md)** - Understand the system design and component relationships
- **[Build Guides](doc/build/)** - Platform-specific setup and compilation instructions  
- **[UI Framework Overview](doc/presentation/ui-framework/overview.md)** - Learn about the component system and declarative APIs

### Key Features Documentation
- **[Graphics & Rendering](doc/infrastructure/gfx.md)** - High-performance OpenGL rendering pipeline
- **[Theme System](doc/presentation/ui-framework/theme-and-rendering.md)** - Light/dark theme support with system following
- **[Layout System](doc/presentation/ui-framework/layouts.md)** - Flexible container and positioning system
- **[Component Library](doc/presentation/components/)** - TopBar, NavRail, TabView, ScrollView components

### Advanced Topics
- **[Data Binding](doc/presentation/binding.md)** - Reactive data binding and UI updates
- **[Platform Integration](doc/infrastructure/platform-windows.md)** - Native Windows features and optimizations

For a complete table of contents, see **[doc/index.md](doc/index.md)**.