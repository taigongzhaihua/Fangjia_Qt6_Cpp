[English](../doc/index.md) | **简体中文**

# Fangjia Qt6 C++ 文档

本文档按系统组织，为开发者提供全面的技术参考和开发指南。

## 架构与设计

### 系统概览
- **[架构概览](architecture/overview.md)** - 系统设计、组件生命周期、数据绑定和线程模型

## 构建与开发

### 特定平台构建指南
- **[Windows 构建指南](build/windows.md)** - Visual Studio 设置、Qt 安装和 Windows 构建配置
- **[Linux 构建指南](build/linux.md)** - 包安装、编译和 Linux 发行版开发环境设置
- **[macOS 构建指南](build/macos.md)** - Xcode 设置、Qt 安装和 macOS 通用二进制文件构建

## 基础设施系统

### 图形与平台
- **[图形与渲染系统](infrastructure/gfx.md)** - RenderData、IconCache、坐标系统和 OpenGL 渲染管线
- **[Windows 平台集成](infrastructure/platform-windows.md)** - 原生窗口装饰、命中测试、DPI 感知和 Windows 特定功能

## 数据管理

### 数据层架构
- **[数据管理概览](data/overview.md)** - ThemeManager、AppConfig、ViewModel 模式和响应式数据绑定

## 表现层

### 架构与设计
- **[表现层架构](presentation/architecture.md)** - 声明式体系概览、核心组件（UiRoot、BindingHost、RebuildHost）和架构设计原则

### UI 框架
- **[UI 框架概览](presentation/ui-framework/overview.md)** - 组件架构、生命周期、容器和小部件系统
- **[布局系统](presentation/ui-framework/layouts.md)** - UiPanel、UiGrid、UiContainer、尺寸策略和响应式设计
- **[主题与渲染](presentation/ui-framework/theme-and-rendering.md)** - 主题管理、调色板、过渡效果和样式自定义

### UI 组件
- **[TopBar 组件](presentation/components/top-bar.md)** - 窗口控件、主题切换、系统集成和声明式 API
- **[TopBar 动画系统](presentation/components/top-bar-animation.md)** - 两阶段动画行为、状态机和性能优化
- **[NavRail 组件](presentation/components/nav-rail.md)** - 带有 DataProvider 集成、动画和响应式行为的导航轨道
- **[TabView 组件](presentation/components/tab-view.md)** - 带有溢出处理、键盘导航和内容管理的选项卡界面
- **[ScrollView 组件](presentation/components/scroll-view.md)** - 平滑滚动、自定义滚动条、惯性滚动和视口裁剪

### 数据绑定
- **[绑定与响应式重建](presentation/binding.md)** - binding/observe/requestRebuild 模式和最佳实践

## 应用层

### 应用架构
- **[App Shell 与应用组装](application/app-shell.md)** - AppShell 集成、导航协调和 TopBar/内容区域交互

## 平台支持

### Windows 平台
- **[Windows 平台集成](infrastructure/platform-windows.md)** - WinWindowChrome 窗口装饰和 HitTest 区域处理

## 导航指南

- **快速入门**: 从 [架构概览](architecture/overview.md) 开始了解整体系统设计
- **组件开发**: 参考 [UI 框架概览](presentation/ui-framework/overview.md) 学习可用组件
- **高级特性**: 深入学习 [数据绑定系统](presentation/binding.md) 掌握响应式数据绑定
- **渲染优化**: 探索 [图形与渲染系统](infrastructure/gfx.md) 了解底层渲染
- **平台特性**: 查看 [Windows 平台集成](infrastructure/platform-windows.md) 了解平台特定功能

## 贡献与反馈

欢迎贡献！请参考项目的贡献指南并确保：

- 文档更改与实际代码实现保持一致
- 示例经过测试且功能正常
- 文档之间的交叉引用准确无误
- 新组件遵循已建立的文档模式

**文档版本**: 基于当前代码库状态  
**最后更新**: 2024年（根据代码库状态）  
**维护者**: Fangjia Qt6 C++ 开发团队