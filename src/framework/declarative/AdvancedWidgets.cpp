#include "AdvancedWidgets.h"
#include <memory>
#include <UiComponent.hpp>
#include <UiPanel.h>
#include <utility>

namespace UI {

	std::unique_ptr<IUiComponent> ListTile::build() const {
		auto row = std::make_unique<UiPanel>(UiPanel::Orientation::Horizontal);
		row->setSpacing(12);
		if (m_leading)  row->addChild(m_leading->build().release(), UiPanel::CrossAlign::Center);
		if (m_title || m_subtitle) {
			auto col = std::make_unique<UiPanel>(UiPanel::Orientation::Vertical);
			col->setSpacing(4);
			if (m_title)    col->addChild(m_title->build().release(), UiPanel::CrossAlign::Start);
			if (m_subtitle) col->addChild(m_subtitle->build().release(), UiPanel::CrossAlign::Start);
			row->addChild(col.release(), UiPanel::CrossAlign::Stretch);
		}
		if (m_trailing) row->addChild(m_trailing->build().release(), UiPanel::CrossAlign::Center);
		return decorate(std::move(row));
	}

	std::unique_ptr<IUiComponent> TabBar::build() const {
		auto row = std::make_unique<UiPanel>(UiPanel::Orientation::Horizontal);
		row->setSpacing(8);
		for (const auto& [label, iconW] : m_tabs) {
			// 每个 tab 一个小行，当前仅放图标（原实现亦未完成文本部分）
			auto tabRow = std::make_unique<UiPanel>(UiPanel::Orientation::Horizontal);
			if (iconW) tabRow->addChild(iconW->build().release(), UiPanel::CrossAlign::Center);
			row->addChild(tabRow.release(), UiPanel::CrossAlign::Center);

		}
		return row;
	}

} // namespace UI