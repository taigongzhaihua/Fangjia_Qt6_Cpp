#include "AdvancedWidgets.h"
#include "Decorators.h"   // 新增
#include <algorithm>
#include <memory>
#include <UiComponent.hpp>
#include <UiPanel.h>
#include <utility>

namespace UI {

	std::unique_ptr<IUiComponent> Card::build() const
	{
		// 先构建内部内容
		std::unique_ptr<IUiComponent> inner = m_child ? m_child->build() : std::unique_ptr<IUiComponent>{};

		// 用 DecoratedBox 承担 Card 的背景/边框/圆角/内边距，并支持按主题切换
		DecoratedBox::Props p;
		p.padding = m_pal.padding;

		// 亮/暗主题背景
		p.useThemeBg = true;
		p.bgLight = m_pal.bgLight;
		p.bgDark = m_pal.bgDark;
		p.bgRadius = m_pal.radius;

		// 亮/暗主题边框
		if (m_pal.borderLight.alpha() > 0 || m_pal.borderDark.alpha() > 0) {
			p.useThemeBorder = true;
			p.borderLight = m_pal.borderLight;
			p.borderDark = m_pal.borderDark;
			p.borderW = std::max(0.0f, m_pal.borderW);
			p.borderRadius = m_pal.radius;
		}

		// 透传用户在基类 Widget 上设置的 size / margin / 可见性 / 透明度 / 交互
		// 注意：如果用户同时在 Card 上设置了 background/border（旧 API），这里优先使用主题化配置
		p.fixedSize = m_decorations.fixedSize;
		p.margin = m_decorations.margin;
		p.visible = m_decorations.isVisible;
		p.opacity = m_decorations.opacity;
		p.onTap = m_decorations.onTap;
		p.onHover = m_decorations.onHover;

		return std::make_unique<DecoratedBox>(std::move(inner), std::move(p));
	}

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