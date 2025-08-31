#pragma once
#include "AdvancedWidgets.h"
#include "BasicWidgets.h"
#include "ComponentWrapper.h"
#include "Layouts.h"
#include "TabView.h"
#include "ScrollView.h"
#include "Widget.h"
#include "AppShell.h"
#include "NavTopBarWidgets.h"

namespace UI {

	// 便捷创建函数
	inline auto text(const QString& str) { return make_widget<Text>(str); }
	inline auto icon(const QString& path) { return make_widget<Icon>(path); }
	inline auto container(WidgetPtr child = nullptr) { return make_widget<Container>(child); }
	inline auto card(WidgetPtr child) { return make_widget<Card>(child); }

	// Panel / Spacer
	inline auto panel(WidgetList children = {}) { return make_widget<Panel>(children); }
	inline auto spacer(int size = 0) { return make_widget<Spacer>(size); }

	// Grid
	inline auto grid() { return make_widget<Grid>(); }

	inline auto tabView() { return make_widget<TabView>(); }

	// ScrollView
	inline auto scrollView(WidgetPtr child = nullptr) { 
		return make_widget<ScrollView>()->child(child); 
	}

	// AppShell factory (declarative)
	inline auto appShell() { return make_widget<AppShell>(); }

	// NavRail factory (declarative)
	inline auto navRail() { return make_widget<NavRail>(); }

	// TopBar factory (declarative)
	inline auto topBar() { return make_widget<TopBar>(); }

	// 条件渲染
	inline auto when(bool condition, WidgetPtr ifTrue, WidgetPtr ifFalse = nullptr) {
		return make_widget<Conditional>(condition, ifTrue, ifFalse);
	}



} // namespace UI

using namespace UI;