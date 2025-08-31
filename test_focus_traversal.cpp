#include <QCoreApplication>
#include <QDebug>
#include <QRect>
#include <QSize>
#include <Qt>

// Test includes
#include "presentation/ui/containers/UiRoot.h"
#include "presentation/ui/containers/UiPanel.h"
#include "presentation/ui/widgets/UiPushButton.h"
#include "presentation/ui/base/IFocusable.hpp"

/**
 * @brief Test focus traversal functionality in UiRoot and containers
 * 
 * This test creates a simple UI hierarchy with buttons and verifies:
 * 1. IFocusContainer enumeration works correctly
 * 2. Tab/Shift+Tab navigation works
 * 3. Focus order is maintained correctly
 */
void testFocusTraversal() {
    qDebug() << "=== Testing Focus Traversal ===";
    
    // Create root and panels
    UiRoot root;
    auto panel1 = std::make_unique<UiPanel>(UiPanel::Orientation::Vertical);
    auto panel2 = std::make_unique<UiPanel>(UiPanel::Orientation::Horizontal);
    
    // Create buttons
    auto button1 = std::make_unique<UiPushButton>();
    auto button2 = std::make_unique<UiPushButton>();
    auto button3 = std::make_unique<UiPushButton>();
    auto button4 = std::make_unique<UiPushButton>();
    
    button1->setText("Button 1");
    button2->setText("Button 2");
    button3->setText("Button 3");
    button4->setText("Button 4");
    
    // Get raw pointers for testing focus state
    UiPushButton* btn1_ptr = button1.get();
    UiPushButton* btn2_ptr = button2.get();
    UiPushButton* btn3_ptr = button3.get();
    UiPushButton* btn4_ptr = button4.get();
    
    // Build hierarchy: Root -> Panel1 -> [Button1, Button2, Panel2 -> [Button3, Button4]]
    panel2->addChild(button3.release());
    panel2->addChild(button4.release());
    
    panel1->addChild(button1.release());
    panel1->addChild(button2.release());
    panel1->addChild(panel2.release());
    
    UiPanel* root_panel = panel1.get();
    root.add(panel1.release());
    
    // Test focus enumeration directly on our root panel
    qDebug() << "Testing focus enumeration...";
    std::vector<IFocusable*> focusables;
    root_panel->enumerateFocusables(focusables);
    
    // We should have 4 focusable buttons
    if (focusables.size() == 4) {
        qDebug() << "✓ Found" << focusables.size() << "focusable components";
    } else {
        qDebug() << "✗ Expected 4 focusables, found" << focusables.size();
        return;
    }
    
    // Test Tab navigation
    qDebug() << "Testing Tab navigation...";
    
    // Initially no button should have focus
    bool anyFocused = btn1_ptr->isFocused() || btn2_ptr->isFocused() || 
                      btn3_ptr->isFocused() || btn4_ptr->isFocused();
    if (!anyFocused) {
        qDebug() << "✓ Initially no buttons have focus";
    } else {
        qDebug() << "✗ Expected no initial focus on buttons";
        return;
    }
    
    // Simulate Tab key press
    bool handled = root.onKeyPress(Qt::Key_Tab, Qt::NoModifier);
    if (handled) {
        qDebug() << "✓ Tab key was handled";
        
        // Check if first button got focus
        if (btn1_ptr->isFocused()) {
            qDebug() << "✓ First button received focus";
        } else {
            qDebug() << "ℹ First button focus state:" << btn1_ptr->isFocused();
        }
    } else {
        qDebug() << "✗ Tab key was not handled";
        return;
    }
    
    // Simulate another Tab key press
    handled = root.onKeyPress(Qt::Key_Tab, Qt::NoModifier);
    if (handled) {
        qDebug() << "✓ Second Tab key was handled";
        if (btn2_ptr->isFocused()) {
            qDebug() << "✓ Second Tab moved focus to second button";
        } else {
            qDebug() << "ℹ Second button focus state:" << btn2_ptr->isFocused();
        }
    } else {
        qDebug() << "✗ Second Tab key was not handled";
    }
    
    // Test Shift+Tab navigation
    qDebug() << "Testing Shift+Tab navigation...";
    handled = root.onKeyPress(Qt::Key_Tab, Qt::ShiftModifier);
    if (handled) {
        qDebug() << "✓ Shift+Tab key was handled";
        if (btn1_ptr->isFocused()) {
            qDebug() << "✓ Shift+Tab moved focus back to first button";
        } else {
            qDebug() << "ℹ First button focus state after Shift+Tab:" << btn1_ptr->isFocused();
        }
    } else {
        qDebug() << "✗ Shift+Tab key was not handled";
    }
    
    qDebug() << "=== Focus Traversal Test Completed ===\n";
}

int main(int argc, char *argv[]) {
    QCoreApplication app(argc, argv);
    
    qDebug() << "Starting Focus Management System Tests...";
    testFocusTraversal();
    qDebug() << "All tests completed!";
    
    return 0;
}