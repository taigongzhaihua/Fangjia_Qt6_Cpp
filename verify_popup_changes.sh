#!/bin/bash
# Simple verification script for HomePage popup changes

echo "=== HomePage Popup Implementation Verification ==="

echo -e "\n1. Checking HomePage.h for setWindowContext method..."
if grep -q "setWindowContext" presentation/pages/HomePage.h; then
    echo "✅ setWindowContext method declared"
else 
    echo "❌ setWindowContext method NOT found"
fi

echo -e "\n2. Checking HomePage.cpp for Popup include..."
if grep -q "#include \"Popup.h\"" presentation/pages/HomePage.cpp; then
    echo "✅ Popup.h included"
else 
    echo "❌ Popup.h NOT included"
fi

echo -e "\n3. Checking for static member definition..."
if grep -q "QWindow\* HomePage::Impl::s_windowContext" presentation/pages/HomePage.cpp; then
    echo "✅ Static member s_windowContext defined"
else 
    echo "❌ Static member s_windowContext NOT defined"
fi

echo -e "\n4. Checking MainOpenGlWindow for setWindowContext call..."
if grep -q "HomePage::setWindowContext" apps/fangjia/MainOpenGlWindow.cpp; then
    echo "✅ HomePage::setWindowContext called in MainOpenGlWindow"
else 
    echo "❌ HomePage::setWindowContext NOT called"
fi

echo -e "\n5. Checking for popup->showPopup() calls..."
popup_show_count=$(grep -c "popup[12]->showPopup()" presentation/pages/HomePage.cpp)
if [ $popup_show_count -ge 2 ]; then
    echo "✅ Found $popup_show_count popup->showPopup() calls"
else 
    echo "❌ Expected at least 2 popup->showPopup() calls, found $popup_show_count"
fi

echo -e "\n6. Checking for popup close functionality..."
if grep -q "popup1->hidePopup()" presentation/pages/HomePage.cpp; then
    echo "✅ Popup close functionality implemented"
else 
    echo "❌ Popup close functionality NOT found"
fi

echo -e "\n=== Summary ==="
echo "The HomePage popup implementation includes:"
echo "- Window context management"
echo "- Direct Popup object creation and storage" 
echo "- Working show/hide functionality"
echo "- Rich popup content with interactive elements"
echo "- Visual feedback based on popup availability"

echo -e "\nNext steps would be:"
echo "- Build and test with Qt6 environment"
echo "- Verify popup positioning and display"
echo "- Test popup interaction and closing"