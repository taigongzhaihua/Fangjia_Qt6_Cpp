[English](../../../doc/presentation/ui/topbar/declarative-topbar.md) | **简体中文**

# 声明式 TopBar 与 NavRail 组件

本文档介绍新的声明式 UI 组件 NavRail 和 TopBar，它们提供一流的声明式 API，无需通过 `UI::wrap()` 包装运行时组件。

## 概览

声明式组件与现有的 `Ui::NavRail` 和 `UiTopBar` 组件保持一致的行为，同时提供现代化的链式 API 配置接口。

### 核心特性

- **链式 API**: 流畅的配置接口
- **行为一致**: 与运行时组件相同的样式和行为
- **生命周期管理**: 自动转发事件和主题变化
- **装饰器支持**: 完全集成 Widget 基类装饰器（padding、margin 等）

## NavRail 组件

`UI::NavRail` 组件提供导航栏的声明式接口。

### 基本用法

```cpp
#include "UI.h"

auto rail = UI::navRail()
    ->widths(48, 200)          // 收缩: 48px, 展开: 200px
    ->iconSize(22)             // 图标大小（逻辑像素）
    ->itemHeight(48)           // 导航项高度
    ->labelFontPx(13);         // 标签字体大小
```

### 高级配置

```cpp
// 自定义调色板
Ui::NavPalette customPalette;
customPalette.railBg = QColor(30, 35, 40, 200);
customPalette.itemSelected = QColor(0, 120, 255, 180);
customPalette.iconColor = QColor(220, 225, 230);

auto customRail = UI::navRail()
    ->dataProvider(myNavDataProvider)                    // 绑定数据源
    ->widths(64, 220)                                   // 自定义尺寸
    ->iconSize(24)                                      // 更大的图标
    ->toggleSvg(":/icons/expand.svg", ":/icons/collapse.svg")  // 自定义切换图标
    ->palette(customPalette)                            // 应用自定义颜色
    ->padding(8)                                        // 装饰器支持
    ->margin(4, 0, 0, 0);                              // 左边距
```

### API 参考

| 方法 | 参数 | 默认值 | 说明 |
|------|------|--------|------|
| `dataProvider()` | `INavDataProvider*` | `nullptr` | 绑定导航数据提供者 |
| `widths()` | `int collapsed, int expanded` | `48, 200` | 设置收缩和展开宽度 |
| `iconSize()` | `int logicalPx` | `22` | 设置图标大小（逻辑像素） |
| `itemHeight()` | `int px` | `48` | 设置导航项高度 |
| `labelFontPx()` | `int px` | `13` | 设置标签字体大小 |
| `toggleSvg()` | `QString expand, QString collapse` | - | 设置展开/收缩按钮图标 |
| `palette()` | `const Ui::NavPalette&` | - | 覆盖默认色彩方案 |

## TopBar 组件

`UI::TopBar` 组件提供顶部栏的声明式接口。

### 基本用法

```cpp
auto bar = UI::topBar()
    ->followSystem(false)      // 初始不跟随系统主题
    ->cornerRadius(6.0f);      // 按钮圆角 6px
```

### 高级配置

```cpp
// 自定义调色板
UiTopBar::Palette customPalette;
customPalette.bg = QColor(45, 55, 70, 180);
customPalette.icon = QColor(240, 245, 250);

auto customBar = UI::topBar()
    ->followSystem(true, true)                          // 跟随系统主题，启用动画
    ->cornerRadius(8.0f)                               // 8px 圆角
    ->svgTheme(":/icons/sun.svg", ":/icons/moon.svg")  // 主题切换图标
    ->svgFollow(":/icons/on.svg", ":/icons/off.svg")   // 跟随系统图标
    ->svgSystem(":/icons/min.svg", ":/icons/max.svg", ":/icons/close.svg")  // 窗口控制按钮
    ->palette(customPalette)                           // 应用自定义颜色
    ->onThemeToggle([]() {                             // 主题切换回调
        // 处理主题切换逻辑
    })
    ->padding(4, 8);                                   // 装饰器支持
```

### API 参考

