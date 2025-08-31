# Fangjia Qt6 C++ 技术文档总目录

本文档按系统分组组织，为开发者提供完整的技术参考和开发指南。

## 表现层系统 (Presentation)

### 架构设计
- **[表现层架构概览](presentation/architecture.md)** - BindingHost、RebuildHost、UiRoot 核心机制与声明式体系设计理念

### UI 组件与容器
- **[UI 基础部件与容器](presentation/ui/components.md)** - UiPanel、UiBoxLayout、UiGrid、UiScrollView 等核心容器的使用与配置
- **[声明式 TopBar 组件](presentation/ui/topbar/declarative-topbar.md)** - TopBar 与 NavRail 声明式 API、配置选项与集成模式
- **[TopBar 动画实现](presentation/ui/topbar/animation.md)** - 跟随系统主题动画的实现细节与交互逻辑

### 数据绑定系统  
- **[Binding 与响应式重建](presentation/binding.md)** - binding/observe/requestRebuild 使用模式与最佳实践

## 基础设施系统 (Infrastructure)

### 图形渲染
- **[渲染与图形系统](infrastructure/gfx.md)** - RenderData、IconCache、RenderUtils 渲染管线与资源管理

## 平台支持 (Platform)

### Windows 平台
- **[Windows 平台集成](platform/windows.md)** - WinWindowChrome 窗口装饰与 HitTest 区域处理

## 应用层系统 (Application)

### 应用架构
- **[App Shell 与应用组装](application/app-shell.md)** - AppShell、导航、TopBar、内容区的拼装与交互协调

---

## 文档导航说明

- **快速入门**: 建议从 [表现层架构概览](presentation/architecture.md) 开始了解整体设计
- **组件开发**: 参考 [UI 基础部件与容器](presentation/ui/components.md) 了解可用组件
- **高级特性**: 深入 [Binding 与响应式重建](presentation/binding.md) 掌握数据绑定机制
- **渲染优化**: 通过 [渲染与图形系统](infrastructure/gfx.md) 理解底层渲染实现
- **平台特性**: 查看 [Windows 平台集成](platform/windows.md) 了解平台特定功能

## 贡献与反馈

文档内容基于当前代码库实现编写，如发现不一致或需要补充的内容，请参与贡献或提供反馈。