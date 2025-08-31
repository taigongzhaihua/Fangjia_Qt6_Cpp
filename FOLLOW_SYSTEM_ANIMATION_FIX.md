# Follow System Animation Fix - Implementation Summary

## Problem
The "Follow System" button in the TopBar was no longer playing its original two-step animation when clicked. The animation was disabled because the declarative TopBar builder always passed `animate=false` to the `followSystem()` method.

## Root Cause
- After switching to declarative AppShell/TopBar, the builder lambda in `initializeDeclarativeShell()` always called `followSystem(followSystem, false)` 
- The `false` parameter disabled animation, even for user-initiated clicks
- The underlying `UiTopBar` still supported the animation via `setFollowSystem(bool on, bool animate)`

## Solution
Added a transient animation flag that tracks user-initiated Follow System toggles:

### Changes Made

1. **MainOpenGlWindow.h** - Added private member:
   ```cpp
   bool m_animateFollowChange{ false };
   ```

2. **MainOpenGlWindow.cpp** - Updated `onFollowSystemToggle()`:
   ```cpp
   void MainOpenGlWindow::onFollowSystemToggle() const
   {
       if (!m_themeMgr) return;
       
       // Set animation flag before changing theme mode
       const_cast<MainOpenGlWindow*>(this)->m_animateFollowChange = true;
       
       setFollowSystem(m_themeMgr->mode() != ThemeManager::ThemeMode::FollowSystem);
   }
   ```

3. **MainOpenGlWindow.cpp** - Updated declarative shell builder:
   ```cpp
   ->followSystem(followSystem, m_animateFollowChange) // Use animation flag
   ```

4. **MainOpenGlWindow.cpp** - Updated theme change handler to reset flag:
   ```cpp
   if (m_shellRebuildHost)
   {
       m_shellRebuildHost->requestRebuild();
       // Reset animation flag after rebuild
       m_animateFollowChange = false;
   }
   ```

## How It Works

1. **User clicks Follow System button** → `onFollowSystemToggle()` called
2. **Flag is set to true** → `m_animateFollowChange = true`
3. **Theme mode changes** → `ThemeManager::modeChanged` signal emitted
4. **Shell rebuilds** → Builder lambda uses `m_animateFollowChange = true`
5. **TopBar created with animation** → `setFollowSystem(followSystem, true)` enables animation
6. **Flag is reset** → `m_animateFollowChange = false` after rebuild

## Animation Behavior

The restored animation has two phases:

**Enabling Follow System:**
1. Theme button fades out (160ms)
2. Follow button slides right into theme position (200ms)

**Disabling Follow System:**
1. Follow button slides left to original position (180ms)  
2. Theme button fades in (160ms)

## Why This Works

- **Synchronous rebuild**: `RebuildHost::requestRebuild()` calls the builder immediately, so the flag is consumed during the same call
- **Single-use flag**: Resetting the flag right after rebuild ensures animation only plays once per user action
- **Startup preservation**: Initial shell construction still uses `animate=false` (flag starts false)
- **Non-user changes**: Other theme changes don't set the flag, so they remain non-animated

## Tests Added

- `test_follow_system_animation.cpp` - Validates flag lifecycle logic
- `test_nav_topbar_widgets.cpp` - Updated to test animation parameter

## Files Modified

- `apps/fangjia/MainOpenGlWindow.h`
- `apps/fangjia/MainOpenGlWindow.cpp`  
- `tests/test_nav_topbar_widgets.cpp`
- `tests/test_follow_system_animation.cpp` (new)