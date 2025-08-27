#pragma once
#include "UiPanel.h"
#include "Widget.h"
#include <qmargins.h>

class UiGrid; // 前向声明

namespace UI {

	// 布局对齐方式（沿用现有定义，供 crossAxisAlignment 使用）
	enum class Alignment {
		Start, Center, End, Stretch, SpaceBetween, SpaceAround, SpaceEvenly
	};

	// Panel（声明式）：顺序容器
	class Panel : public Widget {
	public:
		Panel(WidgetList children = {}) : m_children(std::move(children)) {}
		std::shared_ptr<Panel> orientation(const UiPanel::Orientation o) { m_orient = o; return self<Panel>(); }
		std::shared_ptr<Panel> vertical() { m_orient = UiPanel::Orientation::Vertical; return self<Panel>(); }
		std::shared_ptr<Panel> horizontal() { m_orient = UiPanel::Orientation::Horizontal; return self<Panel>(); }
		std::shared_ptr<Panel> spacing(const int s) { m_spacing = std::max(0, s); return self<Panel>(); }
		std::shared_ptr<Panel> crossAxisAlignment(const Alignment a) { m_crossAlign = a; return self<Panel>(); }
		std::shared_ptr<Panel> children(WidgetList children) { m_children = std::move(children); return self<Panel>(); }
		std::unique_ptr<IUiComponent> build() const override;

	private:
		static UiPanel::CrossAlign toCross(const Alignment a) {
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
		QMargins m_margins{ 0,0,0,0 };
		QMargins m_padding{ 0,0,0,0 };
	};

	// 简单占位
	class Spacer : public Widget {
	public:
		explicit Spacer(const int size = 0) : m_size(size) {}
		std::unique_ptr<IUiComponent> build() const override;
	private:
		int m_size;
	};

	// =================== 新增：Grid（声明式） ===================
	class Grid : public Widget {
	public:
		enum class TrackType { Auto, Pixel, Star };
		struct Track {
			TrackType type{ TrackType::Auto };
			float value{ 0.0f }; // Pixel->像素；Star->权重
			static Track Auto() { return { TrackType::Auto, 0.0f }; }
			static Track Px(const int px) { return { Pixel, static_cast<float>(std::max(0, px)) }; }
			static Track Star(const float w = 1.0f) { return { TrackType::Star, std::max(0.0f, w) }; }
		};

		enum class CellAlign { Start, Center, End, Stretch };

		struct Item {
			WidgetPtr widget;
			int row{ 0 }, col{ 0 };
			int rowSpan{ 1 }, colSpan{ 1 };
			CellAlign h{ Stretch }, v{ Stretch };
		};

		Grid() = default;

		std::shared_ptr<Grid> rows(std::vector<Track> defs) { m_rows = std::move(defs); return self<Grid>(); }
		std::shared_ptr<Grid> columns(std::vector<Track> defs) { m_cols = std::move(defs); return self<Grid>(); }
		std::shared_ptr<Grid> rowSpacing(const int px) { m_rowSpacing = std::max(0, px); return self<Grid>(); }
		std::shared_ptr<Grid> colSpacing(const int px) { m_colSpacing = std::max(0, px); return self<Grid>(); }

		// 添加子项
		std::shared_ptr<Grid> add(WidgetPtr w, const int row, const int col, const int rowSpan = 1, const int colSpan = 1,
			const CellAlign h = Stretch, const CellAlign v = Stretch) {
			m_items.push_back(Item{ std::move(w), row, col, rowSpan, colSpan, h, v });
			return self<Grid>();
		}

		std::unique_ptr<IUiComponent> build() const override;

		using enum CellAlign;
		using enum TrackType;
	private:
		std::vector<Track> m_rows;
		std::vector<Track> m_cols;
		int m_rowSpacing{ 8 };
		int m_colSpacing{ 8 };
		std::vector<Item> m_items;

	};

	inline Grid::Track AUTO = Grid::Track::Auto();

	inline Grid::Track operator""_px(const unsigned long long v) noexcept {
		return Grid::Track::Px(static_cast<int>(v));
	}

	inline Grid::Track operator""_fr(const unsigned long long v) noexcept {
		return Grid::Track::Star(static_cast<float>(v));
	}

	inline Grid::Track operator""_fr(const long double v) noexcept {
		return Grid::Track::Star(static_cast<float>(v));
	}

} // namespace UI