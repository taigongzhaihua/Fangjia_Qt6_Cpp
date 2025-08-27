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
#include <RenderData.hpp>

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

		bool tick() override;
		QRect bounds() const override;

		// 主题变化：现在会影响自身（按主题色选择配色），并继续传递给子项
		void onThemeChanged(bool isDark) override;

	private:
		static QColor withOpacity(QColor c, float mul);

		// 计算当前主题下的有效背景/边框颜色
		QColor effectiveBg() const;
		QColor effectiveBorder() const;

	private:
		std::unique_ptr<IUiComponent> m_child;
		Props   m_p;
		QRect   m_viewport;
		QRect   m_drawRect;
		QRect   m_contentRect;

		bool    m_hover{ false };
		bool    m_isDark{ false };     // 新增：保存当前主题
		IconCache* m_cache{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
	};

} // namespace UI