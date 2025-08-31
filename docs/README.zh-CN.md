# 中文文档索引

本目录包含 Fangjia Qt6 C++ UI 框架的完整中文文档，涵盖核心架构、布局系统、声明式组件和使用指南。

## 文档结构

### 核心架构文档

#### [UI 架构详解](./UI_ARCHITECTURE.zh-CN.md)
- **适用读者**: 框架开发者、高级用户
- **内容概要**: IUiComponent 生命周期、UiRoot 职责、主题传播机制、事件处理流程
- **关键概念**: 生命周期顺序、焦点管理、主题避免闪烁设计、MainOpenGlWindow 集成
- **重要性**: ⭐⭐⭐⭐⭐ 理解框架运行机制的核心文档

#### [布局系统详解](./LAYOUTS.zh-CN.md)
- **适用读者**: UI 开发者、应用开发者
- **内容概要**: ILayoutable 接口、SizeConstraints、UiPanel、UiBoxLayout、UiGrid 布局容器
- **关键概念**: 测量与安排、权重分配、网格轨道定义、空间分配算法
- **重要性**: ⭐⭐⭐⭐ 掌握布局系统必读

### 组件使用文档

#### [声明式 TopBar 和 NavRail](./DECLARATIVE_NAV_TOPBAR.zh-CN.md)
- **适用读者**: 应用开发者、UI 开发者
- **内容概要**: 声明式 NavRail 和 TopBar 组件的完整使用指南
- **关键概念**: 跟随系统动画、BindingHost 集成、MainOpenGlWindow 交互、时长缩放 (2/3)
- **重要性**: ⭐⭐⭐⭐ 应用开发的核心组件

#### [TopBar 动画实现详解](../DECLARATIVE_TOPBAR_ANIMATION_IMPLEMENTATION.zh-CN.md)
- **适用读者**: UI 开发者、框架贡献者
- **内容概要**: TopBar 跟随系统动画的两阶段实现细节、交互判定和集成模式
- **关键概念**: 动画状态机、时长缩放、交互阈值、主题资源顺序
- **重要性**: ⭐⭐⭐ TopBar 动画机制深度理解

#### [滚动容器详解](./SCROLL_VIEW.zh-CN.md)
- **适用读者**: UI 开发者、应用开发者  
- **内容概要**: UiScrollView 使用方法、滚动条渲染、主题适配、IScrollContent 交互
- **关键概念**: width-bounded 测量、viewport 顶坐标滚动、淡入淡出动画、重复滚动
- **重要性**: ⭐⭐⭐ 处理长内容列表必读

### 声明式体系文档

#### [声明式 Widget 体系概览](./DECLARATIVE_OVERVIEW.zh-CN.md)
- **适用读者**: 应用开发者、框架贡献者
- **内容概要**: 完整的声明式 UI 体系，包括 Widget、装饰器、BindingHost、UI 工厂函数
- **关键概念**: 装饰器系统、ComponentWrapper、响应式重建、文本/图标缓存键、主题适配
- **重要性**: ⭐⭐⭐⭐⭐ 现代化 UI 开发的核心指南

### 开发者指南

#### [贡献指南](./CONTRIBUTING.zh-CN.md)
- **适用读者**: 开发者、文档贡献者
- **内容概要**: 代码和文档贡献规范、PR 提交流程、代码审查要求
- **关键概念**: 文档结构、术语统一、代码示例、提交规范、审查清单
- **重要性**: ⭐⭐⭐ 参与项目开发必读

#### [术语表](./GLOSSARY.zh-CN.md)
- **适用读者**: 所有用户
- **内容概要**: 项目中使用的中英文术语对照表，确保文档翻译一致性
- **关键概念**: 核心组件、主题系统、布局术语、交互动画、架构术语
- **重要性**: ⭐⭐⭐⭐ 理解文档和 API 的基础参考

## 快速导航

### 按开发场景分类

#### 🚀 应用开发入门
1. [声明式 Widget 体系概览](./DECLARATIVE_OVERVIEW.zh-CN.md) - 了解基本概念
2. [声明式 TopBar 和 NavRail](./DECLARATIVE_NAV_TOPBAR.zh-CN.md) - 构建应用外壳
3. [布局系统详解](./LAYOUTS.zh-CN.md) - 组织页面布局
4. [滚动容器详解](./SCROLL_VIEW.zh-CN.md) - 处理长内容

