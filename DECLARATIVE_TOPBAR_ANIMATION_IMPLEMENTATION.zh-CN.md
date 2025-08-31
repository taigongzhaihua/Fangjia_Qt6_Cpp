# TopBar 跟随系统动画实现

[English Documentation](./DECLARATIVE_TOPBAR_ANIMATION_IMPLEMENTATION.md)

本文档解释 TopBar 在"跟随系统"开关时的两段式动画行为与交互判定，并给出典型集成模式。

## 动画行为

**启用跟随（`followSystem(true, true)`）：**
1. **阶段一**：主题按钮淡出（160ms，easeInOut）
2. **阶段二**：跟随按钮横向滑动至主题按钮位置（200ms）
3. **隐藏期间**：主题按钮不可交互

**取消跟随（`followSystem(false, true)`）：**
1. **阶段一**：跟随按钮滑回原位（180ms）
2. **阶段二**：主题按钮淡入（160ms）
3. **淡入完成后**：主题按钮恢复交互

实现与常量定义可见 `widgets/UiTopBar.cpp`：
- `scaleDuration(...)` 统一使用 2/3 时长缩放
- `AnimPhase::{HideTheme_FadeOut, MoveFollow_Right, MoveFollow_Left, ShowTheme_FadeIn}`
- `themeInteractive()` 在跟随模式下使用更高的可交互阈值（0.6）

## 交互判定

```cpp
bool UiTopBar::themeInteractive() const {
    if (m_followSystem && m_animPhase != AnimPhase::ShowTheme_FadeIn) {
        return m_themeAlpha > 0.6f;
    }
    return m_themeAlpha > 0.4f;
}
```

## 集成模式（窗口侧）

在窗口中，点击"跟随系统"会：
- 将 `m_animateFollowChange` 置为 `true`（仅用于下一次重建）
- 切换 ThemeManager 模式
- 在下一轮事件循环触发 RebuildHost 重建与动画计时器启动

代码参考 `apps/fangjia/MainOpenGlWindow.cpp`：

```cpp
void MainOpenGlWindow::onFollowSystemToggle() const {
    const_cast<MainOpenGlWindow*>(this)->m_animateFollowChange = true;
    setFollowSystem(!followSystem());
}
```

在 Shell 构建器中：

```cpp
auto bar = UI::topBar()->followSystem(followSystem, m_animateFollowChange);
```

## 与主题、资源上下文的顺序

`UI::RebuildHost::requestRebuild()` 按"先应用主题、再更新资源、最后更新布局"的顺序同步环境，可避免在重建期间出现 TopBar 临时使用错误调色或错误图标缓存键的情况。