/*
 * 简单的Button焦点和键盘支持测试
 * 用于验证新实现的功能是否正常工作
 */

#include "UiPushButton.h"
#include "BasicWidgets_Button.h"
#include "UiRoot.h"
#include <QTest>
#include <QDebug>

void testButtonFocusAndKeyboard() {
    qDebug() << "=== Testing Button Focus and Keyboard Support ===";
    
    // 测试运行时按钮
    UiPushButton button;
    button.setText("Test Button");
    button.setVariant(UiPushButton::Variant::Destructive);
    
    // 测试IFocusable接口
    qDebug() << "Button can focus:" << button.canFocus();
    qDebug() << "Button initially focused:" << button.isFocused();
    
    button.setFocused(true);
    qDebug() << "Button after setFocused(true):" << button.isFocused();
    
    // 测试IKeyInput接口
    bool handled = button.onKeyPress(Qt::Key_Space, Qt::NoModifier);
    qDebug() << "Space key press handled:" << handled;
    
    handled = button.onKeyRelease(Qt::Key_Space, Qt::NoModifier);
    qDebug() << "Space key release handled:" << handled;
    
    // 测试禁用状态
    button.setDisabled(true);
    qDebug() << "Disabled button can focus:" << button.canFocus();
    handled = button.onKeyPress(Qt::Key_Space, Qt::NoModifier);
    qDebug() << "Disabled button handles key:" << handled;
    
    // 测试声明式API
    int clickCount = 0;
    auto declarativeBtn = UI::button("Declarative Test")
        ->destructive()
        ->onTap([&clickCount]() { 
            clickCount++; 
            qDebug() << "Declarative button clicked! Count:" << clickCount;
        });
    
    auto runtimeBtn = declarativeBtn->build();
    qDebug() << "Declarative button created successfully";
    
    // 测试UiRoot焦点管理
    UiRoot root;
    root.add(runtimeBtn.get());
    
    // 模拟点击来设置焦点
    QPoint clickPos(10, 10); // 假设按钮在这个位置
    bool mouseHandled = root.onMousePress(clickPos);
    qDebug() << "UiRoot handled mouse press:" << mouseHandled;
    
    // 测试键盘事件转发
    bool keyHandled = root.onKeyPress(Qt::Key_Enter, Qt::NoModifier);
    qDebug() << "UiRoot handled key press:" << keyHandled;
    
    keyHandled = root.onKeyRelease(Qt::Key_Enter, Qt::NoModifier);
    qDebug() << "UiRoot handled key release:" << keyHandled;
    qDebug() << "Final click count:" << clickCount;
    
    qDebug() << "=== Test completed ===";
}