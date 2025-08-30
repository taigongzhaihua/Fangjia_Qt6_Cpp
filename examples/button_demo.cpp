/*
 * 文件名：button_demo.cpp
 * 职责：演示新的Button组件功能的简单示例
 * 依赖：BasicWidgets_Button、UI声明式框架
 * 说明：非侵入式示例，展示按钮的各种变体和配置选项
 */

#include "BasicWidgets_Button.h"
#include "Layouts.h"
#include <iostream>

using namespace UI;

/// 功能：创建按钮演示布局
/// 返回：包含各种按钮示例的布局组件
/// 说明：展示Primary、Secondary、Ghost变体及不同尺寸
std::shared_ptr<Widget> createButtonDemo() {
	return vbox({
		// 标题
		text("Button Component Demo")->fontSize(18)->color(QColor(60, 65, 70)),
		
		// Primary变体示例
		hbox({
			text("Primary:")->fontSize(14),
			button("Small")->primary()->size(Button::Size::S)->onTap([]() {
				std::cout << "Small Primary button clicked!" << std::endl;
			}),
			button("Medium")->primary()->size(Button::Size::M)->onTap([]() {
				std::cout << "Medium Primary button clicked!" << std::endl;
			}),
			button("Large")->primary()->size(Button::Size::L)->onTap([]() {
				std::cout << "Large Primary button clicked!" << std::endl;
			})
		}),
		
		// Secondary变体示例
		hbox({
			text("Secondary:")->fontSize(14),
			button("Cancel")->secondary()->onTap([]() {
				std::cout << "Cancel button clicked!" << std::endl;
			}),
			button("Reset")->secondary()->disabled()->onTap([]() {
				std::cout << "This should not be called (disabled)" << std::endl;
			})
		}),
		
		// Ghost变体示例
		hbox({
			text("Ghost:")->fontSize(14),
			button("Link Action")->ghost()->onTap([]() {
				std::cout << "Ghost button clicked!" << std::endl;
			}),
			button("Another Link")->ghost()->cornerRadius(4.0f)
		}),
		
		// 带图标的按钮示例（注：需要实际的SVG文件路径）
		hbox({
			text("With Icons:")->fontSize(14),
			button("Save")->primary()->icon(":/icons/save.svg")->onTap([]() {
				std::cout << "Save with icon clicked!" << std::endl;
			}),
			button("")->secondary()->icon(":/icons/settings.svg")->onTap([]() {
				std::cout << "Settings icon button clicked!" << std::endl;
			})
		}),
		
		// 主题图标示例
		hbox({
			text("Theme Icons:")->fontSize(14),
			button("Theme")->ghost()->iconTheme(":/icons/sun.svg", ":/icons/moon.svg")->onTap([]() {
				std::cout << "Theme toggle clicked!" << std::endl;
			})
		}),
		
		// 自定义内边距示例
		hbox({
			text("Custom Padding:")->fontSize(14),
			button("Wide Padding")->primary()->padding(QMargins(32, 12, 32, 12))->onTap([]() {
				std::cout << "Wide padding button clicked!" << std::endl;
			}),
			button("Narrow")->secondary()->padding(QMargins(8, 4, 8, 4))
		})
	});
}

/// 功能：按钮组件测试函数
/// 说明：可以在主应用或测试环境中调用此函数来验证按钮功能
void testButtonComponent() {
	std::cout << "=== Button Component Test ===" << std::endl;
	
	// 创建各种按钮配置进行测试
	auto primaryBtn = button("Test Primary")->primary()->size(Button::Size::M);
	auto secondaryBtn = button("Test Secondary")->secondary();
	auto ghostBtn = button("Test Ghost")->ghost();
	auto disabledBtn = button("Disabled")->primary()->disabled();
	
	std::cout << "Button components created successfully!" << std::endl;
	std::cout << "- Primary button configured" << std::endl;
	std::cout << "- Secondary button configured" << std::endl;
	std::cout << "- Ghost button configured" << std::endl;
	std::cout << "- Disabled button configured" << std::endl;
	
	// 测试链式调用
	auto complexBtn = button("Complex")
		->primary()
		->size(Button::Size::L)
		->cornerRadius(12.0f)
		->padding(QMargins(24, 16, 24, 16))
		->icon(":/icons/star.svg")
		->onTap([]() {
			std::cout << "Complex button with all features clicked!" << std::endl;
		});
	
	std::cout << "Complex button with chained configuration created!" << std::endl;
	std::cout << "=== Test Complete ===" << std::endl;
}

/// 功能：演示回调函数的使用
/// 说明：展示如何为按钮绑定不同类型的回调
void demonstrateCallbacks() {
	// Lambda回调
	auto lambdaBtn = button("Lambda")->primary()->onTap([]() {
		std::cout << "Lambda callback executed!" << std::endl;
	});
	
	// 捕获变量的回调
	int counter = 0;
	auto captureBtn = button("Counter")->secondary()->onTap([&counter]() {
		counter++;
		std::cout << "Counter button clicked " << counter << " times!" << std::endl;
	});
	
	// 函数对象回调
	auto funcBtn = button("Function")->ghost()->onTap(std::function<void()>([]() {
		std::cout << "Function object callback!" << std::endl;
	}));
	
	std::cout << "Callback demonstration setup complete!" << std::endl;
}