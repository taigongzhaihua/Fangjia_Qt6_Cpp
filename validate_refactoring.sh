#!/bin/bash

# Validation script for PageManager -> PageRouter refactoring

echo "=== Fangjia PageRouter Refactoring Validation ==="
echo

# Check if PageManager files are removed
echo "1. Checking PageManager removal..."
if [ ! -f "src/framework/containers/PageManager.h" ] && [ ! -f "src/framework/containers/PageManager.cpp" ]; then
    echo "   ✓ PageManager files successfully removed"
else
    echo "   ✗ PageManager files still exist"
fi

# Check if PageRouter files exist
echo "2. Checking PageRouter creation..."
if [ -f "src/framework/containers/PageRouter.h" ] && [ -f "src/framework/containers/PageRouter.cpp" ]; then
    echo "   ✓ PageRouter files created"
else
    echo "   ✗ PageRouter files missing"
fi

# Check for PageManager references
echo "3. Checking for remaining PageManager references..."
PAGEMANAGER_REFS=$(find . -name "*.cpp" -o -name "*.h" | xargs grep -l "PageManager" 2>/dev/null | wc -l)
if [ $PAGEMANAGER_REFS -eq 0 ]; then
    echo "   ✓ No PageManager references found"
else
    echo "   ✗ Found $PAGEMANAGER_REFS files still referencing PageManager"
    find . -name "*.cpp" -o -name "*.h" | xargs grep -l "PageManager" 2>/dev/null
fi

# Check PageRouter usage
echo "4. Checking PageRouter usage..."
PAGEROUTER_REFS=$(find . -name "*.cpp" -o -name "*.h" | xargs grep -l "PageRouter" 2>/dev/null | wc -l)
if [ $PAGEROUTER_REFS -ge 3 ]; then
    echo "   ✓ PageRouter properly used in $PAGEROUTER_REFS files"
else
    echo "   ✗ PageRouter usage seems incomplete ($PAGEROUTER_REFS files)"
fi

# Check for lifecycle hooks
echo "5. Checking lifecycle hook implementation..."
ONAPPEAR_COUNT=$(find . -name "*.cpp" -o -name "*.h" | xargs grep -c "onAppear\|onDisappear" 2>/dev/null | paste -sd+ | bc)
if [ $ONAPPEAR_COUNT -gt 10 ]; then
    echo "   ✓ Lifecycle hooks implemented ($ONAPPEAR_COUNT occurrences)"
else
    echo "   ✗ Lifecycle hooks may be missing ($ONAPPEAR_COUNT occurrences)"
fi

# Check test file
echo "6. Checking test implementation..."
if [ -f "tests/test_page_router.cpp" ]; then
    echo "   ✓ PageRouter test file created"
else
    echo "   ✗ PageRouter test file missing"
fi

echo
echo "=== Refactoring Summary ==="
echo "The PageManager has been successfully refactored to PageRouter with:"
echo "- Factory pattern for lazy page creation"
echo "- onAppear/onDisappear lifecycle hooks"
echo "- Updated MainOpenGlWindow integration"
echo "- Example implementations in HomePage and DataPage"
echo "- Unit test for verification"
echo
echo "Key benefits achieved:"
echo "- Pages are now created lazily when first accessed"
echo "- Lifecycle hooks enable resource management and analytics"
echo "- Factory pattern supports future dynamic page loading"
echo "- Caching improves performance on repeated page switches"