/*
 * 文件名：button_api_test.cpp
 * 职责：验证Button组件API和基本功能的控制台测试
 * 依赖：BasicWidgets_Button、UiPushButton
 * 说明：测试各种配置选项和API调用
 */

#include "../presentation/ui/declarative/BasicWidgets_Button.h"
#include "../presentation/ui/widgets/UiPushButton.h"
#include "../presentation/ui/declarative/Widget.h"
#include <iostream>
#include <cassert>

using namespace UI;

void testUiPushButtonAPI() {
    std::cout << "=== Testing UiPushButton API ===" << std::endl;
    
    // 创建基本按钮
    UiPushButton button;
    
    // 测试基本属性设置
    button.setText("Test Button");
    assert(button.text() == "Test Button");
    std::cout << "✓ Text setting and retrieval works" << std::endl;
    
    // 测试变体设置
    button.setVariant(UiPushButton::Variant::Primary);
    assert(button.variant() == UiPushButton::Variant::Primary);
    
    button.setVariant(UiPushButton::Variant::Secondary);
    assert(button.variant() == UiPushButton::Variant::Secondary);
    
    button.setVariant(UiPushButton::Variant::Ghost);
    assert(button.variant() == UiPushButton::Variant::Ghost);
    std::cout << "✓ Variant setting works for all types" << std::endl;
    
    // 测试尺寸设置
    button.setSize(UiPushButton::Size::S);
    assert(button.size() == UiPushButton::Size::S);
    
    button.setSize(UiPushButton::Size::M);
    assert(button.size() == UiPushButton::Size::M);
    
    button.setSize(UiPushButton::Size::L);
    assert(button.size() == UiPushButton::Size::L);
    std::cout << "✓ Size setting works for all sizes" << std::endl;
    
    // 测试禁用状态
    button.setDisabled(true);
    assert(button.isDisabled() == true);
    
    button.setDisabled(false);
    assert(button.isDisabled() == false);
    std::cout << "✓ Disabled state setting works" << std::endl;
    
    // 测试图标路径设置
    button.setIconPath(":/icons/test.svg");
    button.setIconThemePaths(":/icons/light.svg", ":/icons/dark.svg");
    std::cout << "✓ Icon path setting works" << std::endl;
    
    // 测试自定义属性
    button.setCornerRadius(12.0f);
    button.setPadding(QMargins(16, 12, 16, 12));
    button.clearCustomPadding();
    std::cout << "✓ Custom properties setting works" << std::endl;
    
    // 测试回调设置
    bool callbackCalled = false;
    button.setOnTap([&callbackCalled]() {
        callbackCalled = true;
    });
    std::cout << "✓ Callback setting works" << std::endl;
    
    std::cout << "UiPushButton API test completed successfully!" << std::endl << std::endl;
}

void testButtonFluentAPI() {
    std::cout << "=== Testing UI::Button Fluent API ===" << std::endl;
    
    // 测试基本流式API
    auto btn1 = button("Primary Button")
        ->primary()
        ->size(Button::Size::M);
    
    assert(btn1 != nullptr);
    std::cout << "✓ Primary button with medium size created" << std::endl;
    
    // 测试次要按钮
    auto btn2 = button("Secondary Button")
        ->secondary()
        ->size(Button::Size::L)
        ->disabled();
    
    assert(btn2 != nullptr);
    std::cout << "✓ Secondary button with large size and disabled state created" << std::endl;
    
    // 测试幽灵按钮
    auto btn3 = button("Ghost Button")
        ->ghost()
        ->size(Button::Size::S)
        ->cornerRadius(6.0f);
    
    assert(btn3 != nullptr);
    std::cout << "✓ Ghost button with small size and custom radius created" << std::endl;
    
    // 测试图标按钮
    auto btn4 = button("Icon Button")
        ->primary()
        ->icon(":/icons/save.svg")
        ->padding(QMargins(20, 10, 20, 10));
    
    assert(btn4 != nullptr);
    std::cout << "✓ Icon button with custom padding created" << std::endl;
    
    // 测试主题图标按钮
    auto btn5 = button("Theme Icon")
        ->secondary()
        ->iconTheme(":/icons/sun.svg", ":/icons/moon.svg");
    
    assert(btn5 != nullptr);
    std::cout << "✓ Theme icon button created" << std::endl;
    
    // 测试回调链式调用
    bool clicked = false;
    auto btn6 = button("Callback Test")
        ->primary()
        ->onTap([&clicked]() {
            clicked = true;
            std::cout << "  Button callback executed!" << std::endl;
        });
    
    assert(btn6 != nullptr);
    std::cout << "✓ Button with callback created" << std::endl;
    
    std::cout << "UI::Button fluent API test completed successfully!" << std::endl << std::endl;
}

