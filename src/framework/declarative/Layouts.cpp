#include "Layouts.h"
#include <IconLoader.h>
#include <memory>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>
#include <UiComponent.hpp>
#include <UiPanel.h>
#include <utility>

namespace UI {

	std::unique_ptr<IUiComponent> Panel::build() const {
		auto layout = std::make_unique<UiPanel>(m_orient);
		layout->setSpacing(m_spacing);
		layout->setMargins(m_margins);
		layout->setPadding(m_padding);

		// 逐个添加子项（交叉轴对齐统一使用当前 Panel 的 crossAxisAlignment）
		const auto cross = toCross(m_crossAlign);
		for (const auto& child : m_children) {
			if (!child) continue;
			auto comp = child->build();
			layout->addChild(comp.release(), cross);
		}

		// Panel 自身的装饰器（背景/圆角/内边距等）交给通用 DecoratedBox（如有设置）
		return decorate(std::move(layout));
	}

	// Spacer -> 固定大小的空组件（对 Panel 来说就是占位/留白）
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
		[[nodiscard]] QRect bounds() const override { return { 0, 0, m_size, m_size }; }
		void onThemeChanged(bool) override {}
	private:
		int m_size;
	};

	std::unique_ptr<IUiComponent> Spacer::build() const { return std::make_unique<SpacerComponent>(m_size); }

} // namespace UI