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

	// 转换对齐方式
	static UiBoxLayout::Alignment toBoxAlignment(Alignment align) {
		switch (align) {
		case Alignment::Start: return UiBoxLayout::Alignment::Start;
		case Alignment::Center: return UiBoxLayout::Alignment::Center;
		case Alignment::End: return UiBoxLayout::Alignment::End;
		case Alignment::Stretch: return UiBoxLayout::Alignment::Stretch;
		default: return UiBoxLayout::Alignment::Start;
		}
	}

	std::unique_ptr<IUiComponent> Column::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Vertical);
		layout->setSpacing(m_spacing);
		for (const auto& child : m_children) {
			if (!child) continue;
			auto comp = child->build();
			float weight = 0.0f;
			if (auto* expanded = dynamic_cast<const Expanded*>(child.get())) weight = expanded->flex();
			layout->addChild(comp.release(), weight, toBoxAlignment(m_crossAxisAlignment));
		}
		// 直接落入布局
		// padding/background 在 decorate 中也能处理，这里优先走原生布局能力
		return decorate(std::move(layout));
	}

	std::unique_ptr<IUiComponent> Row::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Horizontal);
		layout->setSpacing(m_spacing);
		for (const auto& child : m_children) {
			if (!child) continue;
			auto comp = child->build();
			float weight = 0.0f;
			if (auto* expanded = dynamic_cast<const Expanded*>(child.get())) weight = expanded->flex();
			layout->addChild(comp.release(), weight, toBoxAlignment(m_crossAxisAlignment));
		}
		return decorate(std::move(layout));
	}

	std::unique_ptr<IUiComponent> Stack::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Vertical);
		for (const auto& child : m_children) {
			if (!child) continue;
			auto comp = child->build();
			layout->addChild(comp.release(), 1.0f, toBoxAlignment(m_alignment));
		}
		return decorate(std::move(layout));
	}

	// Spacer实现
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
		QRect bounds() const override {
			return QRect(0, 0, m_size, m_size);
		}

	private:
		int m_size;
	};

	std::unique_ptr<IUiComponent> Spacer::build() const {
		return std::make_unique<SpacerComponent>(m_size);
	}

} // namespace UI