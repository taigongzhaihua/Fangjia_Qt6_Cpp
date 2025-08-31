#pragma once
#include "UiComponent.hpp"
#include "IFocusContainer.hpp"
#include "Widget.h"
#include <memory>

namespace UI {

	class ComponentWrapper : public Widget, public IFocusContainer {
	public:
		explicit ComponentWrapper(IUiComponent* component) : m_component(component) {}
		std::unique_ptr<IUiComponent> build() const override;

		// IFocusContainer
		void enumerateFocusables(std::vector<IFocusable*>& out) const override;

	private:
		IUiComponent* m_component;
	};

	inline WidgetPtr wrap(IUiComponent* component) {
		return make_widget<ComponentWrapper>(component);
	}

} // namespace UI