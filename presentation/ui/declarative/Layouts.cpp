#include "Layouts.h"
#include <memory>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include <UiGrid.h>
#include <UiPanel.h>
#include <utility>

namespace UI {

	std::unique_ptr<IUiComponent> Panel::build() const {
		auto layout = std::make_unique<UiPanel>(m_orient);
		layout->setSpacing(m_spacing);

		const auto cross = toCross(m_crossAlign);
		for (const auto& child : m_children) {
			if (!child) continue;
			auto comp = child->build();
			layout->addChild(comp.release(), cross);
		}
		return decorate(std::move(layout));
	}

	// Spacer
	class SpacerComponent : public IUiComponent {
	public:
		explicit SpacerComponent(const int size) : m_size(size) {}
		void updateLayout(const QSize&) override {}
		void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
		void append(Render::FrameData&) const override {}
		bool onMousePress(const QPoint&) override { return false; }
		bool onMouseMove(const QPoint&) override { return false; }
		bool onMouseRelease(const QPoint&) override { return false; }
		bool tick() override { return false; }
		[[nodiscard]] QRect bounds() const override { return { 0, 0, m_size, m_size }; }
		void onThemeChanged(bool) override {}
	private:
		int m_size;
	};
	std::unique_ptr<IUiComponent> Spacer::build() const { return std::make_unique<SpacerComponent>(m_size); }

	// ============ Grid ============
	static UiGrid::TrackDef toDef(const Grid::Track& t) {
		switch (t.type) {
		case Grid::TrackType::Pixel: return UiGrid::TrackDef::Px(static_cast<int>(std::round(t.value)));
		case Grid::TrackType::Star:  return UiGrid::TrackDef::Star(t.value <= 0.0f ? 1.0f : t.value);
		case Grid::TrackType::Auto:
		default:                      return UiGrid::TrackDef::Auto();
		}
	}
	static UiGrid::Align toAlign(const Grid::CellAlign a) {
		switch (a) {
		case Grid::CellAlign::Start:   return UiGrid::Align::Start;
		case Grid::CellAlign::Center:  return UiGrid::Align::Center;
		case Grid::CellAlign::End:     return UiGrid::Align::End;
		case Grid::CellAlign::Stretch:
		default:                       return UiGrid::Align::Stretch;
		}
	}

	std::unique_ptr<IUiComponent> Grid::build() const {
		auto layout = std::make_unique<UiGrid>();

		std::vector<UiGrid::TrackDef> rs; rs.reserve(m_rows.size());
		for (const auto& r : m_rows) rs.push_back(toDef(r));
		std::vector<UiGrid::TrackDef> cs; cs.reserve(m_cols.size());
		for (const auto& c : m_cols) cs.push_back(toDef(c));

		layout->setRowDefs(std::move(rs));
		layout->setColDefs(std::move(cs));
		layout->setRowSpacing(m_rowSpacing);
		layout->setColSpacing(m_colSpacing);

		for (const auto& it : m_items) {
			if (!it.widget) continue;
			auto comp = it.widget->build();
			layout->addChild(
				comp.release(),
				std::max(0, it.row), std::max(0, it.col),
				std::max(1, it.rowSpan), std::max(1, it.colSpan),
				toAlign(it.h), toAlign(it.v)
			);
		}
		return decorate(std::move(layout));
	}

} // namespace UI