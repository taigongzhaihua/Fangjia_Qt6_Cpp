#include "Widget.h"
#include <functional>
#include <memory>
#include <qcolor.h>
#include <qmargins.h>
#include <qsize.h>
#include <UiComponent.hpp>
#include <utility>

namespace UI {

	std::shared_ptr<Widget> Widget::padding(int all) {
		m_decorations.padding = QMargins(all, all, all, all);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::padding(int horizontal, int vertical) {
		m_decorations.padding = QMargins(horizontal, vertical, horizontal, vertical);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::padding(int left, int top, int right, int bottom) {
		m_decorations.padding = QMargins(left, top, right, bottom);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::margin(int all) {
		m_decorations.margin = QMargins(all, all, all, all);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::margin(int horizontal, int vertical) {
		m_decorations.margin = QMargins(horizontal, vertical, horizontal, vertical);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::margin(int left, int top, int right, int bottom) {
		m_decorations.margin = QMargins(left, top, right, bottom);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::background(QColor color, float radius) {
		m_decorations.backgroundColor = color;
		m_decorations.backgroundRadius = radius;
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::border(QColor color, float width, float radius) {
		m_decorations.borderColor = color;
		m_decorations.borderWidth = width;
		m_decorations.borderRadius = radius;
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::size(int width, int height) {
		m_decorations.fixedSize = QSize(width, height);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::width(int w) {
		m_decorations.fixedSize.setWidth(w);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::height(int h) {
		m_decorations.fixedSize.setHeight(h);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::visible(bool v) {
		m_decorations.isVisible = v;
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::opacity(float o) {
		m_decorations.opacity = o;
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::onTap(std::function<void()> handler) {
		m_decorations.onTap = std::move(handler);
		return shared_from_this();
	}

	std::shared_ptr<Widget> Widget::onHover(std::function<void(bool)> handler) {
		m_decorations.onHover = std::move(handler);
		return shared_from_this();
	}

	void Widget::applyDecorations(IUiComponent* component) const {
		// TODO: 应用装饰器到实际组件
		// 这需要根据具体的组件类型来实现
	}

} // namespace UI