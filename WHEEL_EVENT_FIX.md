# Wheel Event Forwarding Fix

## Problem
Mouse wheel scrolling was not working in the "数据页" → "方剂详情" area, while it worked fine in the homepage ScrollView.

## Root Cause
The problem was caused by missing `onWheel` event forwarding in two key wrapper components:

1. **ComponentWrapper::ProxyComponent** - Used by `UI::wrap()` to inject content into UiTabView
2. **DecoratedBox** - Used for styling/decorating components with cards/borders

When wheel events reached these wrapper components, they were discarded instead of being forwarded to the underlying ScrollView components.

## Solution
Added `onWheel` method implementations to both wrapper components:

### ComponentWrapper::ProxyComponent
```cpp
bool onWheel(const QPoint& pos, const QPoint& angleDelta) override { 
    return m_wrapped ? m_wrapped->onWheel(pos, angleDelta) : false; 
}
```

Simple forwarding that passes wheel events directly to the wrapped component, consistent with other mouse event handlers.

### DecoratedBox
```cpp
bool onWheel(const QPoint& pos, const QPoint& angleDelta) override
{
    if (!m_p.visible || !m_viewport.contains(pos)) return false;
    return m_child ? m_child->onWheel(pos, angleDelta) : false;
}
```

Forwarding with validation - checks visibility and viewport containment before forwarding to child, following the same pattern as other mouse events.

## Files Changed
- `src/framework/declarative/ComponentWrapper.cpp` - Added onWheel forwarding
- `src/framework/declarative/Decorators.h` - Added onWheel declaration  
- `src/framework/declarative/Decorators.cpp` - Added onWheel implementation
- `tests/framework/test_wrapper_wheel_forwarding.cpp` - Added comprehensive tests

## Testing
Created specific test cases to verify:
- Normal wheel event forwarding through both wrapper types
- Edge cases: events outside viewport, invisible components
- Proper parameter passing (position, angle delta)

## Impact
This fix enables wheel scrolling in any UI content that uses:
- Tab content injected via `UI::wrap(formulaView.get())`
- Content wrapped in DecoratedBox for styling/cards  
- Any other components using these wrapper patterns

The fix is minimal and surgical - it only adds the missing event forwarding without changing any existing behavior.