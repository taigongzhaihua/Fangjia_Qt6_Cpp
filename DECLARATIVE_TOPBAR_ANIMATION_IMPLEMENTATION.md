# TopBar Follow System Animation Implementation

[简体中文版本 (Simplified Chinese)](./DECLARATIVE_TOPBAR_ANIMATION_IMPLEMENTATION.zh-CN.md)

This document explains the two-phase animation behavior and interaction logic for the TopBar's "Follow System" toggle, along with typical integration patterns.

## Animation Behavior

**Enabling Follow System (`followSystem(true, true)`):**
1. **Phase 1**: Theme button fades out (160ms with ease-in-out curve)
2. **Phase 2**: Follow button slides right into theme button's position (200ms)
3. Theme button becomes non-interactive while hidden

**Disabling Follow System (`followSystem(false, true)`):**
1. **Phase 1**: Follow button slides left back to original position (180ms)
2. **Phase 2**: Theme button fades in to full opacity (160ms)
3. Theme button becomes interactive again after fade-in completes

Implementation and constants can be found in `widgets/UiTopBar.cpp`:
- `scaleDuration(...)` uniformly uses 2/3 duration scaling
- `AnimPhase::{HideTheme_FadeOut, MoveFollow_Right, MoveFollow_Left, ShowTheme_FadeIn}`
- `themeInteractive()` uses higher interaction threshold (0.6) in follow mode

## Interaction Logic

```cpp
bool UiTopBar::themeInteractive() const {
    if (m_followSystem && m_animPhase != AnimPhase::ShowTheme_FadeIn) {
        return m_themeAlpha > 0.6f;
    }
    return m_themeAlpha > 0.4f;
}
```

## Integration Pattern (Window Side)

In the window, clicking "Follow System" will:
- Set `m_animateFollowChange` to true (only for next rebuild)
- Switch ThemeManager mode
- Trigger RebuildHost rebuild and animation timer start in next event loop

Code reference from `apps/fangjia/MainOpenGlWindow.cpp`:

```cpp
void MainOpenGlWindow::onFollowSystemToggle() const {
    const_cast<MainOpenGlWindow*>(this)->m_animateFollowChange = true;
    setFollowSystem(!followSystem());
}
```

In the Shell builder:

```cpp
auto bar = UI::topBar()->followSystem(followSystem, m_animateFollowChange);
```

## Theme and Resource Context Ordering

`UI::RebuildHost::requestRebuild()` synchronizes environment in "apply theme first, then update resources, finally update layout" order, which avoids TopBar temporarily using incorrect colors or icon cache keys during rebuild.