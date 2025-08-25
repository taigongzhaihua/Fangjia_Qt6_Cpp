#pragma once
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include <functional>
#include <memory>
#include <qcolor.h>
#include <qmargins.h>
#include <qsize.h>
#include <vector>

namespace UI {

	// 前向声明
	class Widget;
	using WidgetPtr = std::shared_ptr<Widget>;
	using WidgetList = std::vector<WidgetPtr>;
	using WidgetBuilder = std::function<WidgetPtr()>;

	// 基础Widget类 - 修改链式调用实现
	class Widget : public std::enable_shared_from_this<Widget> {
	public:
		virtual ~Widget() = default;

		// 构建实际的UI组件
		virtual std::unique_ptr<IUiComponent> build() const = 0;

		// 修改：链式调用时安全获取 shared_ptr
		template<typename Derived>
		std::shared_ptr<Derived> self() {
			// 安全地尝试获取 shared_ptr
			try {
				return std::static_pointer_cast<Derived>(shared_from_this());
			}
			catch (const std::bad_weak_ptr&) {
				// 如果失败，返回空指针（调用者需要检查）
				return nullptr;
			}
		}

		// 链式调用的修饰器 - 使用 self() 方法
		std::shared_ptr<Widget> padding(int all);
		std::shared_ptr<Widget> padding(int horizontal, int vertical);
		std::shared_ptr<Widget> padding(int left, int top, int right, int bottom);

		std::shared_ptr<Widget> margin(int all);
		std::shared_ptr<Widget> margin(int horizontal, int vertical);
		std::shared_ptr<Widget> margin(int left, int top, int right, int bottom);

		std::shared_ptr<Widget> background(QColor color, float radius = 0.0f);
		std::shared_ptr<Widget> border(QColor color, float width = 1.0f, float radius = 0.0f);

		std::shared_ptr<Widget> size(int width, int height);
		std::shared_ptr<Widget> width(int w);
		std::shared_ptr<Widget> height(int h);

		std::shared_ptr<Widget> visible(bool v);
		std::shared_ptr<Widget> opacity(float o);

		// 事件处理
		std::shared_ptr<Widget> onTap(std::function<void()> handler);
		std::shared_ptr<Widget> onHover(std::function<void(bool)> handler);

	protected:
		// 装饰器数据
		struct Decorations {
			QMargins padding{ 0, 0, 0, 0 };
			QMargins margin{ 0, 0, 0, 0 };
			QColor backgroundColor{ Qt::transparent };
			float backgroundRadius{ 0.0f };
			QColor borderColor{ Qt::transparent };
			float borderWidth{ 0.0f };
			float borderRadius{ 0.0f };
			QSize fixedSize{ -1, -1 };
			bool isVisible{ true };
			float opacity{ 1.0f };
			std::function<void()> onTap;
			std::function<void(bool)> onHover;
		} m_decorations;

		// 应用装饰器到组件
		void applyDecorations(IUiComponent* component) const;
	};

	// 修改工厂函数，确保立即包装为 shared_ptr
	template<typename T, typename... Args>
	std::shared_ptr<T> make_widget(Args&&... args) {
		return std::make_shared<T>(std::forward<Args>(args)...);
	}

} // namespace UI