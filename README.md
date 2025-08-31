# Fangjia Qt6 C++

## 项目简介

Fangjia Qt6 C++ 是一个基于 Qt 6 + OpenGL 的现代化自绘 UI 应用框架，展示了企业级 C++ 桌面应用的最佳实践。项目采用分层架构设计，提供完整的 UI 组件体系、声明式编程接口、主题管理和跨平台支持。

## 主要特性

- **自定义渲染管线** - 基于 OpenGL 的高性能渲染系统（Renderer、RenderData、IconCache）
- **声明式 UI 体系** - 现代化的链式 API 与组件装饰器系统
- **丰富的 UI 组件** - UiRoot、UiPanel、UiGrid、UiTopBar、NavRail、ScrollView 等
- **响应式数据绑定** - BindingHost/RebuildHost 机制支持自动 UI 更新
- **主题管理系统** - 支持明暗主题切换与跟随系统主题
- **跨平台支持** - Windows 平台窗口装饰与交互优化

## 工程结构概览

```
├─ presentation/        # 表现层（UI 框架、声明式组件、数据绑定）
├─ infrastructure/      # 基础设施（渲染、图形、平台抽象）
├─ domain/             # 业务领域（模型、服务）
├─ apps/               # 应用程序（主窗口、页面路由）
├─ resources/          # Qt 资源文件
├─ doc/                # 项目文档
└─ examples/           # 代码示例
```

## 构建与运行

### 环境要求

- **Qt 6 组件**: Core、Gui、Widgets、OpenGL、Svg
- **构建工具**: CMake ≥ 3.16
- **编译器**: 支持 C++23 的现代编译器

### 构建步骤

```bash
# Linux/macOS
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build -j$(nproc)

# Windows (在 Qt 开发者命令提示符中)
cmake -S . -B build -DCMAKE_BUILD_TYPE=Debug
cmake --build build --config Debug
```

### 运行 Demo

构建完成后运行可执行文件：
- Linux/macOS: `./build/apps/fangjia/fangjia`
- Windows: `.\build\apps\fangjia\Debug\FangJia.exe`

应用启动后可体验声明式 UI 组件、主题切换、导航交互等核心功能。

## 快速开始

1. **克隆仓库并构建项目**
2. **运行示例应用**，熟悉 UI 组件和交互模式
3. **查看代码示例** (`examples/` 目录) 了解声明式 API 用法
4. **阅读架构文档**，深入理解框架设计理念

## 文档入口

完整的中文技术文档请参阅：**[doc/index.md](doc/index.md)**

文档按系统分组组织，涵盖架构设计、组件使用、开发指南等各个方面。