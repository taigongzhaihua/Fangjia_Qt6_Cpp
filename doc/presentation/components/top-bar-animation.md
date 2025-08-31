**English** | [简体中文](../../../doc.zh-cn/presentation/components/top-bar-animation.md)

# TopBar Follow System Animation

## Overview

The TopBar component implements a sophisticated two-phase animation system when toggling the "follow system theme" setting. This creates smooth visual transitions that clearly communicate the relationship between the theme toggle and follow system buttons while maintaining intuitive interaction.

## Animation Behavior

### Enabling Follow System (`followSystem(true, true)`)

When "follow system" is enabled with animation:

1. **Phase 1 - Theme Button Fade Out** (160ms, easeInOut)
   - Theme button gradually fades to transparent
   - Button becomes non-interactive during fade
   - Follow button remains in original position

2. **Phase 2 - Follow Button Slide Right** (200ms, easeInOut)  
   - Follow button smoothly slides to theme button position
   - Theme button remains hidden during slide
   - Animation completes with follow button in theme position

### Disabling Follow System (`followSystem(false, true)`)

When "follow system" is disabled with animation:

1. **Phase 1 - Follow Button Slide Left** (180ms, easeInOut)
   - Follow button slides back to original position
   - Theme button remains hidden during slide
   - Transition prepares for theme button return

2. **Phase 2 - Theme Button Fade In** (160ms, easeInOut)
   - Theme button gradually fades in at original position
   - Follow button settles in original position  
   - Animation completes with both buttons visible and interactive

## Implementation Details

### Animation State Machine

The animation system uses a state machine defined in `presentation/ui/widgets/UiTopBar.cpp`:

```cpp
enum class AnimPhase {
    Idle,                    // No animation active
    HideTheme_FadeOut,      // Phase 1: Theme button fading out
    MoveFollow_Right,       // Phase 2: Follow button moving right  
    MoveFollow_Left,        // Phase 1: Follow button moving left
    ShowTheme_FadeIn        // Phase 2: Theme button fading in
};
```

### Duration Scaling

All animation durations use `scaleDuration()` for consistent timing across different hardware:

```cpp
int UiTopBar::scaleDuration(int durationMs) {
    // Apply 2/3 scaling for responsive feel
    return static_cast<int>(durationMs * 0.67f);
}
```

Standard durations:
- Fade out: `scaleDuration(160)` ≈ 107ms
- Slide right: `scaleDuration(200)` ≈ 133ms  
- Slide left: `scaleDuration(180)` ≈ 120ms
- Fade in: `scaleDuration(160)` ≈ 107ms

### Core Animation Methods

```cpp
void UiTopBar::startAnimSequence(const bool followOn) {
    if (!m_animClock.isValid()) m_animClock.start();
    m_phaseStartAlpha = m_themeAlpha;
    m_phaseStartSlide = m_followSlide;

    if (followOn) 
        beginPhase(AnimPhase::HideTheme_FadeOut, scaleDuration(160));
    else          
        beginPhase(AnimPhase::MoveFollow_Left, scaleDuration(180));
}

void UiTopBar::beginPhase(const AnimPhase ph, const int durationMs) {
    m_animPhase = ph;
    m_animDurationMs = durationMs;
    m_phaseStartMs = m_animClock.elapsed();
}
```

## Interaction & Hit Testing

### Theme Button Interactivity

The theme button's interactivity is carefully managed during animations:

```cpp
bool UiTopBar::themeInteractive() const {
    if (m_followSystem && m_animPhase != AnimPhase::ShowTheme_FadeIn) {
        return m_themeAlpha > 0.6f;  // Higher threshold when following system
    }
    return m_themeAlpha > 0.4f;      // Standard threshold
}
```

**Interaction States:**
- **Normal Mode**: Theme button interactive when alpha > 0.4
- **Follow System Mode**: Theme button interactive when alpha > 0.6 (except during fade-in)
- **During Animation**: Button responsiveness scales with visibility

### Mouse Event Handling

Button hit testing accounts for animation state:

```cpp
bool UiTopBar::onMousePress(const QPoint& pos) {
    // Check theme button (if interactive)
    if (m_btnTheme.contains(pos) && themeInteractive()) {
        // Handle theme toggle
        return true;
    }
    
    // Check follow button
    if (m_btnFollow.contains(pos)) {
        // Handle follow system toggle
        return true;
    }
    
    // Check system buttons (always interactive)
    if (m_btnClose.contains(pos) || m_btnMax.contains(pos) || m_btnMin.contains(pos)) {
        // Handle system button clicks
        return true;
    }
    
    return false;
}
```

## Window Integration Pattern

### Triggering Animation from Main Window

From `apps/fangjia/MainOpenGlWindow.cpp`:

```cpp
void MainOpenGlWindow::onFollowSystemToggle() const {
    // Set animation flag for next rebuild
    const_cast<MainOpenGlWindow*>(this)->m_animateFollowChange = true;
    
    // Toggle ThemeManager state
    setFollowSystem(!followSystem());
    
    // Next event loop will trigger RebuildHost rebuild and animation
}
```

### Shell Configuration

In the declarative shell builder:

```cpp
auto bar = UI::topBar()->followSystem(followSystem, m_animateFollowChange);
```

**Flow:**
1. User clicks "follow system" button
2. `onFollowSystemToggle()` sets animation flag and toggles ThemeManager
3. Next event loop triggers UI rebuild
4. TopBar receives new `followSystem` state with animation flag
5. Animation sequence begins automatically

## Environment Synchronization

### RebuildHost Order

`UI::RebuildHost::requestRebuild()` synchronizes environment in the correct order to prevent visual artifacts:

1. **Theme Application**: Call `onThemeChanged(isDark)` first
2. **Resource Context**: Update `updateResourceContext(...)` with correct theme
3. **Layout Update**: Call `updateLayout(...)` last

This prevents TopBar from temporarily using incorrect colors or cached icon keys during rebuilds.

### Theme & Resource Context Coordination

```cpp
void RebuildHost::performRebuild() {
    if (!m_component || !m_needsRebuild) return;
    
    // Apply theme first (affects resource cache keys)
    if (m_themeValid) {
        m_component->onThemeChanged(m_isDark);
    }
    
    // Update resources with correct theme context
    if (m_resourceContextValid) {
        m_component->updateResourceContext(*m_iconCache, m_gl, m_devicePixelRatio);
    }
    
    // Layout last (may depend on themed resources)
    if (m_viewportValid) {
        m_component->updateLayout(m_viewport);
    }
    
    m_needsRebuild = false;
}
```

## Animation Performance

### Frame-Rate Independence

Animation uses `QElapsedTimer` for consistent timing:

```cpp
bool UiTopBar::tick() {
    if (m_animPhase == AnimPhase::Idle) return false;
    
    // Calculate elapsed time since phase start
    const int elapsed = m_animClock.elapsed() - m_phaseStartMs;
    const float t = std::clamp(static_cast<float>(elapsed) / m_animDurationMs, 0.0f, 1.0f);
    
    // Apply easing curve
    const float e = easeInOut(t);
    
    // Update animation values based on current phase
    // ... phase-specific animation logic
    
    return m_animPhase != AnimPhase::Idle;
}
```

### Easing Function

Smooth animation curves using cubic easing:

```cpp
float UiTopBar::easeInOut(float t) {
    // Cubic ease-in-out curve
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    } else {
        const float f = 2.0f * t - 2.0f;
        return 1.0f + f * f * f / 2.0f;
    }
}
```

### Memory Efficiency

- Animation state uses minimal memory (a few float values)
- No dynamic allocation during animation
- Reuses existing button structures
- Efficient interpolation calculations

## Related Documentation

- [TopBar Component Overview](top-bar.md)
- [UI Framework Overview](../ui-framework/overview.md)
- [Theme System](../ui-framework/theme-and-rendering.md)
- [Graphics & Rendering](../../infrastructure/gfx.md)