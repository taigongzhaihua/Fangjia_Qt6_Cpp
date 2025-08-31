**English** | [简体中文](../../../../doc.zh-cn/presentation/ui/topbar/animation.md)

# TopBar Follow System Animation

This document explains the two-phase animation behavior and interaction handling when toggling the "follow system" setting in TopBar, along with typical integration patterns.

## Animation Behavior

**Enable Follow (`followSystem(true, true)`):**
1. **Phase One**: Theme button fades out (160ms, easeInOut)
2. **Phase Two**: Follow button fades in (160ms, easeInOut, starts after 40ms delay)

**Disable Follow (`followSystem(false, true)`):**
1. **Phase One**: Follow button fades out (160ms, easeInOut)
2. **Phase Two**: Theme button fades in (160ms, easeInOut, starts after 40ms delay)

## Animation Implementation

### Animation State Management

```cpp
class TopBarAnimationController {
public:
    struct AnimationState {
        bool isAnimating = false;
        bool targetFollowSystem = false;
        float themeButtonAlpha = 1.0f;
        float followButtonAlpha = 0.0f;
        qint64 animationStartTime = 0;
    };
    
    void startFollowToggleAnimation(bool enable) {
        m_state.isAnimating = true;
        m_state.targetFollowSystem = enable;
        m_state.animationStartTime = QDateTime::currentMSecsSinceEpoch();
        
        if (enable) {
            // Start with theme button visible, follow button hidden
            m_state.themeButtonAlpha = 1.0f;
            m_state.followButtonAlpha = 0.0f;
        } else {
            // Start with follow button visible, theme button hidden
            m_state.themeButtonAlpha = 0.0f;
            m_state.followButtonAlpha = 1.0f;
        }
    }
    
    void updateAnimation() {
        if (!m_state.isAnimating) return;
        
        qint64 elapsed = QDateTime::currentMSecsSinceEpoch() - m_state.animationStartTime;
        
        if (m_state.targetFollowSystem) {
            updateEnableFollowAnimation(elapsed);
        } else {
            updateDisableFollowAnimation(elapsed);
        }
        
        // Check if animation is complete
        if (elapsed >= TOTAL_ANIMATION_DURATION) {
            completeAnimation();
        }
    }
    
private:
    static constexpr int FADE_DURATION = 160;
    static constexpr int PHASE_DELAY = 40;
    static constexpr int TOTAL_ANIMATION_DURATION = FADE_DURATION + PHASE_DELAY + FADE_DURATION;
    
    void updateEnableFollowAnimation(qint64 elapsed) {
        if (elapsed < FADE_DURATION) {
            // Phase 1: Fade out theme button
            float progress = float(elapsed) / FADE_DURATION;
            m_state.themeButtonAlpha = 1.0f - easeInOut(progress);
            m_state.followButtonAlpha = 0.0f;
        } else if (elapsed < FADE_DURATION + PHASE_DELAY) {
            // Delay between phases
            m_state.themeButtonAlpha = 0.0f;
            m_state.followButtonAlpha = 0.0f;
        } else {
            // Phase 2: Fade in follow button
            float phaseElapsed = elapsed - FADE_DURATION - PHASE_DELAY;
            float progress = float(phaseElapsed) / FADE_DURATION;
            m_state.themeButtonAlpha = 0.0f;
            m_state.followButtonAlpha = easeInOut(progress);
        }
    }
    
    void updateDisableFollowAnimation(qint64 elapsed) {
        if (elapsed < FADE_DURATION) {
            // Phase 1: Fade out follow button
            float progress = float(elapsed) / FADE_DURATION;
            m_state.followButtonAlpha = 1.0f - easeInOut(progress);
            m_state.themeButtonAlpha = 0.0f;
        } else if (elapsed < FADE_DURATION + PHASE_DELAY) {
            // Delay between phases
            m_state.followButtonAlpha = 0.0f;
            m_state.themeButtonAlpha = 0.0f;
        } else {
            // Phase 2: Fade in theme button
            float phaseElapsed = elapsed - FADE_DURATION - PHASE_DELAY;
            float progress = float(phaseElapsed) / FADE_DURATION;
            m_state.followButtonAlpha = 0.0f;
            m_state.themeButtonAlpha = easeInOut(progress);
        }
    }
    
    void completeAnimation() {
        m_state.isAnimating = false;
        
        if (m_state.targetFollowSystem) {
            m_state.themeButtonAlpha = 0.0f;
            m_state.followButtonAlpha = 1.0f;
        } else {
            m_state.themeButtonAlpha = 1.0f;
            m_state.followButtonAlpha = 0.0f;
        }
    }
    
    static float easeInOut(float t) {
        return t * t * (3.0f - 2.0f * t);
    }
    
    AnimationState m_state;
};
```

### Rendering During Animation