#### 🔧 框架深度定制
1. [UI 架构详解](./UI_ARCHITECTURE.zh-CN.md) - 理解框架原理
2. [布局系统详解](./LAYOUTS.zh-CN.md) - 自定义布局容器
3. [声明式 Widget 体系概览](./DECLARATIVE_OVERVIEW.zh-CN.md) - 扩展声明式组件

#### 🎨 主题和样式开发
1. [UI 架构详解](./UI_ARCHITECTURE.zh-CN.md) - 主题传播机制
2. [声明式 TopBar 和 NavRail](./DECLARATIVE_NAV_TOPBAR.zh-CN.md) - 主题动画和集成
3. [声明式 Widget 体系概览](./DECLARATIVE_OVERVIEW.zh-CN.md) - 装饰器和主题适配

### 按技术关键词分类

#### 🔄 动画和交互
- [TopBar 跟随系统动画](./DECLARATIVE_NAV_TOPBAR.zh-CN.md#跟随系统动画) - 两阶段动画、时长缩放
- [TopBar 动画实现详解](../DECLARATIVE_TOPBAR_ANIMATION_IMPLEMENTATION.zh-CN.md) - 深度实现细节和集成模式
- [滚动条淡入淡出](./SCROLL_VIEW.zh-CN.md#淡入淡出动画) - 滚动条动画
- [装饰器状态动画](./DECLARATIVE_OVERVIEW.zh-CN.md#状态动画) - hover/press 状态过渡

#### 🎯 生命周期和事件
- [IUiComponent 生命周期](./UI_ARCHITECTURE.zh-CN.md#iuicomponent-生命周期) - updateLayout → updateResourceContext → append → tick
- [事件转发机制](./UI_ARCHITECTURE.zh-CN.md#事件转发) - 鼠标、滚轮、键盘事件
- [主题传播顺序](./UI_ARCHITECTURE.zh-CN.md#主题变化顺序与避免闪烁) - 避免闪烁的设计

#### 📐 布局和测量
- [ILayoutable 接口](./LAYOUTS.zh-CN.md#ilayoutable-接口与-sizeconstraints) - measure/arrange 协议
- [网格布局算法](./LAYOUTS.zh-CN.md#内容测量算法) - Auto/Pixel/Star 轨道
- [ScrollView 测量策略](./SCROLL_VIEW.zh-CN.md#width-bounded-测量策略) - 宽度约束、高度自由

#### 🎨 主题和缓存
- [缓存键生成](./DECLARATIVE_OVERVIEW.zh-CN.md#texticon-的主题适配与缓存键) - 文本/图标缓存策略
- [主题适配流程](./SCROLL_VIEW.zh-CN.md#主题自适应配色) - applyTheme 实现
- [RebuildHost 顺序](./UI_ARCHITECTURE.zh-CN.md#rebuildhost-的设计) - 主题优先于资源上下文

## 代码示例索引

### 完整应用示例
- [AppShell 集成](./DECLARATIVE_NAV_TOPBAR.zh-CN.md#与-appshell-集成) - 完整的应用外壳配置
- [BindingHost 构建](./DECLARATIVE_OVERVIEW.zh-CN.md#绑定host响应式重建机制) - 响应式 UI 构建
- [MainOpenGlWindow 集成](./UI_ARCHITECTURE.zh-CN.md#mainopenglwindow-中的主题驱动) - 主窗口集成模式

### 组件配置示例
- [TopBar 高级配置](./DECLARATIVE_NAV_TOPBAR.zh-CN.md#高级配置) - 自定义调色板、回调、动画
- [Grid 网格配置](./LAYOUTS.zh-CN.md#网格配置示例) - 轨道定义、子项放置、跨行列
- [ScrollView 使用](./SCROLL_VIEW.zh-CN.md#基本用法) - 滚动容器基本配置

### 装饰器和样式示例
- [Widget 装饰器](./DECLARATIVE_OVERVIEW.zh-CN.md#装饰器应用示例) - padding、margin、background、border
- [DecoratedBox 交互](./DECLARATIVE_OVERVIEW.zh-CN.md#交互状态管理) - hover/press 效果
- [主题色应用](./DECLARATIVE_OVERVIEW.zh-CN.md#texticon-的主题适配与缓存键) - 文本/图标主题适配

## 技术参考

### 重要常量和配置
- **TopBar 高度**: 52px（默认）
- **NavRail 宽度**: 48px（收缩）/ 200px（展开，默认）
- **动画时长缩放**: 2/3（`scaleDuration` 函数）
- **交互阈值**: 0.4f（普通）/ 0.6f（跟随模式）

### 关键文件位置
- **声明式组件**: `presentation/ui/declarative/`
- **布局容器**: `presentation/ui/containers/`
- **基础接口**: `presentation/ui/base/`
- **运行时组件**: `presentation/ui/widgets/`
- **主窗口集成**: `apps/fangjia/MainOpenGlWindow.cpp`

### 核心类和接口
- `IUiComponent` - 所有 UI 组件的基础接口
- `ILayoutable` - 布局参与者接口
- `IUiContent` - 内容视口接口
- `Widget` - 声明式组件基类
- `RebuildHost` - 重建宿主基类
- `UiRoot` - 顶级容器类

## 版本说明

本文档基于当前代码库状态编写，涵盖了以下关键实现细节：

- **动画时长缩放**: 已在 `UiTopBar::scaleDuration` 和 `Ui::NavRail::scaleDuration` 中实现
- **主题传播顺序**: 已在 `RebuildHost::requestRebuild` 中正确实现
- **缓存键生成**: 已在 `RenderUtils::makeTextCacheKey` 和 `makeIconCacheKey` 中实现
- **交互阈值逻辑**: 已在 `UiTopBar::themeInteractive` 中实现

## 贡献和反馈

如果您发现文档中的错误或需要补充的内容，请：

1. 检查对应的源代码实现是否已更新
2. 确认文档描述与实际行为是否一致
3. 参考 [贡献指南](./CONTRIBUTING.zh-CN.md) 了解详细的贡献流程
4. 使用 [术语表](./GLOSSARY.zh-CN.md) 确保术语翻译的一致性
5. 提交反馈或改进建议

## 项目快速上手

### 项目结构概览

本项目采用分层架构设计：

- **`infrastructure/`** - 基础设施层：工具类、算法、平台抽象
- **`domain/`** - 领域层：业务逻辑、数据模型
- **`data/`** - 数据层：数据访问、外部服务集成
- **`presentation/`** - 表现层：UI 组件、视图模型、声明式系统
- **`apps/`** - 应用层：主程序入口、窗口管理

### 构建与运行

#### 依赖要求

- **CMake** 3.20 或更高版本
- **Qt6** 完整开发环境
- **C++23** 兼容编译器

#### 构建步骤

```bash
# 克隆项目
git clone <repository-url>
cd Fangjia_Qt6_Cpp

# 创建构建目录
mkdir build && cd build

# 配置 CMake
cmake ..

# 编译项目
cmake --build .
```

#### Windows 注意事项

- 确保 Qt6 环境变量正确配置
- 推荐使用 Visual Studio 2022 或 QtCreator
- 注意 DPI 缩放设置对 UI 渲染的影响

### 快速入门

#### 1. 找到入口点

主应用程序入口位于：
- **主窗口类**: `apps/fangjia/MainOpenGlWindow.h/.cpp`
- **程序入口**: `apps/fangjia/main.cpp`

#### 2. 了解声明式 UI

声明式 UI 组件位于：
- **组件定义**: `presentation/ui/declarative/`
- **使用示例**: 查看 `MainOpenGlWindow::initializeDeclarativeShell()` 方法

#### 3. 查看核心组件

- **TopBar**: `presentation/ui/widgets/UiTopBar.h/.cpp`
- **NavRail**: `presentation/ui/widgets/UiNavRail.h/.cpp`
- **布局系统**: `presentation/ui/containers/`

## 英文文档链接

- [English TopBar Documentation](./DECLARATIVE_NAV_TOPBAR.md) - 英文版 TopBar 声明式组件文档
- [English TopBar Animation Implementation](../DECLARATIVE_TOPBAR_ANIMATION_IMPLEMENTATION.md) - 英文版 TopBar 动画实现文档

## 相关文档

- [贡献指南](./CONTRIBUTING.zh-CN.md) - 了解如何参与项目开发
- [术语表](./GLOSSARY.zh-CN.md) - 中英文术语对照和翻译规范
- [编码规范](../编码规范.md) - C++/Qt 代码风格要求

---

**文档版本**: 基于 commit hash 的当前代码库状态  
**最后更新**: 2024年（根据代码库状态）  
**维护者**: Fangjia Qt6 C++ 开发团队