# Manual Verification Guide

## TopBar Theme Flicker Fix Verification

This document outlines how to manually verify that the TopBar theme flicker issue has been resolved.

### Issue Description
- **Problem**: On light theme, clicking Nav expand button, selecting a Nav item, or double-clicking Nav causes the TopBar to turn dark unexpectedly
- **Root Cause**: RebuildHost applied theme changes after resource context updates, causing components to use default theme during palette/icon selection
- **Fix**: Reordered operations in `RebuildHost::requestRebuild()` to apply theme before updating resource context

### Manual Testing Steps

#### Prerequisites
1. Build the application with Qt6 
2. Start the application in light theme mode
3. Ensure the TopBar appears in light theme (light colors, appropriate icons)

#### Test Cases

**Test Case 1: Nav Expand/Collapse**
1. Click the Nav expand button
2. **Expected**: TopBar remains in light theme throughout the interaction
3. **Previous behavior**: TopBar would briefly flash dark before returning to light
4. Click Nav collapse button  
5. **Expected**: TopBar remains in light theme

**Test Case 2: Nav Item Selection**
1. With Nav expanded, click on different Nav items
2. **Expected**: TopBar remains in light theme during all selections
3. **Previous behavior**: TopBar would flash dark on each selection

**Test Case 3: Nav Double-Click**
1. Double-click on the Nav area
2. **Expected**: TopBar remains in light theme
3. **Previous behavior**: TopBar would flash dark

**Test Case 4: Theme Toggle Verification**
1. Click the theme toggle button in TopBar
2. **Expected**: TopBar smoothly transitions to dark theme with animations
3. Switch back to light theme
4. **Expected**: TopBar smoothly transitions to light theme
5. Verify theme toggling still works correctly after the fix

**Test Case 5: Follow System Mode**
1. Enable "Follow System" mode in TopBar
2. Change system theme (if possible in test environment)
3. **Expected**: TopBar updates correctly with system theme changes
4. Perform Nav interactions while in Follow System mode
5. **Expected**: No theme flicker during Nav interactions

### Success Criteria

✅ **Fix Successful** if:
- TopBar maintains consistent light appearance during all Nav interactions
- No brief dark flashes occur during Nav expand/collapse, item selection, or double-clicks
- Theme toggle functionality remains working
- Follow system mode continues to work correctly
- TopBar animations and transitions appear smooth

❌ **Fix Failed** if:
- TopBar still shows brief dark flashes during Nav interactions
- Theme toggle stops working
- Follow system mode is broken
- Any regression in TopBar appearance or functionality

### Code Changes Summary

**File Modified**: `presentation/ui/declarative/RebuildHost.h`

**Change**: Reordered operations in `requestRebuild()` method:
- **Before**: viewport → updateResourceContext → updateLayout → onThemeChanged
- **After**: viewport → onThemeChanged → updateResourceContext → updateLayout

**Lines Changed**: 8 lines added, 3 lines removed (minimal, surgical change)

**Comments Added**: Explanatory comments documenting the ordering rationale to prevent future regressions

### Technical Validation

The fix ensures that:
1. `onThemeChanged()` is called before `updateResourceContext()`
2. TopBar components receive correct theme state before selecting palettes and icons
3. No transient dark theme state occurs during rebuilds
4. Existing viewport and layout ordering is preserved
5. All other RebuildHost functionality remains unchanged

### Automated Test Coverage

Three test files were added/enhanced to validate the fix:
1. `tests/test_rebuild_host_theme_ordering.cpp` - Before/after comparison
2. `tests/validate_rebuild_host_fix.cpp` - Fix validation with actual logic
3. `tests/TestRebuildHost.cpp` - Enhanced with theme ordering test case

All tests confirm the fix resolves the ordering issue and prevents theme flicker.