**English** | [简体中文](../../../doc.zh-cn/presentation/components/top-bar.md)

# TopBar Component

## Overview

The TopBar component provides window-level controls including theme switching, system following toggle, and standard window management buttons (minimize, maximize/restore, close). It integrates seamlessly with the application shell and supports smooth animations during state transitions.

## Core Features

- **Theme Management**: Toggle between light and dark themes
- **System Integration**: Follow system theme option with smooth transitions  
- **Window Controls**: Standard minimize, maximize/restore, and close buttons
- **Animation Support**: Smooth two-phase animations for theme/follow interactions
- **Platform Integration**: Native window chrome integration on supported platforms

## Quick Start

### Basic Configuration

```cpp
auto bar = UI::topBar()
    ->followSystem(false)      // Initially don't follow system theme
    ->cornerRadius(6.0f);      // Button corner radius 6px
```

### Advanced Configuration

```cpp
// Custom color palette (optional)
UiTopBar::Palette customPalette;
customPalette.bg = QColor(45, 55, 70, 180);
customPalette.icon = QColor(240, 245, 250);

auto customBar = UI::topBar()
    ->followSystem(true, true)                          // Enable animated "follow system"
    ->cornerRadius(8.0f)                               // 8px corner radius
    ->svgTheme(":/icons/sun.svg", ":/icons/moon.svg")  // Theme toggle icons
    ->svgFollow(":/icons/on.svg", ":/icons/off.svg")   // Follow system icons
    ->svgSystem(":/icons/min.svg", ":/icons/max.svg", ":/icons/close.svg") // System buttons
    ->palette(customPalette)                           // Apply custom colors
    ->onThemeToggle([](){ /* theme toggle callback */ })
    ->padding(4, 8);                                   // Widget decorator usage
```

## API Reference

### Configuration Methods

| Method | Parameters | Default | Description |
|--------|------------|---------|-------------|
| `followSystem()` | `bool on, bool animate = false` | `false, false` | Enable/disable system theme following, with optional animation |
| `cornerRadius()` | `float r` | `6.0f` | Set button corner radius |
| `svgTheme()` | `QString sunWhenDark, QString moonWhenLight` | - | Set theme toggle icons (dark/light themes) |
| `svgFollow()` | `QString on, QString off` | - | Set "follow system" icons (on/off states) |
| `svgSystem()` | `QString min, QString max, QString close` | - | Set system window button icons |
| `palette()` | `const UiTopBar::Palette&` | - | Override default color scheme |

### Event Callbacks

| Method | Parameters | Description |
|--------|------------|-------------|
| `onThemeToggle()` | `std::function<void()>` | Callback when user clicks theme toggle |
| `onFollowToggle()` | `std::function<void()>` | Callback when user clicks "follow system" |
| `onMinimize()` | `std::function<void()>` | Callback for minimize button |
| `onMaxRestore()` | `std::function<void()>` | Callback for maximize/restore button |
| `onClose()` | `std::function<void()>` | Callback for close button |

### Color Palette Structure

```cpp
struct UiTopBar::Palette {
    QColor bg;              // Background color
    QColor icon;            // Icon color
    QColor iconHover;       // Icon hover color
    QColor iconPressed;     // Icon pressed color
    QColor iconDisabled;    // Icon disabled color
    QColor buttonBg;        // Button background
    QColor buttonHover;     // Button hover background
    QColor buttonPressed;   // Button pressed background
};
```

## AppShell Integration

The TopBar integrates seamlessly with the AppShell system (from `apps/fangjia/MainOpenGlWindow.cpp`):

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
    ->topBarHeight(52);
```

### Integration Details

- `followSystem(followSystem, animateNow)`: Current mode from ThemeManager, with `m_animateFollowChange` set by window when "follow system" is clicked to trigger animation
- System button callbacks are handled at the window level
- TopBar height is configurable through AppShell

## Theme & Resource Context Management

### Theme Propagation

Theme changes are distributed through `UiRoot::propagateThemeChange(isDark)` across the entire component tree.

### RebuildHost Environment Synchronization

For declarative UI subtrees, `UI::RebuildHost` synchronizes environment in the correct order during `requestRebuild()`:

1. **Viewport**: Set component viewport if available
2. **Theme**: Call `onThemeChanged(isDark)` if theme state is available  
3. **Resources**: Update resource context `updateResourceContext(...)` (prevents theme mismatch causing icon/color flashing)
4. **Layout**: Call `updateLayout(...)` last

This matches the implementation in `RebuildHost.h` and prevents TopBar from temporarily using incorrect colors during rebuilds.

## Component Architecture

### UiTopBar Implementation

The underlying `UiTopBar` component provides the core functionality:

```cpp
class UiTopBar : public IUiComponent {
public:
    // Configuration
    void setFollowSystem(bool follow);
    void setCornerRadius(float radius);
    void setCustomPalette(const Palette& palette);
    
    // Icon configuration
    void setThemeIcons(const QString& sunIcon, const QString& moonIcon);
    void setFollowIcons(const QString& onIcon, const QString& offIcon);
    void setSystemIcons(const QString& minIcon, const QString& maxIcon, const QString& closeIcon);
    
    // Animation control
    void startAnimSequence(bool followOn);
    bool isAnimating() const;
    
    // Event handling
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    
    // Rendering
    void append(Render::FrameData& frameData) const override;
    bool tick() override;

private:
    // Button states
    Ui::Button m_btnTheme;
    Ui::Button m_btnFollow;
    Ui::Button m_btnMin;
    Ui::Button m_btnMax;
    Ui::Button m_btnClose;
    
    // Animation state
    AnimPhase m_animPhase = AnimPhase::Idle;
    QElapsedTimer m_animClock;
    float m_themeAlpha = 1.0f;
    float m_followSlide = 0.0f;
    
    // Configuration
    bool m_followSystem = false;
    float m_cornerRadius = 6.0f;
    Palette m_palette;
};
```

### Button Management

Each TopBar button is implemented as a `Ui::Button` with individual state tracking:

```cpp
struct Ui::Button {
    QRect baseRect;        // Base position and size
    QPointF offset;        // Animation offset
    float opacity = 1.0f;  // Current opacity
    bool isHovered = false;
    bool isPressed = false;
    bool isEnabled = true;
    
    // Hit testing
    bool contains(const QPoint& point) const {
        return QRectF(baseRect).translated(offset).contains(point);
    }
    
    // Animation support
    void setOffset(const QPointF& newOffset) { offset = newOffset; }
    void setOpacity(float newOpacity) { opacity = qBound(0.0f, newOpacity, 1.0f); }
    void setEnabled(bool enabled) { isEnabled = enabled; }
};
```

## Performance Considerations

### Animation Optimization

- Animations use `QElapsedTimer` for frame-rate independent timing
- Duration scaling via `scaleDuration()` for consistent feel across different hardware
- Easing functions (`easeInOut()`) provide smooth motion curves
- Animation state machine prevents conflicting animations

### Resource Management

- Icons are cached in `IconCache` with theme-aware keys
- Button textures are shared across similar components
- Layout calculations are cached until next update
- Mouse hit testing uses efficient rectangle operations

### Memory Usage

- Component uses fixed-size button array (no dynamic allocation during animations)
- Animation state is minimal (a few float values)
- Resource references rather than copies for icon data

## Related Documentation

- [TopBar Animation Details](top-bar-animation.md)
- [UI Framework Overview](../ui-framework/overview.md)
- [Theme System](../ui-framework/theme-and-rendering.md)
- [AppShell Integration](../../application/app-shell.md)