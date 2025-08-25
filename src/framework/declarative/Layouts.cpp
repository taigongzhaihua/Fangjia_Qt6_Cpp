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

	// 转换交叉轴对齐方式（已有）
	static UiBoxLayout::Alignment toBoxAlignment(Alignment align) {
		switch (align) {
		case Alignment::Start: return UiBoxLayout::Alignment::Start;
		case Alignment::Center: return UiBoxLayout::Alignment::Center;
		case Alignment::End: return UiBoxLayout::Alignment::End;
		case Alignment::Stretch: return UiBoxLayout::Alignment::Stretch;
		default: return UiBoxLayout::Alignment::Start;
		}
	}

	// 新增：转换主轴对齐方式
	static UiBoxLayout::MainAlignment toBoxMain(Alignment align) {
		switch (align) {
		case Alignment::Start: return UiBoxLayout::MainAlignment::Start;
		case Alignment::Center: return UiBoxLayout::MainAlignment::Center;
		case Alignment::End: return UiBoxLayout::MainAlignment::End;
		case Alignment::SpaceBetween: return UiBoxLayout::MainAlignment::SpaceBetween;
		case Alignment::SpaceAround: return UiBoxLayout::MainAlignment::SpaceAround;
		case Alignment::SpaceEvenly: return UiBoxLayout::MainAlignment::SpaceEvenly;
		case Alignment::Stretch: return UiBoxLayout::MainAlignment::Start; // Stretch 不用于主轴，退化为 Start
		default: return UiBoxLayout::MainAlignment::Start;
		}
	}

	std::unique_ptr<IUiComponent> Column::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Vertical);
		layout->setSpacing(m_spacing);
		// 主轴对齐（垂直方向）
		layout->setMainAlignment(toBoxMain(m_mainAxisAlignment));

		for (const auto& child : m_children) {
			if (!child) continue;
			auto comp = child->build();
			float weight = 0.0f;
			if (auto* expanded = dynamic_cast<const Expanded*>(child.get())) weight = expanded->flex();
			layout->addChild(comp.release(), weight, toBoxAlignment(m_crossAxisAlignment));
		}
		// 直接落入布局
		return decorate(std::move(layout));
	}

	std::unique_ptr<IUiComponent> Row::build() const {
		auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Horizontal);
		layout->setSpacing(m_spacing);
		// 主轴对齐（水平方向）
		layout->setMainAlignment(toBoxMain(m_mainAxisAlignment));

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