| 方法 | 参数 | 默认值 | 说明 |
|------|------|--------|------|
| `followSystem()` | `bool on, bool animate = false` | `false, false` | 设置系统主题跟随，可选动画 |
| `cornerRadius()` | `float r` | `6.0f` | 设置按钮圆角半径 |
| `svgTheme()` | `QString sunWhenDark, QString moonWhenLight` | - | 设置主题切换图标 |
| `svgFollow()` | `QString on, QString off` | - | 设置跟随系统图标 |
| `svgSystem()` | `QString min, QString max, QString close` | - | 设置窗口控制图标 |
| `palette()` | `const UiTopBar::Palette&` | - | 覆盖默认色彩方案 |
| `onThemeToggle()` | `std::function<void()>` | - | 设置主题切换回调 |
| `onFollowToggle()` | `std::function<void()>` | - | 设置跟随系统切换回调 |

### 跟随系统动画

TopBar 组件包含复杂的两阶段"跟随系统"切换动画。详细的动画实现请参阅 [TopBar 动画实现](animation.md)。

## 与 AppShell 集成

两个组件都与现有的 AppShell 组件无缝集成：

```cpp
auto app = UI::appShell()
    ->nav(UI::navRail()->widths(64, 220)->iconSize(24))
    ->topBar(UI::topBar()->followSystem(true)->cornerRadius(8.0f))
    ->content(myMainContent);
```

## 与 BindingHost 的集成范式

使用 `BindingHost` 构建 Shell 的典型模式：

```cpp
m_shellHost = UI::bindingHost([this]() -> WidgetPtr {
    const bool animateNow = m_animateFollowChange;
    const bool follow = m_themeMgr && m_themeMgr->mode() == ThemeManager::ThemeMode::FollowSystem;
    return appShell()
        ->nav(wrap(&m_nav))
        ->topBar(UI::topBar()
            ->followSystem(follow, animateNow)
            ->onThemeToggle([this]() { onThemeToggle(); })
            ->onFollowToggle([this]() { onFollowSystemToggle(); })
        )
        ->content([this]{ return wrap(m_pageHost.get()); });
});
```

其中 `m_animateFollowChange` 是动画意图标志，用于标识当前重建是否由用户的"跟随系统"操作触发。

## 与 MainOpenGlWindow 交互

TopBar 组件通过回调与主窗口交互：

```cpp
auto topBar = UI::topBar()
    ->onThemeToggle([this]() { 
        // 调用窗口的主题切换逻辑
        onThemeToggle(); 
    })
    ->onFollowToggle([this]() { 
        // 调用窗口的跟随系统切换逻辑
        onFollowSystemToggle(); 
    });
```

窗口侧的处理方法：
- `onThemeToggle()`: 切换明亮/暗色主题
- `onFollowSystemToggle()`: 切换是否跟随系统主题

## 装饰器支持

两个组件都继承自 Widget 基类，支持所有标准装饰器：

```cpp
auto decoratedRail = UI::navRail()
    ->widths(48, 200)
    ->padding(8)                           // 内边距
    ->margin(4, 0, 0, 0)                  // 左边距
    ->background(QColor(0, 0, 0, 50))     // 半透明背景
    ->border(QColor(100, 100, 100), 1.0f); // 边框
```

## 工厂函数

UI 命名空间中提供便利的工厂函数：

```cpp
using namespace UI;

auto nav = navRail();     // 等同于 make_widget<NavRail>()
auto top = topBar();      // 等同于 make_widget<TopBar>()
```

## 主题传播与资源上下文

- 主题变更通过 `UiRoot::propagateThemeChange(isDark)` 统一下发
- 对于重建型声明式子树，`UI::RebuildHost::requestRebuild()` 按特定顺序同步环境：
  1. 设置 viewport（给 IUiContent）
  2. 调用 `onThemeChanged(isDark)`
  3. 更新资源上下文 `updateResourceContext(...)`
  4. 调用 `updateLayout(...)`

此顺序已在 `presentation/ui/declarative/RebuildHost.h` 中实现，避免主题切换时的闪烁问题。

## 注意事项

- 这些组件与现有的 `Ui::NavRail` 和 `UiTopBar` 组件保持相同的行为和样式
- 主题变化自动转发到包装的运行时组件
- 所有鼠标事件和生命周期方法都正确委托
- 包装组件拥有运行时实例并管理其生命周期
- MainOpenGlWindow 集成在此实现中未更改，避免动画/WinWindowChrome 耦合问题

## 相关文档

- [表现层架构概览](../../architecture.md) - 组件生命周期和主题传播机制
- [UI 基础部件与容器](../components.md) - Panel、BoxLayout、Grid 布局详解
- [Binding 与响应式重建](../../binding.md) - 数据绑定与声明式体系
- [TopBar 动画实现](animation.md) - 跟随系统动画的详细实现