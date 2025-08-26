#pragma once
#include "UiPanel.h"
#include "Widget.h"

namespace UI {

	// 布局对齐方式（沿用现有定义，供 crossAxisAlignment 使用）
	enum class Alignment {
		Start, Center, End, Stretch, SpaceBetween, SpaceAround, SpaceEvenly
	};

	// Panel（声明式）：顺序容器，按子项实际尺寸依次排布
	class Panel : public Widget {
	public:
		Panel(WidgetList children = {}) : m_children(std::move(children)) {}

		// 设置轴向：水平/垂直
		std::shared_ptr<Panel> orientation(UiPanel::Orientation o) { m_orient = o; return self<Panel>(); }

		// 子项之间的间距（像素）
		std::shared_ptr<Panel> spacing(int s) { m_spacing = std::max(0, s); return self<Panel>(); }

		// 交叉轴对齐（Start/Center/End/Stretch）
		std::shared_ptr<Panel> crossAxisAlignment(Alignment a) { m_crossAlign = a; return self<Panel>(); }

		std::shared_ptr<Panel> children(WidgetList children) { m_children = std::move(children); return self<Panel>(); }

		std::unique_ptr<IUiComponent> build() const override;

	private:
		static UiPanel::CrossAlign toCross(Alignment a) {
			switch (a) {
			case Alignment::Center:  return UiPanel::CrossAlign::Center;
			case Alignment::End:     return UiPanel::CrossAlign::End;
			case Alignment::Stretch: return UiPanel::CrossAlign::Stretch;
			case Alignment::Start:
			case Alignment::SpaceBetween:
			case Alignment::SpaceAround:
			case Alignment::SpaceEvenly:
			default:                 return UiPanel::CrossAlign::Start;
			}
		}

	private:
		WidgetList m_children;
		UiPanel::Orientation m_orient{ UiPanel::Orientation::Vertical };
		int m_spacing{ 0 };
		Alignment m_crossAlign{ Alignment::Start };
	};

	// 简单占位/留白
	class Spacer : public Widget {
	public:
		explicit Spacer(int size = 0) : m_size(size) {}
		std::unique_ptr<IUiComponent> build() const override;
	private:
		int m_size;
	};

} // namespace UI