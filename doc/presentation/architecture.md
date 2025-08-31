# 表现层架构与声明式体系概览

本文档介绍 Fangjia Qt6 C++ 项目的表现层架构核心机制，包括 BindingHost、RebuildHost、UiRoot 的职责与协作，以及声明式 UI 体系的设计理念。

## 核心组件概览

### UiRoot - 根容器与事件协调器

`UiRoot` 是整个 UI 系统的根容器，负责：

- **统一事件分发**: 管理鼠标事件（press/move/release/wheel）与指针捕获
- **布局驱动**: 协调所有顶级组件的 `updateLayout()`、`updateResourceContext()` 调用
- **渲染协调**: 收集所有组件的渲染命令到 `Render::FrameData`
- **主题传播**: 通过 `propagateThemeChange(isDark)` 向整个组件树下发主题变更

### BindingHost - 响应式重建容器

`BindingHost` 提供响应式 UI 构建机制：

```cpp
m_shellHost = UI::bindingHost([this]() -> WidgetPtr {
    const bool follow = m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem;
    return appShell()
        ->nav(wrap(&m_nav))
        ->topBar(UI::topBar()->followSystem(follow, m_animateFollowChange))
        ->content([this]{ return wrap(m_pageHost.get()); });
});
```

**职责**：
- **依赖追踪**: 自动检测构建函数中访问的外部状态
- **变更响应**: 当依赖状态变化时触发重建
- **生命周期管理**: 管理重建产生的 Widget 实例

### RebuildHost - 重建顺序管理

`RebuildHost` 确保重建过程中的正确顺序，避免主题切换时的视觉闪烁：

```
requestRebuild() 执行顺序：
1. 设置 viewport（给 IUiContent/ILayoutable）
2. 调用 onThemeChanged(isDark)
3. 更新资源上下文 updateResourceContext(...)
4. 调用 updateLayout(...)
```

这一顺序设计确保组件在获得新布局尺寸前已完成主题适配与资源更新。

## 声明式 UI 体系

### Widget 基类与装饰器模式

声明式体系以 `Widget` 基类为核心，支持链式配置：

```cpp
auto decoratedPanel = UI::panel()
    ->padding(16)                        // 内边距装饰器
    ->margin(8, 12)                     // 外边距装饰器
    ->background(QColor(240, 240, 240)) // 背景装饰器
    ->border(QColor(200, 200, 200), 1.0f); // 边框装饰器
```

**装饰器特性**：
- **可组合**: 多个装饰器可任意组合
- **顺序无关**: 装饰器应用顺序不影响最终效果
- **类型安全**: 编译时检查装饰器与组件的兼容性

### 工厂函数与类型推导

UI 命名空间提供便利的工厂函数：

```cpp
using namespace UI;

// 容器组件
auto panel = panel();           // UiPanel 容器
auto grid = grid();             // UiGrid 网格布局
auto scroll = scrollView();     // UiScrollView 滚动容器

// 导航组件  
auto nav = navRail();           // NavRail 导航栏
auto top = topBar();            // TopBar 顶部栏

// 应用组件
auto shell = appShell();        // AppShell 应用外壳
```

## 组件生命周期

### IUiComponent 接口

所有 UI 组件实现 `IUiComponent` 接口，按以下顺序调用：

1. **`updateLayout(const QSize&)`** - 基于窗口逻辑尺寸计算布局
2. **`updateResourceContext(IconCache&, QOpenGLFunctions*, float dpr)`** - 更新纹理/GL 上下文（与 DPR 相关）
3. **`append(Render::FrameData&) const`** - 生成绘制命令
4. **`tick()`** - 推进动画，返回是否仍需重绘

### 主题变更流程

主题变更通过以下流程传播：

1. **触发源**: 用户操作或系统主题变化
2. **UiRoot 协调**: 调用 `propagateThemeChange(isDark)`
3. **组件响应**: 各组件的 `onThemeChanged(bool)` 被调用
4. **资源更新**: 组件更新调色板、图标缓存键等
5. **重新渲染**: 下一帧使用新的主题资源

## 与 MainOpenGlWindow 集成

主窗口类负责：

- **OpenGL 上下文管理**: 初始化渲染器与图形资源
- **事件桥接**: 将 Qt 事件转发给 UiRoot
- **主题管理**: 集成 ThemeManager，响应系统主题变化
- **页面路由**: 管理不同页面间的导航与切换

### 典型集成模式

```cpp
class MainOpenGlWindow : public QOpenGLWidget {
private:
    std::unique_ptr<UiRoot> m_uiRoot;
    std::unique_ptr<UI::BindingHost> m_shellHost;
    
    void setupUI() {
        m_uiRoot = std::make_unique<UiRoot>();
        m_shellHost = UI::bindingHost([this]() {
            return createAppShell();  // 构建应用外壳
        });
        m_uiRoot->setContent(m_shellHost.get());
    }
};
```

## 优势与设计目标

### 声明式编程的优势

- **可读性**: 代码结构直观反映 UI 层次结构
- **可维护性**: 状态变化自动触发 UI 更新，减少手动同步
- **可测试性**: 组件配置与业务逻辑分离，便于单元测试
- **性能优化**: 依赖追踪避免不必要的重建

### 架构设计目标

- **分离关注点**: 表现层专注 UI 渲染，业务逻辑在领域层处理
- **可扩展性**: 新组件可轻松集成到现有体系
- **跨平台一致性**: 抽象平台差异，提供统一的编程接口
- **性能导向**: 批量渲染与状态缓存优化性能表现

## 相关文档

- [UI 基础部件与容器](ui/components.md) - 具体组件的使用方法
- [Binding 与响应式重建](binding.md) - 数据绑定机制详解
- [声明式 TopBar 组件](ui/topbar/declarative-topbar.md) - TopBar 组件示例
- [渲染与图形系统](../infrastructure/gfx.md) - 底层渲染实现