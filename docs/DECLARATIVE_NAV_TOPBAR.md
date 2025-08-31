# Declarative NavRail and TopBar Widgets

This document describes the new declarative UI widgets for NavRail and TopBar components that provide first-class declarative APIs without requiring runtime component wrapping via `UI::wrap()`.

## Overview

The declarative widgets maintain consistency with existing `Ui::NavRail` and `UiTopBar` components while providing a modern, chainable API for configuration.

### Key Features

- **Chainable API**: Fluent interface for configuration
- **Consistent Behavior**: Identical styling and behavior to runtime components
- **Lifecycle Management**: Automatic forwarding of events and theme changes
- **Decorator Support**: Full integration with Widget base class decorators (padding, margin, etc.)

## NavRail Widget

The `UI::NavRail` widget provides a declarative interface to the navigation rail component.

### Basic Usage

```cpp
#include "UI.h"

auto rail = UI::navRail()
    ->widths(48, 200)          // collapsed: 48px, expanded: 200px
    ->iconSize(22)             // icon size in logical pixels
    ->itemHeight(48)           // navigation item height
    ->labelFontPx(13);         // label font size
```

### Advanced Configuration

```cpp
// Custom palette
Ui::NavPalette customPalette;
customPalette.railBg = QColor(30, 35, 40, 200);
customPalette.itemSelected = QColor(0, 120, 255, 180);
customPalette.iconColor = QColor(220, 225, 230);

auto customRail = UI::navRail()
    ->dataProvider(myNavDataProvider)                    // bind data source
    ->widths(64, 220)                                   // custom dimensions
    ->iconSize(24)                                      // larger icons
    ->toggleSvg(":/icons/expand.svg", ":/icons/collapse.svg")  // custom toggle icons
    ->palette(customPalette)                            // apply custom colors
    ->padding(8)                                        // decorator support
    ->margin(4, 0, 0, 0);                              // left margin
```

### API Reference

| Method | Parameters | Default | Description |
|--------|------------|---------|-------------|
| `dataProvider()` | `INavDataProvider*` | `nullptr` | Bind navigation data provider |
| `widths()` | `int collapsed, int expanded` | `48, 200` | Set collapsed and expanded widths |
| `iconSize()` | `int logicalPx` | `22` | Set icon size in logical pixels |
| `itemHeight()` | `int px` | `48` | Set navigation item height |
| `labelFontPx()` | `int px` | `13` | Set label font size |
| `toggleSvg()` | `QString expand, QString collapse` | - | Set expand/collapse button icons |
| `palette()` | `const Ui::NavPalette&` | - | Override default color scheme |

## TopBar Widget

The `UI::TopBar` widget provides a declarative interface to the top bar component.

### Basic Usage

```cpp
auto bar = UI::topBar()
    ->followSystem(false)      // don't follow system theme initially
    ->cornerRadius(6.0f);      // 6px corner radius for buttons
```

### Advanced Configuration

```cpp
// Custom palette
UiTopBar::Palette customPalette;
customPalette.bg = QColor(45, 55, 70, 180);
customPalette.icon = QColor(240, 245, 250);

auto customBar = UI::topBar()
    ->followSystem(true, true)                          // follow system with animation
    ->cornerRadius(8.0f)                               // 8px corner radius
    ->svgTheme(":/icons/sun.svg", ":/icons/moon.svg")  // theme toggle icons
    ->svgFollow(":/icons/on.svg", ":/icons/off.svg")   // follow system icons
    ->svgSystem(":/icons/min.svg", ":/icons/max.svg", ":/icons/close.svg")  // window controls
    ->palette(customPalette)                           // apply custom colors
    ->onThemeToggle([]() {                             // theme change callback
        // Handle theme toggle logic
    })
    ->padding(4, 8);                                   // decorator support
```

### API Reference

