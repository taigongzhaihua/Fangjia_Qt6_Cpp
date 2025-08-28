# UiScrollView Scrollbar & Mouse Wheel Fix - Visual Reference

## Problem Statement Summary

The UiScrollView component had two main issues:
1. **Scrollbar visibility problems**: Not showing or showing unstably, lacking proper semi-transparent/hover feedback
2. **Mouse wheel scrolling not working**: Wheel events weren't being forwarded from UiPage to content area

## Visual Behavior Changes

### Before Fix
```
┌─────────────────────┐
│ Page Title Area     │
├─────────────────────┤
│                     │
│ Scrollable Content  │  ← Mouse wheel events not working
│                     │
│ Content continues   │  ← No visible scrollbar
│ beyond viewport...  │     or unstable appearance
│                     │
└─────────────────────┘
```

### After Fix  
```
┌─────────────────────┐
│ Page Title Area     │  ← Wheel events here ignored (correct)
├─────────────────────┤
│                     │
│ Scrollable Content  │  ← Mouse wheel works here ✓
│                     │
│ Content continues   │  ░ ← Semi-transparent scrollbar (30%)
│ beyond viewport...  │  ░   visible when content overflows
│                     │  ░
└─────────────────────┘
```

### On Interaction (Hover/Wheel/Drag)
```
┌─────────────────────┐
│ Page Title Area     │
├─────────────────────┤
│                     │
│ Scrollable Content  │  ← Smooth wheel scrolling
│                     │
│ Content continues   │  █ ← Opaque scrollbar (100%)
│ beyond viewport...  │  █   during interaction
│                     │  █
└─────────────────────┘
```

### Animation Timeline
```
Interaction Event (wheel/hover/drag)
    ↓
Scrollbar: 30% → 100% opacity (immediate)
    ↓
900ms delay (idle)
    ↓
Fade out: 100% → 30% opacity (300ms duration)
```

## Technical Implementation Details

### 1. Event Flow for Mouse Wheel
```
Mouse Wheel Event
    ↓
UiPage::onWheel(pos, angleDelta)
    ↓
Check: pos within contentRectF()?
    ↓ YES
Forward to content->onWheel(pos, angleDelta)
    ↓
UiScrollView::onWheel(pos, angleDelta)
    ↓
Check: pos within bounds() && maxScrollY() > 0?
    ↓ YES
Calculate scroll delta: -(angleDelta.y() * 48) / 120
Apply scroll: setScrollY(m_scrollY + scrollDelta)
Show scrollbar: m_thumbAlpha = 1.0f, start animation
Return: true (event consumed)
```

### 2. Scrollbar Alpha Animation States
```
State: IDLE
├─ m_thumbAlpha = BASE_ALPHA (0.3f)
├─ m_animActive = false
└─ Scrollbar visible if content overflows

State: INTERACTING  
├─ m_thumbAlpha = 1.0f
├─ m_animActive = true
├─ m_lastInteractMs = current time
└─ Scrollbar fully opaque

State: FADE_DELAY (900ms)
├─ m_thumbAlpha = 1.0f (maintained)
├─ m_animActive = true
└─ Waiting to start fade

State: FADING (300ms)
├─ m_thumbAlpha = 1.0 - fadeProgress * (1.0 - BASE_ALPHA)
├─ m_animActive = true
└─ Interpolating to BASE_ALPHA

State: FADE_COMPLETE
├─ m_thumbAlpha = BASE_ALPHA (0.3f)
├─ m_animActive = false
└─ Back to idle state
```

### 3. Theme Colors (Fluent Design)

#### Light Theme
```
Track Color:      rgba(0, 0, 0, 25)     # Very subtle track
Thumb Color:      rgba(0, 0, 0, 120)    # Visible but not intrusive  
Thumb Hover:      rgba(0, 0, 0, 160)    # More prominent on hover
Thumb Press:      rgba(0, 0, 0, 200)    # Clearly visible when pressed
```

#### Dark Theme  
```
Track Color:      rgba(255, 255, 255, 25)   # Very subtle track
Thumb Color:      rgba(255, 255, 255, 120)  # Visible but not intrusive
Thumb Hover:      rgba(255, 255, 255, 160)  # More prominent on hover  
Thumb Press:      rgba(255, 255, 255, 200)  # Clearly visible when pressed
```

### 4. Scrollbar Dimensions
```
Width: 6px (SCROLLBAR_WIDTH)
Minimum thumb height: 20px (THUMB_MIN_HEIGHT)
Corner radius: 3px (THUMB_RADIUS) - Fluent Design rounded corners
Thumb height: Proportional to (viewport_height / content_height)
```

## Files Modified

### UiPage.h
- Added `bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;`

### UiPage.cpp  
- Implemented onWheel() method that forwards events to content area when within contentRectF()

### UiScrollView.h
- Added `static constexpr float BASE_ALPHA = 0.3f;` for semi-transparent base state

### UiScrollView.cpp
- Constructor: Initialize m_thumbAlpha to BASE_ALPHA instead of 0.0f
- tick(): Fade from 1.0 to BASE_ALPHA instead of 0.0 (completely invisible)
- onMouseMove(): Trigger showScrollbar() when hovering over scrollbar area

## Testing Strategy

### Automated Tests Added
1. **UiScrollView wheel event bounds checking**
2. **UiScrollView scroll consumption logic**  
3. **UiScrollView animation state management**
4. **UiPage wheel event forwarding boundaries**

### Manual Testing Points
1. **Functional**: Wheel scrolling works in all scrollable areas
2. **Visual**: Scrollbar appears semi-transparent, becomes opaque on interaction
3. **Animation**: Smooth fade transitions with proper timing
4. **Theme**: Colors update correctly for light/dark modes
5. **Edge cases**: Events outside content area don't trigger scrolling

## Success Criteria

✅ **Mouse wheel scrolling works** in pages with ScrollView content  
✅ **Scrollbar is always visible** when content overflows (30% opacity minimum)  
✅ **Scrollbar becomes prominent** during interaction (100% opacity)  
✅ **Smooth animations** with proper timing (900ms delay, 300ms fade)  
✅ **Proper event boundaries** (wheel events outside content area ignored)  
✅ **Theme compatibility** (appropriate colors for light/dark modes)  
✅ **No regression** in existing scrollbar drag/click functionality