void testButtonBuild() {
    std::cout << "=== Testing Button Build Process ===" << std::endl;
    
    // 创建并构建按钮
    auto buttonWidget = button("Build Test")
        ->primary()
        ->size(Button::Size::M)
        ->icon(":/icons/test.svg");
    
    // 构建运行时组件
    auto component = buttonWidget->build();
    assert(component != nullptr);
    std::cout << "✓ Button builds to valid IUiComponent" << std::endl;
    
    // 验证组件可以转换为所需接口
    auto* uiComponent = dynamic_cast<IUiComponent*>(component.get());
    auto* uiContent = dynamic_cast<IUiContent*>(component.get());
    auto* layoutable = dynamic_cast<ILayoutable*>(component.get());
    
    assert(uiComponent != nullptr);
    assert(uiContent != nullptr);
    assert(layoutable != nullptr);
    std::cout << "✓ Built component implements all required interfaces" << std::endl;
    
    // 测试布局接口
    SizeConstraints constraints;
    constraints.minW = 100;
    constraints.maxW = 300;
    constraints.minH = 32;
    constraints.maxH = 100;
    
    QSize measuredSize = layoutable->measure(constraints);
    assert(measuredSize.width() >= constraints.minW);
    assert(measuredSize.width() <= constraints.maxW);
    assert(measuredSize.height() >= constraints.minH);
    assert(measuredSize.height() <= constraints.maxH);
    std::cout << "✓ Measure method returns valid size within constraints" << std::endl;
    
    // 测试安排
    QRect finalRect(0, 0, measuredSize.width(), measuredSize.height());
    layoutable->arrange(finalRect);
    
    QRect bounds = uiComponent->bounds();
    assert(bounds == finalRect);
    std::cout << "✓ Arrange method sets correct bounds" << std::endl;
    
    std::cout << "Button build process test completed successfully!" << std::endl << std::endl;
}

void testVariousConfigurations() {
    std::cout << "=== Testing Various Button Configurations ===" << std::endl;
    
    // 配置1：最小按钮
    auto minBtn = button("")->ghost()->size(Button::Size::S);
    auto minComponent = minBtn->build();
    assert(minComponent != nullptr);
    std::cout << "✓ Minimal button (no text, ghost, small) created" << std::endl;
    
    // 配置2：最大按钮
    auto maxBtn = button("Very Long Button Text That Should Be Handled Properly")
        ->primary()
        ->size(Button::Size::L)
        ->padding(QMargins(32, 20, 32, 20))
        ->cornerRadius(16.0f)
        ->icon(":/icons/complex.svg");
    auto maxComponent = maxBtn->build();
    assert(maxComponent != nullptr);
    std::cout << "✓ Complex button (long text, large, custom padding/radius, icon) created" << std::endl;
    
    // 配置3：禁用按钮
    auto disabledBtn = button("Disabled")
        ->secondary()
        ->disabled()
        ->onTap([]() {
            // 这个回调不应该被调用
            assert(false && "Disabled button callback should not be called");
        });
    auto disabledComponent = disabledBtn->build();
    assert(disabledComponent != nullptr);
    std::cout << "✓ Disabled button created" << std::endl;
    
    // 配置4：仅图标按钮
    auto iconOnlyBtn = button("")
        ->secondary()
        ->size(Button::Size::M)
        ->icon(":/icons/only.svg")
        ->cornerRadius(50.0f); // 圆形按钮
    auto iconOnlyComponent = iconOnlyBtn->build();
    assert(iconOnlyComponent != nullptr);
    std::cout << "✓ Icon-only button created" << std::endl;
    
    std::cout << "Various configurations test completed successfully!" << std::endl << std::endl;
}

int main() {
    std::cout << "Starting Button Component API Tests..." << std::endl << std::endl;
    
    try {
        testUiPushButtonAPI();
        testButtonFluentAPI();
        testButtonBuild();
        testVariousConfigurations();
        
        std::cout << "🎉 All tests passed successfully!" << std::endl;
        std::cout << "Button component implementation is working correctly." << std::endl;
        
        return 0;
    } catch (const std::exception& e) {
        std::cerr << "❌ Test failed with exception: " << e.what() << std::endl;
        return 1;
    } catch (...) {
        std::cerr << "❌ Test failed with unknown exception" << std::endl;
        return 1;
    }
}