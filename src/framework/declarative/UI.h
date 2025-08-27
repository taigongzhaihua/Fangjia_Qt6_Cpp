#pragma once
#include "AdvancedWidgets.h"
#include "BasicWidgets.h"
#include "ComponentWrapper.h"
#include "Layouts.h"
#include "TabView.h"
#include "Theme.h"
#include "Widget.h"

namespace UI {

	// 便捷创建函数
	inline auto text(const QString& str) { return make_widget<Text>(str); }
	inline auto icon(const QString& path) { return make_widget<Icon>(path); }
	inline auto button(WidgetPtr child) { return make_widget<Button>(child); }
	inline auto container(WidgetPtr child = nullptr) { return make_widget<Container>(child); }
	inline auto card(WidgetPtr child) { return make_widget<Card>(child); }

	// Panel / Spacer
	inline auto panel(WidgetList children = {}) { return make_widget<Panel>(children); }
	inline auto spacer(int size = 0) { return make_widget<Spacer>(size); }

	// Grid
	inline auto grid() { return make_widget<Grid>(); }

	inline auto listTile() { return make_widget<ListTile>(); }
	inline auto tabView() { return make_widget<TabView>(); }

	// 条件渲染
	inline auto when(bool condition, WidgetPtr ifTrue, WidgetPtr ifFalse = nullptr) {
		return make_widget<Conditional>(condition, ifTrue, ifFalse);
	}

	// 主题
	inline auto theme(ThemeData data, WidgetPtr child) { return make_widget<Theme>(data, child); }
	inline auto themed(std::function<WidgetPtr(const ThemeData&)> builder) { return make_widget<ThemedBuilder>(builder); }

} // namespace UI

using namespace UI;