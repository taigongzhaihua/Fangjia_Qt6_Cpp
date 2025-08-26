#include "Decorators.h"
#include <algorithm>
#include <cmath>
#include <ILayoutable.hpp>
#include <memory>
#include <qcolor.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <utility>

namespace UI
{
	DecoratedBox::DecoratedBox(std::unique_ptr<IUiComponent> child, Props p)
		: m_child(std::move(child)), m_p(std::move(p))
	{
	}

	void DecoratedBox::setViewportRect(const QRect& r)
	{
		m_viewport = r;

		// 视觉外边距：只影响绘制区与内容区，不影响父布局分配的 viewport
		m_drawRect = m_viewport.adjusted(
			m_p.margin.left(), m_p.margin.top(),
			-m_p.margin.right(), -m_p.margin.bottom()
		);

		// 内容区：在绘制区基础上再扣除 border 和 padding
		const int bw = static_cast<int>(std::round(std::max(0.0f, m_p.borderW)));
		QRect inner = m_drawRect.adjusted(bw, bw, -bw, -bw);
		m_contentRect = inner.adjusted(
			m_p.padding.left(), m_p.padding.top(),
			-m_p.padding.right(), -m_p.padding.bottom()
		);

		// 下发给子项
		if (auto* c = dynamic_cast<IUiContent*>(m_child.get()))
		{
			c->setViewportRect(m_contentRect);
		}
		if (auto* l = dynamic_cast<ILayoutable*>(m_child.get()))
		{
			l->arrange(m_contentRect);
		}
	}

	QSize DecoratedBox::measure(const SizeConstraints& cs)
	{
		// 固定尺寸优先（不考虑 margin，margin 仅视觉）
		if (m_p.fixedSize.width() > 0 || m_p.fixedSize.height() > 0)
		{
			int w = (m_p.fixedSize.width() > 0 ? m_p.fixedSize.width() : 0);
			int h = (m_p.fixedSize.height() > 0 ? m_p.fixedSize.height() : 0);
			w = std::clamp(w, cs.minW, cs.maxW);
			h = std::clamp(h, cs.minH, cs.maxH);
			return QSize(w, h);
		}

		// 仅将 padding 算入测量（border/margin 仅视觉，不影响父布局）
		const int padW = m_p.padding.left() + m_p.padding.right();
		const int padH = m_p.padding.top() + m_p.padding.bottom();

		QSize inner(0, 0);
		if (auto* l = dynamic_cast<ILayoutable*>(m_child.get()))
		{
			SizeConstraints innerCs = cs;
			innerCs.minW = std::max(0, cs.minW - padW);
			innerCs.minH = std::max(0, cs.minH - padH);
			innerCs.maxW = std::max(0, cs.maxW - padW);
			innerCs.maxH = std::max(0, cs.maxH - padH);
			inner = l->measure(innerCs);
		}
		else if (m_child)
		{
			inner = m_child->bounds().size();
		}

		int w = inner.width() + padW;
		int h = inner.height() + padH;
		w = std::clamp(w, cs.minW, cs.maxW);
		h = std::clamp(h, cs.minH, cs.maxH);
		return QSize(w, h);
	}

	void DecoratedBox::arrange(const QRect& finalRect)
	{
		setViewportRect(finalRect);
	}

	void DecoratedBox::updateLayout(const QSize& windowSize)
	{
		if (m_child) m_child->updateLayout(windowSize);
	}

	void DecoratedBox::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio)
	{
		m_loader = &loader;
		m_gl = gl;
		m_dpr = std::max(0.5f, devicePixelRatio);
		if (m_child) m_child->updateResourceContext(loader, gl, devicePixelRatio);
	}

	void DecoratedBox::append(Render::FrameData& fd) const
	{
		if (!m_p.visible) return;

		// 裁剪到上级 viewport
		const QRectF clip = QRectF(m_viewport);

		// 先画边框（若启用）
		if (m_drawRect.isValid() && m_p.border.alpha() > 0 && m_p.borderW > 0.0f)
		{
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(m_drawRect),
				.radiusPx = (m_p.borderRadius > 0.0f ? m_p.borderRadius : m_p.bgRadius),
				.color = withOpacity(m_p.border, m_p.opacity),
				.clipRect = clip
				});
		}

		// 再画背景（若启用），要扣除边框厚度
		if (m_drawRect.isValid() && m_p.bg.alpha() > 0)
		{
			const int bw = static_cast<int>(std::round(std::max(0.0f, m_p.borderW)));
			const QRect bgRect = m_drawRect.adjusted(bw, bw, -bw, -bw);
			if (bgRect.isValid())
			{
				fd.roundedRects.push_back(Render::RoundedRectCmd{
					.rect = QRectF(bgRect),
					.radiusPx = std::max(0.0f, m_p.bgRadius - static_cast<float>(bw)),
					.color = withOpacity(m_p.bg, m_p.opacity),
					.clipRect = clip
					});
			}
		}

		// 子内容
		if (m_child) m_child->append(fd);
	}

	bool DecoratedBox::onMousePress(const QPoint& pos)
	{
		if (!m_p.visible || !m_viewport.contains(pos)) return false;
		if (m_child && m_child->onMousePress(pos)) return true;
		return false;
	}

	bool DecoratedBox::onMouseMove(const QPoint& pos)
	{
		if (!m_p.visible) return false;
		bool handled = false;
		if (m_child) handled = m_child->onMouseMove(pos) || handled;
		if (m_p.onHover)
		{
			const bool hov = m_viewport.contains(pos);
			if (hov != m_hover)
			{
				m_hover = hov;
				m_p.onHover(m_hover);
				handled = true;
			}
		}
		return handled;
	}

	bool DecoratedBox::onMouseRelease(const QPoint& pos)
	{
		if (!m_p.visible) return false;
		bool handled = false;
		if (m_child) handled = m_child->onMouseRelease(pos) || handled;
		if (m_p.onTap && m_viewport.contains(pos))
		{
			m_p.onTap();
			handled = true;
		}
		return handled;
	}

	bool DecoratedBox::tick()
	{
		return m_child && m_child->tick();
	}

	QRect DecoratedBox::bounds() const
	{
		// 若设置了 fixedSize，则作为 preferred size
		if (m_p.fixedSize.width() > 0 || m_p.fixedSize.height() > 0)
		{
			return QRect(
				0,
				0,
				std::max(0, m_p.fixedSize.width()),
				std::max(0, m_p.fixedSize.height())
			);
		}
		if (m_child)
		{
			const QRect cb = m_child->bounds();
			const int bw2 = static_cast<int>(std::round(std::max(0.0f, m_p.borderW))) * 2;
			return {
				0,0,
				cb.width() + m_p.padding.left() + m_p.padding.right() + bw2,
				cb.height() + m_p.padding.top() + m_p.padding.bottom() + bw2
			};
		}
		return {};
	}

	void DecoratedBox::onThemeChanged(bool isDark)
	{
		// 将主题变化继续传递给子项
		if (m_child) m_child->onThemeChanged(isDark);
	}


	QColor DecoratedBox::withOpacity(QColor c, float mul)
	{
		const int a = std::clamp(static_cast<int>(std::lround(c.alphaF() * mul * 255.0f)), 0, 255);
		c.setAlpha(a);
		return c;
	}
} // namespace UI
