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
#include <RenderUtils.hpp>
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

		// 内容区：在绘制区基础上再扣除边框和内边距
		const int bw = static_cast<int>(std::round(std::max(0.0f, m_p.borderW)));
		const QRect inner = m_drawRect.adjusted(bw, bw, -bw, -bw);
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
			return { w, h };
		}

		// 仅将 padding 算入测量（border/margin 仅视觉，不影响父布局）
		const int padW = m_p.padding.left() + m_p.padding.right();
		const int padH = m_p.padding.top() + m_p.padding.bottom();

		QSize inner(0, 0);
		if (auto* l = dynamic_cast<ILayoutable*>(m_child.get()))
		{
			SizeConstraints innerCs;
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
		return { w, h };
	}

	void DecoratedBox::arrange(const QRect& finalRect)
	{
		setViewportRect(finalRect);
	}

	void DecoratedBox::updateLayout(const QSize& windowSize)
	{
		if (m_child) m_child->updateLayout(windowSize);
	}

	void DecoratedBox::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float devicePixelRatio)
	{
		m_cache = &cache;
		m_gl = gl;
		m_dpr = std::max(0.5f, devicePixelRatio);
		if (m_child) m_child->updateResourceContext(cache, gl, devicePixelRatio);
	}

	QColor DecoratedBox::effectiveBg() const
	{
		if (m_p.useThemeBg) return m_isDark ? m_p.bgDark : m_p.bgLight;
		return m_p.bg;
	}

	QColor DecoratedBox::effectiveBorder() const
	{
		if (m_p.useThemeBorder) return m_isDark ? m_p.borderDark : m_p.borderLight;
		return m_p.border;
	}

	void DecoratedBox::append(Render::FrameData& fd) const
	{
		if (!m_p.visible) return;

		// 裁剪到上级 viewport
		const auto clip = QRectF(m_viewport);

		const QColor borderColor = effectiveBorderForState();
		const QColor bgColor = effectiveBgForState();

		// 先画边框（若启用）
		if (m_drawRect.isValid() && borderColor.alpha() > 0 && m_p.borderW > 0.0f)
		{
			fd.roundedRects.push_back(Render::RoundedRectCmd{
				.rect = QRectF(m_drawRect),
				.radiusPx = (m_p.borderRadius > 0.0f ? m_p.borderRadius : m_p.bgRadius),
				.color = withOpacity(borderColor, m_p.opacity),
				.clipRect = clip
				});
		}

		// 再画背景（若启用），要扣除边框厚度
		if (m_drawRect.isValid() && bgColor.alpha() > 0)
		{
			const int bw = static_cast<int>(std::round(std::max(0.0f, m_p.borderW)));
			const QRect bgRect = m_drawRect.adjusted(bw, bw, -bw, -bw);
			if (bgRect.isValid())
			{
				fd.roundedRects.push_back(Render::RoundedRectCmd{
					.rect = QRectF(bgRect),
					.radiusPx = std::max(0.0f, m_p.bgRadius - static_cast<float>(bw)),
					.color = withOpacity(bgColor, m_p.opacity),
					.clipRect = clip
					});
			}
		}

		// 子内容追加 + 内容区裁剪
		if (m_child) {
			const int rr0 = static_cast<int>(fd.roundedRects.size());
			const int im0 = static_cast<int>(fd.images.size());

			m_child->append(fd);

			RenderUtils::applyParentClip(fd, rr0, im0, QRectF(m_contentRect));
		}
	}

	bool DecoratedBox::onMousePress(const QPoint& pos)
	{
		if (!m_p.visible) return false;
		
		// 首先尝试让子组件处理
		if (m_child && m_child->onMousePress(pos)) return true;
		
		// 如果子组件没有处理，且我们有onTap回调，检查是否在可点击区域内
		// 对于交互元素，使用完整的viewport而不是drawRect，这样margin区域也可以点击
		if (m_p.onTap && m_viewport.contains(pos))
		{
			m_pressed = true;
			return true; // 声明处理此事件
		}
		
		return false;
	}

	bool DecoratedBox::onMouseMove(const QPoint& pos)
	{
		if (!m_p.visible) return false;
		bool handled = false;
		if (m_child) handled = m_child->onMouseMove(pos) || handled;
		
		// Update hover state for interactive styling (regardless of onHover callback)
		if (m_p.onTap || m_p.onHover)
		{
			// 对于交互元素，hover也应该使用完整的viewport区域
			const bool hov = m_viewport.contains(pos);
			if (hov != m_hover)
			{
				m_hover = hov;
				if (m_p.onHover) m_p.onHover(m_hover);
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
		
		// 检查是否为有效的点击：之前按下且释放时仍在区域内
		// 对于交互元素，使用完整的viewport而不是drawRect，保持与onMousePress一致
		if (m_p.onTap && m_pressed && m_viewport.contains(pos))
		{
			m_p.onTap();
			handled = true;
		}
		
		// 重置按下状态
		m_pressed = false;
		
		return handled;
	}

	bool DecoratedBox::onWheel(const QPoint& pos, const QPoint& angleDelta)
	{
		if (!m_p.visible || !m_viewport.contains(pos)) return false;
		return m_child ? m_child->onWheel(pos, angleDelta) : false;
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
			return {
				0,
				0,
				std::max(0, m_p.fixedSize.width()),
				std::max(0, m_p.fixedSize.height())
			};
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

	void DecoratedBox::onThemeChanged(const bool isDark)
	{
		// 先更新自身主题，再传递给子项
		m_isDark = isDark;
		if (m_child) m_child->onThemeChanged(isDark);
	}

	QColor DecoratedBox::withOpacity(QColor c, const float mul)
	{
		const int a = std::clamp(static_cast<int>(std::lround(c.alphaF() * mul * 255.0f)), 0, 255);
		c.setAlpha(a);
		return c;
	}

	QColor DecoratedBox::effectiveBgForState() const
	{
		// Check for explicit interactive background colors first
		if (m_p.useInteractiveBg)
		{
			if (m_pressed) return m_p.bgPressed;
			if (m_hover) return m_p.bgHover;
		}
		else if (m_p.useThemeInteractiveBg)
		{
			if (m_pressed) return m_isDark ? m_p.bgPressedDark : m_p.bgPressedLight;
			if (m_hover) return m_isDark ? m_p.bgHoverDark : m_p.bgHoverLight;
		}
		// Check for auto interactive (when onTap is set and enableAutoInteractive is true)
		else if (m_p.enableAutoInteractive && m_p.onTap)
		{
			if (m_pressed) return defaultPressedBg();
			if (m_hover) return defaultHoverBg();
		}

		// Fall back to regular background
		return effectiveBg();
	}

	QColor DecoratedBox::effectiveBorderForState() const
	{
		// Check for explicit interactive border colors first
		if (m_p.useInteractiveBorder)
		{
			if (m_pressed) return m_p.borderPressed;
			if (m_hover) return m_p.borderHover;
		}
		else if (m_p.useThemeInteractiveBorder)
		{
			if (m_pressed) return m_isDark ? m_p.borderPressedDark : m_p.borderPressedLight;
			if (m_hover) return m_isDark ? m_p.borderHoverDark : m_p.borderHoverLight;
		}

		// Fall back to regular border (no auto interactive for borders)
		return effectiveBorder();
	}

	QColor DecoratedBox::defaultHoverBg() const
	{
		// Default hover colors matching NavRail/TreeList palette
		if (m_isDark)
		{
			// Dark theme: hover rgba(255,255,255,18%)
			return QColor(255, 255, 255, static_cast<int>(255 * 0.18));
		}
		else
		{
			// Light theme: hover rgba(0,0,0,14%)
			return QColor(0, 0, 0, static_cast<int>(255 * 0.14));
		}
	}

	QColor DecoratedBox::defaultPressedBg() const
	{
		// Default pressed colors matching NavRail/TreeList palette
		if (m_isDark)
		{
			// Dark theme: pressed rgba(255,255,255,30%)
			return QColor(255, 255, 255, static_cast<int>(255 * 0.30));
		}
		else
		{
			// Light theme: pressed rgba(0,0,0,26%)
			return QColor(0, 0, 0, static_cast<int>(255 * 0.26));
		}
	}
} // namespace UI