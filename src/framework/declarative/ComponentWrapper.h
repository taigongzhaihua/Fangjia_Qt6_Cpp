#pragma once
#include "UiComponent.hpp"
#include "Widget.h"
#include <memory>

namespace UI {

	// 用于包装现有的 IUiComponent 为声明式 Widget
	class ComponentWrapper : public Widget {
	public:
		explicit ComponentWrapper(IUiComponent* component) : m_component(component) {}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		IUiComponent* m_component;
	};

	// 便捷函数
	inline WidgetPtr wrap(IUiComponent* component) {
		return make_widget<ComponentWrapper>(component);
	}

} // namespace UI