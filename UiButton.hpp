#pragma once
#include "RenderData.hpp"
#include <algorithm>
#include <cmath>
#include <functional>
#include <qcolor.h>
#include <qpoint.h>
#include <qrect.h>
#include <qtypes.h>
#include <utility>

namespace Ui
{
	// 回调签名：在给定按钮矩形中绘制图标
	// iconColor：已根据按钮不透明度处理后的颜色
	// opacity  ：按钮整体不透明度（可用于细化图标透明度）
	using IconPainter = std::function<void(const QRectF& rect, Render::FrameData& fd, const QColor& iconColor,
		float opacity)>;

	class Button
	{
	public:
		void setBaseRect(const QRect& r) { m_baseRect = r; }
		[[nodiscard]] const QRect& baseRect() const noexcept { return m_baseRect; }

		void setOffset(const QPointF& off) { m_offset = off; }
		[[nodiscard]] const QPointF& offset() const noexcept { return m_offset; }

		[[nodiscard]] QRectF visualRectF() const
		{
			return {
				m_baseRect.left() + m_offset.x(),
				m_baseRect.top() + m_offset.y(),
				static_cast<qreal>(m_baseRect.width()),
				static_cast<qreal>(m_baseRect.height())
			};
		}

		void setOpacity(const float a) { m_opacity = std::clamp(a, 0.0f, 1.0f); }
		[[nodiscard]] float opacity() const noexcept { return m_opacity; }

		void setEnabled(const bool e)
		{
			m_enabled = e;
			if (!m_enabled)
			{
				m_hovered = false;
				m_pressed = false;
			}
		}

		[[nodiscard]] bool enabled() const noexcept { return m_enabled; }

		void setCornerRadius(const float r) { m_corner = r; }

		void setPalette(const QColor& bg, const QColor& bgHover, const QColor& bgPressed, const QColor& icon)
		{
			m_bg = bg;
			m_bgHover = bgHover;
			m_bgPressed = bgPressed;
			m_icon = icon;
		}

		void setIconPainter(IconPainter p) { m_iconPainter = std::move(p); }

		// 事件处理：返回是否消耗
		bool onMousePress(const QPoint& pos)
		{
			if (!m_enabled) return false;
			if (hit(pos))
			{
				m_pressed = true;
				return true;
			}
			return false;
		}

		bool onMouseMove(const QPoint& pos)
		{
			const bool old = m_hovered;
			m_hovered = m_enabled && hit(pos);
			return old != m_hovered;
		}

		bool onMouseRelease(const QPoint& pos, bool& clicked)
		{
			clicked = false;
			if (!m_enabled)
			{
				m_pressed = false;
				return false;
			}
			const bool wasPressed = m_pressed;
			m_pressed = false;
			if (wasPressed && hit(pos))
			{
				clicked = true;
				return true;
			}
			return wasPressed;
		}

		[[nodiscard]] bool hovered() const noexcept { return m_hovered; }
		[[nodiscard]] bool pressed() const noexcept { return m_pressed; }

		void append(Render::FrameData& fd) const
		{
			if (m_opacity <= 0.001f) return; // 完全透明不绘制
			const QRectF r = visualRectF();
			const QColor bg = withOpacity(backgroundForState(), m_opacity);
			fd.roundedRects.push_back(Render::RoundedRectCmd{ .rect = r, .radiusPx = m_corner, .color = bg });

			if (m_iconPainter)
			{
				const QColor ic = withOpacity(m_icon, m_opacity);
				m_iconPainter(r, fd, ic, m_opacity);
			}
		}

	private:
		[[nodiscard]] bool hit(const QPoint& pos) const { return visualRectF().toRect().contains(pos); }

		static QColor withOpacity(QColor c, const float mul)
		{
			const int a = std::clamp(
				static_cast<int>(std::lround(static_cast<float>(c.alpha()) * std::clamp(mul, 0.0f, 1.0f))), 0, 255);
			c.setAlpha(a);
			return c;
		}

		[[nodiscard]] QColor backgroundForState() const
		{
			if (m_pressed) return m_bgPressed;
			if (m_hovered) return m_bgHover;
			return m_bg;
		}

	private:
		QRect m_baseRect;
		QPointF m_offset{ 0, 0 };
		float m_opacity{ 1.0f };
		float m_corner{ 8.0f };
		bool m_enabled{ true };
		bool m_hovered{ false };
		bool m_pressed{ false };

		QColor m_bg, m_bgHover, m_bgPressed, m_icon;
		IconPainter m_iconPainter;
	};
} // namespace Ui
