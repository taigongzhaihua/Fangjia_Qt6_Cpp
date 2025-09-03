#pragma once
#include "UiComponent.hpp"
#include <functional>
#include <memory>
#include <qcolor.h>
#include <qmargins.h>
#include <qsize.h>
#include <vector>

namespace UI {

	class Widget : public std::enable_shared_from_this<Widget> {
	public:
		virtual ~Widget() = default;
		virtual std::unique_ptr<IUiComponent> build() const = 0;

		template<typename Derived>
		std::shared_ptr<Derived> self() {
			try { return std::static_pointer_cast<Derived>(shared_from_this()); }
			catch (...) { return nullptr; }
		}

		// 修饰器 API（略）...
		std::shared_ptr<Widget> padding(int all);
		std::shared_ptr<Widget> padding(int horizontal, int vertical);
		std::shared_ptr<Widget> padding(int left, int top, int right, int bottom);
		std::shared_ptr<Widget> margin(int all);
		std::shared_ptr<Widget> margin(int horizontal, int vertical);
		std::shared_ptr<Widget> margin(int left, int top, int right, int bottom);
		std::shared_ptr<Widget> background(QColor color, float radius = 0.0f);
		std::shared_ptr<Widget> border(QColor color, float width = 1.0f, float radius = 0.0f);
		std::shared_ptr<Widget> shadow(QColor color, float blurPx, QPoint offset, float spreadPx = 0.0f);
		std::shared_ptr<Widget> size(int width, int height);
		std::shared_ptr<Widget> width(int w);
		std::shared_ptr<Widget> height(int h);
		std::shared_ptr<Widget> visible(bool v);
		std::shared_ptr<Widget> opacity(float o);
		std::shared_ptr<Widget> onTap(std::function<void()> handler);
		std::shared_ptr<Widget> onHover(std::function<void(bool)> handler);

	protected:
		struct Decorations {
			QMargins padding{ 0,0,0,0 };
			QMargins margin{ 0,0,0,0 };
			QColor   backgroundColor{ Qt::transparent };
			float    backgroundRadius{ 0.0f };
			QColor   borderColor{ Qt::transparent };
			float    borderWidth{ 0.0f };
			float    borderRadius{ 0.0f };
			
			// Shadow properties / 阴影属性
			bool     useShadow{ false };
			QColor   shadowColor{ 0, 0, 0, 160 };
			float    shadowBlurPx{ 0.0f };
			QPoint   shadowOffset{ 0, 0 };
			float    shadowSpreadPx{ 0.0f };
			
			QSize    fixedSize{ -1,-1 };
			bool     isVisible{ true };
			float    opacity{ 1.0f };
			std::function<void()> onTap;
			std::function<void(bool)> onHover;
		} m_decorations;

		// 仍保留（对可以直接改属性的组件在里面直接设置）
		void applyDecorations(IUiComponent* component) const;

		// 新增：统一包裹装饰器
		std::unique_ptr<IUiComponent> decorate(std::unique_ptr<IUiComponent> inner) const;
	};

	template<typename T, typename... Args>
	std::shared_ptr<T> make_widget(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

	using WidgetPtr = std::shared_ptr<Widget>;
	using WidgetList = std::vector<WidgetPtr>;

} // namespace UI