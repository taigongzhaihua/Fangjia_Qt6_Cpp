#include "Layouts.h"
#include "UiBoxLayout.h"
#include <IconLoader.h>
#include <memory>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>
#include <UiComponent.hpp>
#include <utility>

namespace UI {

	static UiBoxLayout::Alignment toBoxAlignment(Alignment align) {
		switch (align) {
		case Alignment::Start: return UiBoxLayout::Alignment::Start;
		case Alignment::Center: return UiBoxLayout::Alignment::Center;
		case Alignment::End: return UiBoxLayout::Alignment::End;
		case Alignment::Stretch: return UiBoxLayout::Alignment::Stretch;
		default: return UiBoxLayout::Alignment::Start;
		}
	}

	static UiBoxLayout::MainAlignment toBoxMain(Alignment align) {
		switch (align) {
		case Alignment::Start: return UiBoxLayout::MainAlignment::Start;
		case Alignment::Center: return UiBoxLayout::MainAlignment::Center;
		case Alignment::End: return UiBoxLayout::MainAlignment::End;
		case Alignment::SpaceBetween: return UiBoxLayout::MainAlignment::SpaceBetween;
		case Alignment::SpaceAround: return UiBoxLayout::MainAlignment::SpaceAround;
		case Alignment::SpaceEvenly: return UiBoxLayout::MainAlignment::SpaceEvenly;
		case Alignment::Stretch: return UiBoxLayout::MainAlignment::Start;
		default: return UiBoxLayout::MainAlignment::Start;
		}
	}

	static UiBoxLayout::SizeMode toBoxSizeMode(LayoutSizeMode m) {
		return (m == LayoutSizeMode::Natural) ? UiBoxLayout::SizeMode::Natural : UiBoxLayout::SizeMode::Weighted;
	}

	std::unique_ptr<IUiComponent> Column::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Vertical);
		layout->setSpacing(m_spacing);
		layout->setMainAlignment(toBoxMain(m_mainAxisAlignment));
		layout->setSizeMode(toBoxSizeMode(m_sizeMode));

		for (const auto& child : m_children) {
			if (!child) continue;
			auto comp = child->build();
			float weight = 0.0f;
			if (auto* expanded = dynamic_cast<const Expanded*>(child.get())) weight = expanded->flex();
			layout->addChild(comp.release(), weight, toBoxAlignment(m_crossAxisAlignment));
		}
		return decorate(std::move(layout));
	}

	std::unique_ptr<IUiComponent> Row::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Horizontal);
		layout->setSpacing(m_spacing);
		layout->setMainAlignment(toBoxMain(m_mainAxisAlignment));
		layout->setSizeMode(toBoxSizeMode(m_sizeMode));

		for (const auto& child : m_children) {
			if (!child) continue;
			auto comp = child->build();
			float weight = 0.0f;
			if (auto* expanded = dynamic_cast<const Expanded*>(child.get())) weight = expanded->flex();
			layout->addChild(comp.release(), weight, toBoxAlignment(m_crossAxisAlignment));
		}
		return decorate(std::move(layout));
	}

	class SpacerComponent : public IUiComponent {
	public:
		explicit SpacerComponent(int size) : m_size(size) {}
		void updateLayout(const QSize&) override {}
		void updateResourceContext(IconLoader&, QOpenGLFunctions*, float) override {}
		void append(Render::FrameData&) const override {}
		bool onMousePress(const QPoint&) override { return false; }
		bool onMouseMove(const QPoint&) override { return false; }
		bool onMouseRelease(const QPoint&) override { return false; }
		bool tick() override { return false; }
		QRect bounds() const override { return QRect(0, 0, m_size, m_size); }
	private:
		int m_size;
	};

	std::unique_ptr<IUiComponent> Spacer::build() const { return std::make_unique<SpacerComponent>(m_size); }

} // namespace UI