```cpp
void UiTopBar::append(Render::FrameData& fd) const {
    // Render buttons with animation-controlled alpha
    if (m_animationController.getState().themeButtonAlpha > 0.0f) {
        renderThemeButton(fd, m_animationController.getState().themeButtonAlpha);
    }
    
    if (m_animationController.getState().followButtonAlpha > 0.0f) {
        renderFollowButton(fd, m_animationController.getState().followButtonAlpha);
    }
    
    // Render other buttons normally
    renderWindowButtons(fd);
}

void UiTopBar::renderThemeButton(Render::FrameData& fd, float alpha) const {
    if (alpha <= 0.0f) return;
    
    // Calculate button bounds and colors with alpha
    QRectF buttonRect = calculateThemeButtonRect();
    QColor buttonColor = m_hoverStates.themeButton ? m_colors.buttonHover : m_colors.buttonNormal;
    buttonColor.setAlphaF(buttonColor.alphaF() * alpha);
    
    // Render button background
    fd.addRect(buttonRect, buttonColor, m_cornerRadius);
    
    // Render icon with alpha
    if (m_themeIconTextureId > 0) {
        QRectF iconRect = calculateIconRect(buttonRect);
        fd.addImage(iconRect, m_themeIconTextureId, QRectF(), alpha);
    }
}

void UiTopBar::renderFollowButton(Render::FrameData& fd, float alpha) const {
    if (alpha <= 0.0f) return;
    
    // Similar implementation for follow button
    QRectF buttonRect = calculateFollowButtonRect();
    QColor buttonColor = m_hoverStates.followButton ? m_colors.buttonHover : m_colors.buttonNormal;
    buttonColor.setAlphaF(buttonColor.alphaF() * alpha);
    
    fd.addRect(buttonRect, buttonColor, m_cornerRadius);
    
    if (m_followIconTextureId > 0) {
        QRectF iconRect = calculateIconRect(buttonRect);
        fd.addImage(iconRect, m_followIconTextureId, QRectF(), alpha);
    }
}
```

## Interaction Handling During Animation

### Event Filtering

```cpp
class TopBarInteractionFilter {
public:
    bool shouldProcessClick(const QPoint& pos, const TopBarAnimationController& animator) {
        if (!animator.isAnimating()) {
            return true;  // Normal processing when not animating
        }
        
        // During animation, only allow clicks on fully visible buttons
        auto state = animator.getState();
        
        if (isInThemeButtonArea(pos) && state.themeButtonAlpha >= 0.8f) {
            return true;
        }
        
        if (isInFollowButtonArea(pos) && state.followButtonAlpha >= 0.8f) {
            return true;
        }
        
        // Block clicks on window control buttons during follow toggle animation
        // to prevent accidental window operations
        return !isInWindowButtonArea(pos);
    }
    
    bool shouldShowHover(const QPoint& pos, const TopBarAnimationController& animator) {
        if (!animator.isAnimating()) {
            return true;
        }
        
        // Only show hover for sufficiently visible buttons
        auto state = animator.getState();
        
        if (isInThemeButtonArea(pos)) {
            return state.themeButtonAlpha >= 0.5f;
        }
        
        if (isInFollowButtonArea(pos)) {
            return state.followButtonAlpha >= 0.5f;
        }
        
        return true;  // Other areas process hover normally
    }
    
private:
    bool isInThemeButtonArea(const QPoint& pos) const;
    bool isInFollowButtonArea(const QPoint& pos) const;
    bool isInWindowButtonArea(const QPoint& pos) const;
};
```

### Animation-Aware Event Handling

```cpp
bool UiTopBar::handleMousePress(const QPoint& pos) {
    if (!m_interactionFilter.shouldProcessClick(pos, m_animationController)) {
        return false;  // Ignore click during animation
    }
    
    // Determine which button was clicked based on current animation state
    auto state = m_animationController.getState();
    
    if (isInThemeButtonArea(pos) && state.themeButtonAlpha >= 0.8f) {
        onThemeButtonClicked();
        return true;
    }
    
    if (isInFollowButtonArea(pos) && state.followButtonAlpha >= 0.8f) {
        onFollowButtonClicked();
        return true;
    }
    
    // Handle other button clicks
    return handleOtherButtonClicks(pos);
}

bool UiTopBar::handleMouseMove(const QPoint& pos) {
    // Update hover states based on animation visibility
    bool oldThemeHover = m_hoverStates.themeButton;
    bool oldFollowHover = m_hoverStates.followButton;
    
    m_hoverStates.themeButton = isInThemeButtonArea(pos) && 
                               m_interactionFilter.shouldShowHover(pos, m_animationController);
    
    m_hoverStates.followButton = isInFollowButtonArea(pos) && 
                                m_interactionFilter.shouldShowHover(pos, m_animationController);
    
    // Request repaint if hover state changed
    if (oldThemeHover != m_hoverStates.themeButton || 
        oldFollowHover != m_hoverStates.followButton) {
        requestRepaint();
    }
    
    return true;
}
```

