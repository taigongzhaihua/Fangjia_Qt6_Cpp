# Follow System Access Violation Fix - Summary

## Problem Fixed
- **Issue**: Clicking "Follow system" button in Debug mode caused access violation with `ch.component is 0xFFFFFFFFFFFFFFFF` in `UiGrid::tick()`
- **Root Cause**: Synchronous `requestRebuild()` calls during mouse event handling caused re-entrant destruction of UI tree, leading to use-after-free

## Solution Implemented
Deferred rebuild calls to next event loop iteration using `QTimer::singleShot(0, ...)` to prevent re-entrant destruction.

## Changes Made

### 1. `apps/fangjia/MainOpenGlWindow.cpp` - `onFollowSystemToggle()`
**Before (Problematic):**
```cpp
void MainOpenGlWindow::onFollowSystemToggle() const {
    // ... set animation flag ...
    setFollowSystem(/*...*/);
    
    // PROBLEMATIC: Synchronous rebuild during event handling
    if (m_shellRebuildHost) {
        m_shellRebuildHost->requestRebuild();
    }
    // ... animation setup ...
}
```

**After (Fixed):**
```cpp
void MainOpenGlWindow::onFollowSystemToggle() const {
    // ... set animation flag ...
    setFollowSystem(/*...*/);
    
    // FIXED: Defer rebuild to next event loop turn
    auto* self = const_cast<MainOpenGlWindow*>(this);
    QTimer::singleShot(0, self, [self]() {
        if (self->m_shellRebuildHost) {
            self->m_shellRebuildHost->requestRebuild();
        }
        // ... animation setup ...
    });
}
```

### 2. `apps/fangjia/MainOpenGlWindow.cpp` - `setupThemeListeners()`
**Before (Problematic):**
```cpp
connect(m_themeMgr.get(), &ThemeManager::modeChanged, this,
    [this](const ThemeManager::ThemeMode mode) {
        // PROBLEMATIC: Synchronous rebuild in signal handler
        if (m_shellRebuildHost) {
            m_shellRebuildHost->requestRebuild();
        }
        // ...
    });
```

**After (Fixed):**
```cpp
connect(m_themeMgr.get(), &ThemeManager::modeChanged, this,
    [this](const ThemeManager::ThemeMode mode) {
        // FIXED: Defer rebuild to next event loop turn
        QTimer::singleShot(0, this, [this]() {
            if (m_shellRebuildHost) {
                m_shellRebuildHost->requestRebuild();
            }
            // ...
        });
    });
```

## Call Chain That Caused the Issue
1. User clicks "Follow system" button
2. `mouseReleaseEvent()` → `m_uiRoot.onMouseRelease()`
3. Event propagates through UI tree to button
4. Button callback → `onFollowSystemToggle()`
5. **PROBLEMATIC**: `requestRebuild()` called synchronously
6. `RebuildHost::requestRebuild()` destroys current UI tree (`m_child`)
7. **CRASH**: Control returns to original `onMouseRelease()` with dangling pointers

## How the Fix Works
1. User clicks "Follow system" button
2. `mouseReleaseEvent()` → `m_uiRoot.onMouseRelease()`
3. Event propagates through UI tree to button
4. Button callback → `onFollowSystemToggle()`
5. **FIXED**: `QTimer::singleShot(0, ...)` schedules rebuild for next event loop
6. `onMouseRelease()` completes safely with original UI tree intact
7. Next event loop iteration: Rebuild executes safely

## Validation
- **Unit Tests**: All existing tests pass
- **Animation Tests**: Follow system animation functionality preserved
- **Concept Tests**: Created `test_deferred_rebuild_simple.cpp` to validate approach
- **Build Tests**: Project builds without warnings
- **Regression Tests**: No existing functionality broken

## Benefits
- **Safety**: Eliminates access violation crashes
- **Compatibility**: Preserves existing animation behavior
- **Performance**: Minimal overhead (single event loop deferral)
- **Maintainability**: Clear intent with descriptive comments

## Files Modified
- `apps/fangjia/MainOpenGlWindow.cpp` - Core fix implementation
- `tests/test_deferred_rebuild_simple.cpp` - Validation test (new)
- `tests/manual_follow_system_test.cpp` - Manual verification test (new)