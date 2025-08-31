# UI 框架中文概览

本文概述仓库中 UI 框架的关键概念：组件生命周期、主题传播、布局协议、容器与声明式体系。

## IUiComponent 生命周期

按调用顺序：
1. `updateLayout(const QSize&)` —— 基于窗口逻辑尺寸计算布局
2. `updateResourceContext(IconCache&, QOpenGLFunctions*, float dpr)` —— 更新纹理/GL 上下文（与 DPR 相关）
3. `append(Render::FrameData&) const` —— 生成绘制命令
4. `tick()` —— 推进动画，返回是否仍需重绘

交互：`onMousePress/Move/Release/onWheel`；主题：`onThemeChanged(bool)`（默认转发至 `applyTheme`）。

根容器 `UiRoot` 负责：
- 统一分发事件与指针捕获
- 驱动所有顶级组件的布局、资源上下文与渲染
- 通过 `propagateThemeChange(isDark)` 递归下发主题

## 主题传播与重建顺序

声明式重建容器 `UI::RebuildHost` 会在 `requestRebuild()` 中按如下顺序同步：
1) 先下发 viewport（给 IUiContent/ILayoutable）
2) 调用 `onThemeChanged(isDark)`
3) 调用 `updateResourceContext(...)`
4) 调用 `updateLayout(...)`

此顺序已在源码中实现，能有效避免"主题切换/事件触发重建"期间的调色/图标闪烁。

## 布局协议与可选接口

- `ILayoutable::measure/arrange`：容器可对实现该接口的子项进行测量与安排
- `IUiContent::setViewportRect`：声明内容可获取实际绘制视口用于裁剪
- `IScrollContent`：被 `UiScrollView` 驱动滚动偏移

## 常用容器

- `UiPanel`：顺序布局（水平/垂直），支持交叉轴对齐
- `UiBoxLayout`：盒式权重/自然尺寸混合布局，支持主轴对齐（Start/Center/End/Space*）
- `UiGrid`：WPF 风格网格（Auto/Pixel/Star），支持跨行列与固有测量
- `UiContainer`：单子项容器，支持两轴对齐
- `UiScrollView`：垂直滚动容器，基于"移动子项 viewport 顶坐标 + 父级裁剪"实现
- `UiPage`：带标题区/内容区的页面容器
- `UiRoot`：顶级根容器，统一更新/事件/主题

## 声明式体系（Widget）

- `Widget`：装饰器支持（padding/margin/background/border/opacity/onTap/onHover）
- `DecoratedBox`：通用装饰容器，支持主题化 hover/press 背景
- `BasicWidgets::Text/Icon`：文本与图标，支持主题色/主题路径
- `AdvancedWidgets::Card`：卡片：主题化背景/边框/圆角/内边距
- `Layouts::Panel/Grid`：声明式布局
- `ScrollView`：声明式滚动容器（封装 UiScrollView）
- `TabView`：基于 DataProvider 的 Tab 容器
- `ComponentWrapper::wrap()`：将运行时 IUiComponent 包装为声明式子树
- `Binding::BindingHost`：重建宿主，提供 `observe(...)` 绑定 VM 信号触发 `requestRebuild()`

## 代码片段

### 使用 BindingHost 构建 Shell
```cpp
m_shellHost = UI::bindingHost([this]() -> WidgetPtr {
    const bool animateNow = m_animateFollowChange;
    const bool follow = m_themeMgr && m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem;
    return appShell()
        ->nav(wrap(&m_nav))
        ->topBar(UI::topBar()->followSystem(follow, animateNow))
        ->content([this]{ return wrap(m_pageHost.get()); });
});
```

### ScrollView 包裹内容
```cpp
auto scroll = UI::scrollView(
    UI::panel(/* children */)
);
```

### 文本与图标（主题色）
```cpp
auto title = UI::text("标题").
    ->fontSize(24)->fontWeight(QFont::Bold)
    ->themeColor(QColor(30,35,40), QColor(240,245,250));

auto icon = UI::icon(":/icons/any.svg")
    ->themePaths(":/icons/light.svg", ":/icons/dark.svg");
```