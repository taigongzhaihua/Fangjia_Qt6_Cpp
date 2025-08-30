# Button Focus and Keyboard Support - Demo and Documentation

## Overview

This implements PR4 which enhances the Button control with focus, keyboard interaction, and infrastructure for keyboard navigation. 

## New Features

### 1. Focus and Keyboard Infrastructure

**New Interfaces:**
- `IFocusable`: Interface for components that can receive focus
- `IKeyInput`: Interface for components that can handle keyboard input

**Focus Management:**
- `UiRoot` now manages focus state across all components  
- Clicking a focusable component automatically sets focus
- Only one component can have focus at a time

**Keyboard Event Forwarding:**
- `MainOpenGlWindow` now handles `keyPressEvent` and `keyReleaseEvent`
- Key events are forwarded to `UiRoot` which routes them to the focused component

### 2. Enhanced Button Features

**New Destructive Variant:**
```cpp
// Runtime usage
UiPushButton btn;
btn.setVariant(UiPushButton::Variant::Destructive);

// Declarative usage  
UI::button("删除")->destructive()->onTap([]{ /* delete action */ })
```

**Focus and Keyboard Support:**
- Buttons can now receive focus (implements `IFocusable`)
- Space and Enter keys activate focused buttons (implements `IKeyInput`)  
- Visual focus ring appears around focused buttons
- Focus is automatically set when clicking buttons

**Focus Ring Rendering:**
- Semi-transparent blue ring appears around focused buttons
- Ring color adapts to light/dark theme
- Only visible when button is focused and enabled

### 3. Declarative API Enhancement

**New Methods:**
```cpp
// All button variants now supported
UI::button("保存")->primary()
UI::button("取消")->secondary()  
UI::button("链接")->ghost()
UI::button("删除")->destructive()  // NEW

// All can be keyboard activated when focused
```

## Demo Usage

```cpp
// Create buttons with different variants
auto saveBtn = UI::button("保存")->primary()->onTap([]{ qDebug() << "Save clicked!"; });
auto cancelBtn = UI::button("取消")->secondary()->onTap([]{ qDebug() << "Cancel clicked!"; });
auto linkBtn = UI::button("链接")->ghost()->onTap([]{ qDebug() << "Link clicked!"; });
auto deleteBtn = UI::button("删除")->destructive()->onTap([]{ qDebug() << "Delete clicked!"; });

// All buttons support:
// - Mouse click activation
// - Keyboard focus (click to focus)
// - Keyboard activation (Space/Enter when focused)
// - Visual focus ring when focused
// - Theme-aware colors
```

## Keyboard Navigation

1. **Click any button** to give it focus (focus ring appears)
2. **Press Space or Enter** to activate the focused button  
3. **Click another button** to move focus
4. **Focus ring** provides visual feedback for current focus

## Implementation Details

### Files Created:
- `presentation/ui/base/IFocusable.hpp` - Focus management interface
- `presentation/ui/base/IKeyInput.hpp` - Keyboard input interface

### Files Enhanced:
- `UiPushButton.h/cpp` - Added focus, keyboard support, Destructive variant
- `BasicWidgets_Button.h/cpp` - Added destructive() API and Destructive variant  
- `UiRoot.h/cpp` - Added focus management and key event forwarding
- `MainOpenGlWindow.h/cpp` - Added keyboard event handlers
- `UiButton.hpp` - Added simulatePress/Release for keyboard activation

### Color Scheme:
**Destructive Variant Colors:**
- Light theme: Red (#D23232) with white text
- Dark theme: Slightly brighter red (#DC3C3C) with white text
- Proper hover/pressed state variations

**Focus Ring:**
- Light theme: Blue (#4682FF) with 120 alpha  
- Dark theme: Lighter blue (#78AAFF) with 120 alpha
- 2px width, appears outside button bounds