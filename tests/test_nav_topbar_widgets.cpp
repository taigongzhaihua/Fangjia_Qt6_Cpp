/*
 * 文件名：test_nav_topbar_widgets.cpp
 * 职责：测试声明式NavRail和TopBar组件的API和基本功能
 * 依赖：NavTopBarWidgets.h、测试框架
 * 线程：仅在UI线程使用
 * 备注：验证链式API的正确性和build()方法的有效性
 */

#include "NavTopBarWidgets.h"
#include "UI.h"
#include <iostream>
#include <cassert>

namespace {
	// 测试NavRail声明式API
	void testNavRailAPI() {
		// 创建NavRail并链式配置
		auto rail = UI::navRail()
			->widths(48, 200)
			->iconSize(22)
			->itemHeight(48)
			->labelFontPx(13)
			->toggleSvg(":/icons/expand.svg", ":/icons/collapse.svg");

		// 验证能够构建组件
		auto component = rail->build();
		assert(component != nullptr);

		std::cout << "NavRail API test passed" << std::endl;
	}

	// 测试TopBar声明式API
	void testTopBarAPI() {
		// 创建TopBar并链式配置
		auto bar = UI::topBar()
			->followSystem(true, false)
			->cornerRadius(6.0f)
			->svgTheme(":/icons/sun.svg", ":/icons/moon.svg")
			->svgFollow(":/icons/follow_on.svg", ":/icons/follow_off.svg")
			->svgSystem(":/icons/min.svg", ":/icons/max.svg", ":/icons/close.svg")
			->onThemeToggle([]() {
				std::cout << "Theme toggle callback triggered" << std::endl;
			});

		// 验证能够构建组件
		auto component = bar->build();
		assert(component != nullptr);

		std::cout << "TopBar API test passed" << std::endl;
	}

	// 测试装饰器集成
	void testDecorators() {
		// 测试NavRail与装饰器的配合
		auto decoratedRail = UI::navRail()
			->widths(64, 220)
			->padding(8)
			->margin(4, 2)
			->background(QColor(255, 255, 255, 200), 8.0f);

		auto component = decoratedRail->build();
		assert(component != nullptr);

		std::cout << "Decorators integration test passed" << std::endl;
	}
}

int main() {
	try {
		testNavRailAPI();
		testTopBarAPI();
		testDecorators();
		
		std::cout << "All NavTopBarWidgets tests passed!" << std::endl;
		return 0;
	}
	catch (const std::exception& e) {
		std::cerr << "Test failed with exception: " << e.what() << std::endl;
		return 1;
	}
	catch (...) {
		std::cerr << "Test failed with unknown exception" << std::endl;
		return 1;
	}
}