| Method | Parameters | Default | Description |
|--------|------------|---------|-------------|
| `followSystem()` | `bool on, bool animate = false` | `false, false` | Set system theme following with optional animation |
| `cornerRadius()` | `float r` | `6.0f` | Set button corner radius |
| `svgTheme()` | `QString sunWhenDark, QString moonWhenLight` | - | Set theme toggle icons |
| `svgFollow()` | `QString on, QString off` | - | Set follow system icons |
| `svgSystem()` | `QString min, QString max, QString close` | - | Set window control icons |
| `palette()` | `const UiTopBar::Palette&` | - | Override default color scheme |
| `onThemeToggle()` | `std::function<void()>` | - | Set theme change callback |
| `onFollowToggle()` | `std::function<void()>` | - | Set follow system toggle callback |

### Follow System Animation

The TopBar component includes sophisticated two-phase animation for the Follow System toggle:

#### Animation Behavior

**Enabling Follow System (when `followSystem(true, true)`):**
1. **Phase 1**: Theme button fades out (160ms with ease-in-out curve)
2. **Phase 2**: Follow button slides right into theme button's position (200ms)
3. Theme button becomes non-interactive while hidden

**Disabling Follow System (when `followSystem(false, true)`):**
1. **Phase 1**: Follow button slides left back to original position (180ms)
2. **Phase 2**: Theme button fades in to full opacity (160ms)
3. Theme button becomes interactive again

#### Usage with Animation

```cpp
// Enable follow system with animation
auto bar = UI::topBar()
    ->followSystem(true, true)   // animate = true triggers animation
    ->onFollowToggle([this]() {
        // This callback is triggered by user clicks
        // MainOpenGlWindow sets animation flag before calling setFollowSystem
    });
```

#### Integration Pattern

The typical integration pattern in MainOpenGlWindow:

```cpp
// 1. User clicks Follow button -> onFollowSystemToggle() called
void MainOpenGlWindow::onFollowSystemToggle() const {
    // Set animation flag before changing theme mode
    const_cast<MainOpenGlWindow*>(this)->m_animateFollowChange = true;
    setFollowSystem(!followSystem());
}

// 2. Shell rebuilds with animation flag
->followSystem(followSystem, m_animateFollowChange)  // Uses animation flag

// 3. Flag is reset after rebuild
m_animateFollowChange = false;
```

#### Technical Details

- **Timing Values**: Subtle timing (160-200ms) for smooth user experience
- **Easing**: Smooth ease-in-out curve for natural motion
- **Interactivity**: Theme button disabled while Follow System is active (except during fade-in)
- **State Management**: Explicit animation state machine prevents conflicts

## Integration with AppShell

Both widgets integrate seamlessly with the existing AppShell component:

```cpp
auto app = UI::appShell()
    ->navRail(UI::navRail()->widths(64, 220)->iconSize(24))
    ->topBar(UI::topBar()->followSystem(true)->cornerRadius(8.0f))
    ->content(myMainContent);
```

## Decorator Support

Both widgets inherit from the Widget base class and support all standard decorators:

```cpp
auto decoratedRail = UI::navRail()
    ->widths(48, 200)
    ->padding(8)                           // inner padding
    ->margin(4, 0, 0, 0)                  // left margin
    ->background(QColor(0, 0, 0, 50))     // semi-transparent background
    ->border(QColor(100, 100, 100), 1.0f); // border
```

## Factory Functions

Convenience factory functions are available in the `UI` namespace:

```cpp
using namespace UI;

auto nav = navRail();     // equivalent to make_widget<NavRail>()
auto top = topBar();      // equivalent to make_widget<TopBar>()
```

## Notes

- These widgets maintain the same behavior and styling as the existing `Ui::NavRail` and `UiTopBar` components
- Theme changes are automatically forwarded to the wrapped runtime components
- All mouse events and lifecycle methods are properly delegated
- The wrapper components own the runtime instances and manage their lifecycle
- MainOpenGlWindow integration is not changed in this implementation to avoid animation/WinWindowChrome coupling issues