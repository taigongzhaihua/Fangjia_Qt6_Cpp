# 声明式 TopBar 使用指南（中文）

本指南介绍如何使用声明式 UI 中的 TopBar 组件及其与 AppShell 的集成，并与现有 UiTopBar 保持一致的行为与外观。

## 快速开始

```cpp
auto bar = UI::topBar()
    ->followSystem(false)      // 初始不跟随系统主题
    ->cornerRadius(6.0f);      // 按钮圆角 6px
```

## 进阶配置

```cpp
// 自定义调色板（可选）
UiTopBar::Palette customPalette;
customPalette.bg = QColor(45, 55, 70, 180);
customPalette.icon = QColor(240, 245, 250);

auto customBar = UI::topBar()
    ->followSystem(true, true)                          // 开启动画的"跟随系统"
    ->cornerRadius(8.0f)                               // 8px 圆角
    ->svgTheme(":/icons/sun.svg", ":/icons/moon.svg")  // 主题切换图标
    ->svgFollow(":/icons/on.svg", ":/icons/off.svg")   // 跟随系统图标
    ->svgSystem(":/icons/min.svg", ":/icons/max.svg", ":/icons/close.svg") // 系统按钮
    ->palette(customPalette)                           // 应用自定义色彩
    ->onThemeToggle([](){ /* 主题切换回调 */ })
    ->padding(4, 8);                                   // 作为 Widget 装饰器使用
```

## API 参考

| 方法 | 参数 | 默认值 | 说明 |
|------|------|--------|------|
| `followSystem()` | `bool on, bool animate = false` | `false, false` | 设置/取消跟随系统主题，可选动画 |
| `cornerRadius()` | `float r` | `6.0f` | 设置按钮圆角半径 |
| `svgTheme()` | `QString sunWhenDark, QString moonWhenLight` | - | 设置主题切换图标（暗色/亮色） |
| `svgFollow()` | `QString on, QString off` | - | 设置"跟随系统"图标（开/关） |
| `svgSystem()` | `QString min, QString max, QString close` | - | 设置系统窗口按钮图标 |
| `palette()` | `const UiTopBar::Palette&` | - | 覆盖默认色彩方案 |
| `onThemeToggle()` | `std::function<void()>` | - | 用户点击主题切换时回调 |
| `onFollowToggle()` | `std::function<void()>` | - | 用户点击"跟随系统"时回调 |
| `onMinimize()/onMaxRestore()/onClose()` | `std::function<void()>` | - | 系统按钮回调 |

## 与 AppShell 集成

TopBar 与 AppShell 的常见组合（来自 apps/fangjia/MainOpenGlWindow.cpp 的 initializeDeclarativeShell）：

```cpp
return appShell()
    ->nav(wrap(&m_nav))
    ->topBar(UI::topBar()
        ->followSystem(followSystem, animateNow)
        ->cornerRadius(8.0f)
        ->svgTheme(":/icons/sun.svg", ":/icons/moon.svg")
        ->svgFollow(":/icons/follow_on.svg", ":/icons/follow_off.svg")
        ->svgSystem(":/icons/sys_min.svg", ":/icons/sys_max.svg", ":/icons/sys_close.svg")
        ->onThemeToggle([this]() { onThemeToggle(); })
        ->onFollowToggle([this]() { onFollowSystemToggle(); })
        ->onMinimize([this]() { showMinimized(); })
        ->onMaxRestore([this]() { /* maximize/restore */ })
        ->onClose([this]() { close(); })
    )
    ->content([this]() { return wrap(m_pageHost.get()); })
    ->navWidthProvider([this] { return m_nav.currentWidth(); })
    ->topBarHeight(52)
;
```

其中：
- `followSystem(followSystem, animateNow)`: 来自 ThemeManager 的当前模式，并由窗口在点击"跟随系统"时设置 `m_animateFollowChange` 以触发一次动画。
- 系统按钮回调均在窗口侧处理。

## 主题与资源上下文

- 主题变更通过 `UiRoot::propagateThemeChange(isDark)` 统一下发。
- 对于"重建型"声明式子树，`UI::RebuildHost` 会在 `requestRebuild()` 内按如下顺序同步环境：
  1) 若已设置 viewport，则先下发给 IUiContent
  2) 若已有主题状态，则先调用 `onThemeChanged(isDark)`
  3) 再更新资源上下文 `updateResourceContext(...)`（避免主题错配导致的图标/调色闪烁）
  4) 最后调用 `updateLayout(...)`

这与本仓库 RebuildHost.h 的实现一致，可避免 TopBar 在亮色主题下因事件重建而短暂使用错误的暗色调。