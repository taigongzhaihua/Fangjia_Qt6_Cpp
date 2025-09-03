#pragma once
#include "Widget.h"
#include <memory>
#include "UiComponent.hpp"

namespace UI {

	// 声明式 ScrollView：包装 UiScrollView 提供滚动容器
	class ScrollView : public Widget {
	public:
		ScrollView() = default;
		
		// 设置子组件
		std::shared_ptr<ScrollView> child(WidgetPtr widget) {
			m_child = std::move(widget);
			return self<ScrollView>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetPtr m_child;
	};

} // namespace UI