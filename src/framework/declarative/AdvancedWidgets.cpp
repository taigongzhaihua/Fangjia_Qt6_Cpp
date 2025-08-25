#include "AdvancedWidgets.h"
#include "UiBoxLayout.h"
#include <memory>
#include <UiComponent.hpp>
#include <utility>

namespace UI {

	std::unique_ptr<IUiComponent> ListTile::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Horizontal);
		layout->setSpacing(12);
		if (m_leading)  layout->addChild(m_leading->build().release(), 0.0f, UiBoxLayout::Alignment::Center);
		if (m_title || m_subtitle) {
			auto col = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Vertical);
			col->setSpacing(4);
			if (m_title)    col->addChild(m_title->build().release(), 0.0f, UiBoxLayout::Alignment::Start);
			if (m_subtitle) col->addChild(m_subtitle->build().release(), 0.0f, UiBoxLayout::Alignment::Start);
			layout->addChild(col.release(), 1.0f, UiBoxLayout::Alignment::Stretch);
		}
		if (m_trailing) layout->addChild(m_trailing->build().release(), 0.0f, UiBoxLayout::Alignment::Center);
		return decorate(std::move(layout));
	}

	std::unique_ptr<IUiComponent> TabBar::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Horizontal);

		// 简化实现：创建一行按钮
		for (const auto& [label, icon] : m_tabs)
		{
			auto container = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Horizontal);

			if (icon) {
				container->addChild(icon->build().release(), 0.0f);
			}

			// 这里需要创建文本组件，暂时跳过

			layout->addChild(container.release(), 1.0f);
		}

		return layout;
	}

} // namespace UI