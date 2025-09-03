#pragma once
#include <functional>
#include <memory>

#include "ILayoutable.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include <qcolor.h>
#include <qmargins.h>
#include <qnamespace.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include "RenderData.hpp"

namespace UI {

	class DecoratedBox : public IUiComponent, public IUiContent, public ILayoutable {
	public:
		struct Props {
			QMargins padding{ 0,0,0,0 };
			QMargins margin{ 0,0,0,0 };

			// 静态背景/圆角（不随主题）
			QColor   bg{ Qt::transparent };
			float    bgRadius{ 0.0f };

			// 静态边框（不随主题）
			QColor   border{ Qt::transparent };
			float    borderW{ 0.0f };
			float    borderRadius{ 0.0f };

			// 新增：按主题切换的背景/边框（如果启用，覆盖静态配色）
			bool     useThemeBg{ false };
			QColor   bgLight{ Qt::transparent };
			QColor   bgDark{ Qt::transparent };

			bool     useThemeBorder{ false };
			QColor   borderLight{ Qt::transparent };
			QColor   borderDark{ Qt::transparent };

			// Interactive states (hover/press) — background
			// Option A: static colors for hover/press
			bool     useInteractiveBg{ false };
			QColor   bgHover{ Qt::transparent };
			QColor   bgPressed{ Qt::transparent };
			// Option B: theme-aware colors for hover/press
			bool     useThemeInteractiveBg{ false };
			QColor   bgHoverLight{ Qt::transparent };
			QColor   bgHoverDark{ Qt::transparent };
			QColor   bgPressedLight{ Qt::transparent };
			QColor   bgPressedDark{ Qt::transparent };

			// Interactive states (hover/press) — border (optional)
			bool     useInteractiveBorder{ false };
			QColor   borderHover{ Qt::transparent };
			QColor   borderPressed{ Qt::transparent };
			bool     useThemeInteractiveBorder{ false };
			QColor   borderHoverLight{ Qt::transparent };
			QColor   borderHoverDark{ Qt::transparent };
			QColor   borderPressedLight{ Qt::transparent };
			QColor   borderPressedDark{ Qt::transparent };

			// Auto interactive background for clickable boxes (default on)
			// When onTap is set and no explicit interactive bg provided,
			// DecoratedBox will apply subtle hover/press backgrounds by theme.
			bool     enableAutoInteractive{ true };

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
		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
		void append(Render::FrameData& fd) const override;

		bool onMousePress(const QPoint& pos) override;
		bool onMouseMove(const QPoint& pos) override;
		bool onMouseRelease(const QPoint& pos) override;
		bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;

		bool tick() override;
		QRect bounds() const override;

		// 主题变化：现在会影响自身（按主题色选择配色），并继续传递给子项
		void onThemeChanged(bool isDark) override;

	private:
		static QColor withOpacity(QColor c, float mul);

		// Base colors by theme (no interactive consideration)
		QColor effectiveBg() const;
		QColor effectiveBorder() const;

		// State-aware effective colors (hover/press)
		QColor effectiveBgForState() const;
		QColor effectiveBorderForState() const;

		// Defaults for auto interactive backgrounds
		QColor defaultHoverBg() const;
		QColor defaultPressedBg() const;

	private:
		std::unique_ptr<IUiComponent> m_child;
		Props   m_p;
		QRect   m_viewport;
		QRect   m_drawRect;
		QRect   m_contentRect;

		bool    m_hover{ false };
		bool    m_pressed{ false };    // 新增：保存按下状态
		bool    m_isDark{ false };     // 新增：保存当前主题
		IconCache* m_cache{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
	};

} // namespace UI