## Integration Patterns

### Declarative API Integration

```cpp
auto createAnimatedTopBar() {
    return UI::topBar()
        ->followSystem(m_themeManager->followSystem(), true)  // Enable animations
        ->animationDuration(160)        // Custom fade duration
        ->animationDelay(40)           // Custom phase delay
        ->animationEasing(EasingCurve::InOut)  // Custom easing function
        ->onThemeToggle([this]() {
            if (!m_animationController.isAnimating()) {
                toggleTheme();
            }
        })
        ->onFollowToggle([this]() {
            if (!m_animationController.isAnimating()) {
                toggleFollowSystem();
            }
        });
}
```

### State Management Integration

```cpp
class TopBarViewModel : public INotifyPropertyChanged {
public:
    bool followSystem() const {
        notifyAccess("followSystem");
        return m_followSystem;
    }
    
    bool isAnimating() const {
        notifyAccess("isAnimating");
        return m_animationController.isAnimating();
    }
    
    void setFollowSystem(bool follow, bool animate = true) {
        if (m_followSystem != follow && !m_animationController.isAnimating()) {
            m_followSystem = follow;
            
            if (animate) {
                m_animationController.startFollowToggleAnimation(follow);
                // Start animation update timer
                startAnimationTimer();
            }
            
            notifyChanged("followSystem");
            notifyChanged("isAnimating");
        }
    }
    
private:
    void startAnimationTimer() {
        m_animationTimer = std::make_unique<QTimer>();
        m_animationTimer->setInterval(16);  // ~60 FPS
        
        connect(m_animationTimer.get(), &QTimer::timeout, this, [this]() {
            m_animationController.updateAnimation();
            
            if (!m_animationController.isAnimating()) {
                m_animationTimer->stop();
                notifyChanged("isAnimating");
            }
            
            // Trigger UI update
            notifyChanged("animationState");
        });
        
        m_animationTimer->start();
    }
    
    bool m_followSystem = false;
    TopBarAnimationController m_animationController;
    std::unique_ptr<QTimer> m_animationTimer;
};
```

### Performance Optimization

```cpp
class AnimationOptimizer {
public:
    void optimizeForAnimation(TopBarAnimationController& controller) {
        // Pre-calculate button rectangles to avoid repeated calculations
        cacheButtonRects();
        
        // Pre-load icon textures for both states
        preloadIconTextures();
        
        // Set up efficient rendering pipeline
        setupRenderingOptimizations();
    }
    
private:
    void cacheButtonRects() {
        m_themeButtonRect = calculateThemeButtonRect();
        m_followButtonRect = calculateFollowButtonRect();
    }
    
    void preloadIconTextures() {
        // Ensure both theme and follow icons are loaded
        // This prevents texture loading during animation
        m_iconCache->ensureLoaded("theme_icon");
        m_iconCache->ensureLoaded("follow_icon");
    }
    
    void setupRenderingOptimizations() {
        // Enable GPU-accelerated blending for smooth alpha transitions
        // Configure render state for optimal performance
    }
    
    QRectF m_themeButtonRect;
    QRectF m_followButtonRect;
    IconCache* m_iconCache;
};
```

## Common Usage Patterns

### Simple Animation Setup

```cpp
// Basic setup with default animation parameters
auto topBar = UI::topBar()
    ->followSystem(false, true)  // Start with follow disabled, animations enabled
    ->onFollowToggle([this]() {
        m_themeManager->setFollowSystem(!m_themeManager->followSystem());
    });
```

### Custom Animation Timing

```cpp
// Custom animation with different timing
auto customTopBar = UI::topBar()
    ->followSystem(true, true)
    ->animationDuration(200)      // Slower fade (200ms instead of 160ms)
    ->animationDelay(60)          // Longer delay between phases
    ->onAnimationComplete([this]() {
        // Custom callback when animation finishes
        onTopBarAnimationComplete();
    });
```

### Animation with State Synchronization

```cpp
// Synchronized with external state changes
auto syncedTopBar = UI::topBar()
    ->followSystem(m_appState->followSystem(), true)
    ->onFollowToggle([this]() {
        // Update application state, which will trigger UI rebuild
        m_appState->toggleFollowSystem();
    });

// In the state change handler:
void onAppStateChanged() {
    if (m_topBar->followSystem() != m_appState->followSystem()) {
        m_topBar->setFollowSystem(m_appState->followSystem(), true);
    }
}
```

## Related Documentation

- [Declarative TopBar Component](declarative-topbar.md) - Full TopBar component API
- [TopBar Component](../../../components/top-bar.md) - Core TopBar functionality
- [Theme & Rendering](../../ui-framework/theme-and-rendering.md) - Theme system integration