#pragma once
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "ILayoutable.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <memory>
#include <qcolor.h>
#include <qmargins.h>
#include <qnamespace.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>
#include <utility>

namespace UI {

	// 通用装饰器：将 Widget 的 Decorations 落地到一个 IUiComponent 包裹层
	class DecoratedBox : public IUiComponent, public IUiContent, public ILayoutable {
	public:
		struct Props {
			QMargins padding{ 0,0,0,0 };
			QMargins margin{ 0,0,0,0 }; // 视觉外边距，不参与父布局测量
			QColor   bg{ Qt::transparent };
			float    bgRadius{ 0.0f };
			QColor   border{ Qt::transparent };
			float    borderW{ 0.0f };
			float    borderRadius{ 0.0f };
			QSize    fixedSize{ -1, -1 };
			bool     visible{ true };
			float    opacity{ 1.0f };
			std::function<void()> onTap;
			std::function<void(bool)> onHover;
		};

		DecoratedBox(std::unique_ptr<IUiComponent> child, Props p);

		// IUiContent
		void setViewportRect(const QRect& r) override;

		// ILayoutable
		QSize measure(const SizeConstraints& cs) override;
		void arrange(const QRect& finalRect) override;

		// IUiComponent
		void updateLayout(const QSize& windowSize) override;
		void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
		void append(Render::FrameData& fd) const override;

		bool onMousePress(const QPoint& pos) override;
		bool onMouseMove(const QPoint& pos) override;
		bool onMouseRelease(const QPoint& pos) override;

		bool tick() override;
		QRect bounds() const override;

	private:
		static QColor withOpacity(QColor c, float mul);

	private:
		std::unique_ptr<IUiComponent> m_child;
		Props   m_p;
		QRect   m_viewport;
		QRect   m_contentRect;

		bool    m_hover{ false };
		IconLoader* m_loader{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
	};

} // namespace UI