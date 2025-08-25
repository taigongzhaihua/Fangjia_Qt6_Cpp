#pragma once
#include "AdvancedWidgets.h"
#include "BasicWidgets.h"
#include "ComponentWrapper.h"  // 新增
#include "Layouts.h"
#include "Theme.h"
#include "Widget.h"

namespace UI {

	// 便捷创建函数
	inline auto text(const QString& str) { return make_widget<Text>(str); }
	inline auto icon(const QString& path) { return make_widget<Icon>(path); }
	inline auto button(WidgetPtr child) { return make_widget<Button>(child); }
	inline auto container(WidgetPtr child = nullptr) { return make_widget<Container>(child); }
	inline auto card(WidgetPtr child) { return make_widget<Card>(child); }

	inline auto column(WidgetList children = {}) { return make_widget<Column>(children); }
	inline auto row(WidgetList children = {}) { return make_widget<Row>(children); }
	inline auto stack(WidgetList children = {}) { return make_widget<Stack>(children); }

	inline auto expanded(WidgetPtr child, float flex = 1.0f) {
		return make_widget<Expanded>(child, flex);
	}
	inline auto spacer(int size = 0) { return make_widget<Spacer>(size); }

	inline auto listTile() { return make_widget<ListTile>(); }
	inline auto tabBar(std::vector<TabBar::Tab> tabs) { return make_widget<TabBar>(tabs); }

	// 条件渲染
	inline auto when(bool condition, WidgetPtr ifTrue, WidgetPtr ifFalse = nullptr) {
		return make_widget<Conditional>(condition, ifTrue, ifFalse);
	}

	// 主题
	inline auto theme(ThemeData data, WidgetPtr child) {
		return make_widget<Theme>(data, child);
	}

	inline auto themed(std::function<WidgetPtr(const ThemeData&)> builder) {
		return make_widget<ThemedBuilder>(builder);
	}


} // namespace UI

// 简化命名空间使用
using namespace UI;