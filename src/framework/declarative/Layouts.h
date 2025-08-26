#pragma once
#include "UiPanel.h"
#include "Widget.h"
#include <qmargins.h>  // 新增

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
		// 便捷：垂直/水平
		std::shared_ptr<Panel> vertical() { m_orient = UiPanel::Orientation::Vertical; return self<Panel>(); }
		std::shared_ptr<Panel> horizontal() { m_orient = UiPanel::Orientation::Horizontal; return self<Panel>(); }

		// 子项之间的间距（像素）
		std::shared_ptr<Panel> spacing(int s) { m_spacing = std::max(0, s); return self<Panel>(); }

		// 交叉轴对齐（Start/Center/End/Stretch）
		std::shared_ptr<Panel> crossAxisAlignment(Alignment a) { m_crossAlign = a; return self<Panel>(); }

		// Panel 自身的边距/内边距（作用于 UiPanel，非 DecoratedBox）
		std::shared_ptr<Panel> margins(const QMargins& m) { m_margins = m; return self<Panel>(); }
		std::shared_ptr<Panel> margin(int all) { m_margins = QMargins(all, all, all, all); return self<Panel>(); }
		std::shared_ptr<Panel> margin(int horizontal, int vertical) { m_margins = QMargins(horizontal, vertical, horizontal, vertical); return self<Panel>(); }
		std::shared_ptr<Panel> margin(int left, int top, int right, int bottom) { m_margins = QMargins(left, top, right, bottom); return self<Panel>(); }

		std::shared_ptr<Panel> padding(const QMargins& p) { m_padding = p; return self<Panel>(); }
		std::shared_ptr<Panel> padding(int all) { m_padding = QMargins(all, all, all, all); return self<Panel>(); }
		std::shared_ptr<Panel> padding(int horizontal, int vertical) { m_padding = QMargins(horizontal, vertical, horizontal, vertical); return self<Panel>(); }
		std::shared_ptr<Panel> padding(int left, int top, int right, int bottom) { m_padding = QMargins(left, top, right, bottom); return self<Panel>(); }

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

		// 新增：Panel 自身的 margin/padding
		QMargins m_margins{ 0,0,0,0 };
		QMargins m_padding{ 0,0,0,0 };
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