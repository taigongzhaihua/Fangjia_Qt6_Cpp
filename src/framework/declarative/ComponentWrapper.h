#pragma once
#include "UiComponent.hpp"
#include "Widget.h"
#include <memory>

namespace UI {

	class ComponentWrapper : public Widget {
	public:
		explicit ComponentWrapper(IUiComponent* component) : m_component(component) {}
		std::unique_ptr<IUiComponent> build() const override;

	private:
		IUiComponent* m_component;
	};

	inline WidgetPtr wrap(IUiComponent* component) {
		return make_widget<ComponentWrapper>(component);
	}

} // namespace UI