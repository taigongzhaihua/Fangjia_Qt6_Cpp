#pragma once
#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"

#include <algorithm>
#include <cmath>
#include <qbytearray.h>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qrect.h>

#include <qpoint.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>
#include <utility>
#include <vector>

// 前向声明：接入轻量 ViewModel
class NavViewModel;

namespace Ui {

	struct NavPalette {
		QColor railBg;
		QColor itemHover;
		QColor itemPressed;
		QColor itemSelected;
		QColor iconColor;
		QColor labelColor;
		QColor indicator;
	};

	struct NavItem {
		QString id;
		QString svgLight;
		QString svgDark;
		QString label;
	};

	class NavRail final : public IUiComponent
	{
	public:
		void setItems(std::vector<NavItem> items);
#if defined(_MSC_VER)
#endif
		int  count() const noexcept { return m_vm ? vmCount() : static_cast<int>(m_items.size()); }

		void setViewModel(NavViewModel* vm);

		void setDarkTheme(bool dark) { m_isDark = dark; }
		void setPalette(const NavPalette& p) { m_pal = p; }

		void setIconLogicalSize(int s) { m_iconLogical = s > 0 ? s : 20; }
		void setItemHeight(int h) { m_itemH = h > 24 ? h : 44; }

		void setWidths(int collapsedW, int expandedW) {
			m_collapsedW = std::max(40, collapsedW);
			m_expandedW = std::max(m_collapsedW + 40, expandedW);
			m_expandT = std::clamp(m_expandT, 0.0f, 1.0f);
		}
		int  currentWidth() const {
			return static_cast<int>(std::lround(static_cast<float>(m_collapsedW) + static_cast<float>(m_expandedW - m_collapsedW) * m_expandT));
		}

		void setLabelFontPx(int px) { m_labelFontPx = std::max(10, px); }

		void setSelectedIndex(int idx);
		int  selectedIndex() const noexcept { return m_selected; }

		void toggleExpanded();
		bool expanded() const noexcept { return m_expandT > 0.5f; }

		void updateLayout(const QSize& windowSize) override { m_rect = QRect(0, 0, currentWidth(), windowSize.height()); }
		void updateResourceContext(IconCache& iconCache, QOpenGLFunctions* gl, float devicePixelRatio) override {
			m_cache = &iconCache; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
		}
		void append(Render::FrameData& fd) const override;
		bool onMousePress(const QPoint& pos) override;
		bool onMouseMove(const QPoint& pos) override;
		bool onMouseRelease(const QPoint& pos) override;
		bool tick() override;

		void onThemeChanged(bool isDark) override {
			setDarkTheme(isDark);
			setPalette(isDark ? getDarkPalette() : getLightPalette());
		}

		QRect bounds() const override { return m_rect; }

		bool hasActiveAnimation() const {
			return m_animIndicator.active || m_animExpand.active;
		}

		void setToggleSvgPaths(QString expand, QString collapse) {
			m_svgToggleExpand = std::move(expand);
			m_svgToggleCollapse = std::move(collapse);
		}

	private:
		QRectF itemRectF(int i) const;
		QRectF toggleRectF() const;
		qreal topItemsStartY() const;
		int findSettingsIndex() const;

		QByteArray svgDataCached(const QString& path) const;
		QString iconCacheKey(const QString& baseKey, int px, bool dark) const;
		QString textCacheKey(const QString& baseKey, int px, const QColor& color) const;

		static float easeInOut(float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }

		struct ScalarAnim {
			bool active{ false };
			float start{ 0 }, end{ 0 };
			qint64 startMs{ 0 };
			int durationMs{ 0 };
		};

		void startIndicatorAnim(float toY, int durationMs = 220);
		void startExpandAnim(float toT, int durationMs = 220);

		void syncFromVmInstant();
		int vmCount() const noexcept;

		static Ui::NavPalette getDarkPalette() {
			return Ui::NavPalette{
				.railBg = QColor(21, 28, 36, 0),
				.itemHover = QColor(255,255,255,18),
				.itemPressed = QColor(255,255,255,30),
				.itemSelected = QColor(255,255,255,36),
				.iconColor = QColor(242,245,255,198),
				.labelColor = QColor(255,255,255,255),
				.indicator = QColor(0,122,255,200)
			};
		}

		static Ui::NavPalette getLightPalette() {
			return Ui::NavPalette{
				.railBg = QColor(246,248,250,0),
				.itemHover = QColor(0,0,0,14),
				.itemPressed = QColor(0,0,0,26),
				.itemSelected = QColor(0,0,0,32),
				.iconColor = QColor(70,76,84,255),
				.labelColor = QColor(70,76,84,255),
				.indicator = QColor(0,102,204,220)
			};
		}

	private:
		QRect m_rect;

		std::vector<NavItem> m_items;
		NavPalette m_pal;
		bool m_isDark{ true };

		int  m_iconLogical{ 18 };
		int  m_itemH{ 48 };
		int  m_labelFontPx{ 13 };

		int  m_collapsedW{ 64 };
		int  m_expandedW{ 220 };

		int  m_hover{ -1 };
		int  m_pressed{ -1 };
		int  m_selected{ -1 };

		bool m_toggleHovered{ false };
		bool m_togglePressed{ false };

		float m_indicatorY{ -1.0f };
		float m_expandT{ 0.0f };

		ScalarAnim m_animIndicator;
		ScalarAnim m_animExpand;
		QElapsedTimer m_clock;

		IconCache* m_cache{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };

		NavViewModel* m_vm{ nullptr };

		QString m_svgToggleExpand{ ":/icons/nav_toggle_expand.svg" };
		QString m_svgToggleCollapse{ ":/icons/nav_toggle_collapse.svg" };
	};

} // namespace Ui