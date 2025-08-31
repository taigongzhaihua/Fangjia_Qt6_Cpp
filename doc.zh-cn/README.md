[English](../README.md) | **简体中文**

# Fangjia Qt6 C++

## 项目概述

Fangjia Qt6 C++ 是一个基于 Qt 6 和 OpenGL 构建的现代桌面应用程序框架，展示了企业级 C++ 桌面应用程序的最佳实践。该项目采用分层架构设计，具有全面的 UI 组件、声明式编程接口、主题管理和跨平台支持。

## 核心特性

- **自定义渲染管线** - 高性能基于 OpenGL 的渲染系统（Renderer、RenderData、IconCache）
- **声明式 UI 系统** - 现代链式 API 和组件装饰器系统
- **丰富的 UI 组件** - UiRoot、UiPanel、UiGrid、UiTopBar、NavRail、ScrollView 等
- **响应式数据绑定** - BindingHost/RebuildHost 机制，支持自动 UI 更新
- **主题管理系统** - 明/暗主题切换，支持跟随系统主题
- **跨平台支持** - Windows 平台窗口装饰和交互优化

## 项目结构

```
├─ presentation/        # 表现层（UI 框架、声明式组件、数据绑定）
├─ infrastructure/      # 基础设施层（渲染、图形、平台抽象）
├─ domain/             # 业务领域层（模型、服务）
├─ apps/               # 应用程序（主窗口、页面路由）
├─ resources/          # Qt 资源文件
├─ doc/                # 项目文档
└─ examples/           # 代码示例
```

## 构建与运行

### 系统要求

- **Qt 6 组件**: Core、Gui、Widgets、OpenGL、Svg
- **构建工具**: CMake ≥ 3.16
- **编译器**: 支持现代 C++23 的编译器

### 构建步骤

```bash
# Linux/macOS
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Windows（在 Qt 开发者命令提示符中）
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

### 运行演示

构建成功后，运行可执行文件：
- Linux/macOS: `./build/apps/fangjia/fangjia`
- Windows: `.\build\apps\fangjia\Debug\FangJia.exe`

该应用程序演示了声明式 UI 组件、主题切换、导航交互和其他核心特性。

## 快速入门

1. **克隆仓库并构建项目**
2. **运行示例应用程序**来探索 UI 组件和交互模式
3. **查看代码示例**（`examples/` 目录）以了解声明式 API 的使用
4. **阅读架构文档**以理解框架设计理念

## 文档

在 **[doc.zh-cn/](index.md)** 目录中提供了全面的技术文档：

### 入门指南
- **[架构概览](architecture/overview.md)** - 了解系统设计和组件关系
- **[构建指南](build/)** - 特定平台的设置和编译说明
- **[UI 框架概览](presentation/ui-framework/overview.md)** - 学习组件系统和声明式 API

### 核心特性文档
- **[图形与渲染](infrastructure/gfx.md)** - 高性能 OpenGL 渲染管线
- **[主题系统](presentation/ui-framework/theme-and-rendering.md)** - 明/暗主题支持，跟随系统主题
- **[布局系统](presentation/ui-framework/layouts.md)** - 灵活的容器和定位系统
- **[组件库](presentation/components/)** - TopBar、NavRail、TabView、ScrollView 组件

### 高级主题
- **[数据绑定](presentation/binding.md)** - 响应式数据绑定和 UI 更新
- **[平台集成](infrastructure/platform-windows.md)** - Windows 原生特性和优化

完整的目录请参阅 **[doc.zh-cn/index.md](index.md)**。