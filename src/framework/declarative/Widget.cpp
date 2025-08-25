#include "Decorators.h"
#include "Widget.h"
#include <functional>
#include <memory>
#include <qcolor.h>
#include <qmargins.h>
#include <qsize.h>
#include <UiComponent.hpp>
#include <utility>

namespace UI {

	std::shared_ptr<Widget> Widget::padding(int all) { m_decorations.padding = QMargins(all, all, all, all); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::padding(int h, int v) { m_decorations.padding = QMargins(h, v, h, v); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::padding(int l, int t, int r, int b) { m_decorations.padding = QMargins(l, t, r, b); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::margin(int all) { m_decorations.margin = QMargins(all, all, all, all); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::margin(int h, int v) { m_decorations.margin = QMargins(h, v, h, v); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::margin(int l, int t, int r, int b) { m_decorations.margin = QMargins(l, t, r, b); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::background(QColor c, float r) { m_decorations.backgroundColor = c; m_decorations.backgroundRadius = r; return self<Widget>(); }
	std::shared_ptr<Widget> Widget::border(QColor c, float w, float r) { m_decorations.borderColor = c; m_decorations.borderWidth = w; m_decorations.borderRadius = r; return self<Widget>(); }
	std::shared_ptr<Widget> Widget::size(int w, int h) { m_decorations.fixedSize = QSize(w, h); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::width(int w) { m_decorations.fixedSize.setWidth(w); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::height(int h) { m_decorations.fixedSize.setHeight(h); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::visible(bool v) { m_decorations.isVisible = v; return self<Widget>(); }
	std::shared_ptr<Widget> Widget::opacity(float o) { m_decorations.opacity = o; return self<Widget>(); }
	std::shared_ptr<Widget> Widget::onTap(std::function<void()> h) { m_decorations.onTap = std::move(h); return self<Widget>(); }
	std::shared_ptr<Widget> Widget::onHover(std::function<void(bool)> h) { m_decorations.onHover = std::move(h); return self<Widget>(); }

	void Widget::applyDecorations(IUiComponent* /*component*/) const {
		// 保留给可以直接吃属性的组件（比如 UiBoxLayout 可直接 setMargins/setBackground）
		// 通用场景交给 decorate()
	}

	std::unique_ptr<IUiComponent> Widget::decorate(std::unique_ptr<IUiComponent> inner) const {
		// 若没有任何装饰，直接返回
		const bool need = (m_decorations.backgroundColor.alpha() > 0) ||
			(m_decorations.padding != QMargins()) ||
			(m_decorations.fixedSize.width() > 0 || m_decorations.fixedSize.height() > 0) ||
			(m_decorations.opacity < 0.999f) ||
			(!m_decorations.isVisible) ||
			(bool)m_decorations.onTap || (bool)m_decorations.onHover ||
			(m_decorations.borderColor.alpha() > 0);
		if (!need) return inner;

		DecoratedBox::Props p;
		p.padding = m_decorations.padding;
		p.margin = m_decorations.margin;
		p.bg = m_decorations.backgroundColor;
		p.bgRadius = m_decorations.backgroundRadius;
		p.border = m_decorations.borderColor;
		p.borderW = m_decorations.borderWidth;
		p.borderRadius = m_decorations.borderRadius;
		p.fixedSize = m_decorations.fixedSize;
		p.visible = m_decorations.isVisible;
		p.opacity = m_decorations.opacity;
		p.onTap = m_decorations.onTap;
		p.onHover = m_decorations.onHover;

		return std::make_unique<DecoratedBox>(std::move(inner), std::move(p));
	}

} // namespace UI