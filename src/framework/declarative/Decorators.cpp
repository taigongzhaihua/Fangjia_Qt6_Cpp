#include "Decorators.h"

namespace UI {

	DecoratedBox::DecoratedBox(std::unique_ptr<IUiComponent> child, Props p)
		: m_child(std::move(child)), m_p(std::move(p)) {
	}

	void DecoratedBox::setViewportRect(const QRect& r) {
		m_viewport = r;
		m_contentRect = m_viewport.adjusted(
			m_p.padding.left(), m_p.padding.top(),
			-m_p.padding.right(), -m_p.padding.bottom()
		);
		if (auto* c = dynamic_cast<IUiContent*>(m_child.get())) {
			c->setViewportRect(m_contentRect);
		}
		if (auto* l = dynamic_cast<ILayoutable*>(m_child.get())) {
			l->arrange(m_contentRect);
		}
	}

	QSize DecoratedBox::measure(const SizeConstraints& cs) {
		// 固定尺寸优先
		if (m_p.fixedSize.width() > 0 || m_p.fixedSize.height() > 0) {
			int w = (m_p.fixedSize.width() > 0 ? m_p.fixedSize.width() : 0);
			int h = (m_p.fixedSize.height() > 0 ? m_p.fixedSize.height() : 0);
			w = std::clamp(w, cs.minW, cs.maxW);
			h = std::clamp(h, cs.minH, cs.maxH);
			return QSize(w, h);
		}

		QSize inner(0, 0);
		if (auto* l = dynamic_cast<ILayoutable*>(m_child.get())) {
			SizeConstraints innerCs = cs;
			innerCs.minW = std::max(0, cs.minW - (m_p.padding.left() + m_p.padding.right()));
			innerCs.minH = std::max(0, cs.minH - (m_p.padding.top() + m_p.padding.bottom()));
			innerCs.maxW = std::max(0, cs.maxW - (m_p.padding.left() + m_p.padding.right()));
			innerCs.maxH = std::max(0, cs.maxH - (m_p.padding.top() + m_p.padding.bottom()));
			inner = l->measure(innerCs);
		}
		else if (m_child) {
			inner = m_child->bounds().size();
		}

		int w = inner.width() + m_p.padding.left() + m_p.padding.right();
		int h = inner.height() + m_p.padding.top() + m_p.padding.bottom();
		w = std::clamp(w, cs.minW, cs.maxW);
		h = std::clamp(h, cs.minH, cs.maxH);
		return QSize(w, h);
	}

	void DecoratedBox::arrange(const QRect& finalRect) {
		setViewportRect(finalRect);
	}

	void DecoratedBox::updateLayout(const QSize& windowSize) {
		if (m_child) m_child->updateLayout(windowSize);
	}

	void DecoratedBox::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) {
		m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
		if (m_child) m_child->updateResourceContext(loader, gl, devicePixelRatio);
	}

	void DecoratedBox::append(Render::FrameData& fd) const {
		if (!m_p.visible) return;
		if (m_p.bg.alpha() > 0 && m_viewport.isValid()) {
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(m_viewport),
				.radiusPx = m_p.bgRadius,
				.color = withOpacity(m_p.bg, m_p.opacity),
				.clipRect = QRectF(m_viewport)
				});
		}
		if (m_child) m_child->append(fd);
	}

	bool DecoratedBox::onMousePress(const QPoint& pos) {
		if (!m_p.visible || !m_viewport.contains(pos)) return false;
		if (m_child && m_child->onMousePress(pos)) return true;
		return false;
	}

	bool DecoratedBox::onMouseMove(const QPoint& pos) {
		if (!m_p.visible) return false;
		bool handled = false;
		if (m_child) handled = m_child->onMouseMove(pos) || handled;
		if (m_p.onHover) {
			const bool hov = m_viewport.contains(pos);
			if (hov != m_hover) {
				m_hover = hov;
				m_p.onHover(m_hover);
				handled = true;
			}
		}
		return handled;
	}

	bool DecoratedBox::onMouseRelease(const QPoint& pos) {
		if (!m_p.visible) return false;
		bool handled = false;
		if (m_child) handled = m_child->onMouseRelease(pos) || handled;
		if (m_p.onTap && m_viewport.contains(pos)) {
			m_p.onTap();
			handled = true;
		}
		return handled;
	}

	bool DecoratedBox::tick() {
		return m_child && m_child->tick();
	}

	QRect DecoratedBox::bounds() const {
		// 若设置了 fixedSize，则作为 preferred size
		if (m_p.fixedSize.width() > 0 || m_p.fixedSize.height() > 0) {
			return QRect(0, 0, std::max(0, m_p.fixedSize.width()), std::max(0, m_p.fixedSize.height()));
		}
		if (m_child) {
			const QRect cb = m_child->bounds();
			return QRect(0, 0,
				cb.width() + m_p.padding.left() + m_p.padding.right(),
				cb.height() + m_p.padding.top() + m_p.padding.bottom());
		}
		return QRect();
	}

	QColor DecoratedBox::withOpacity(QColor c, float mul) {
		const int a = std::clamp(int(std::lround(c.alphaF() * mul * 255.0f)), 0, 255);
		c.setAlpha(a);
		return c;
	}

} // namespace UI