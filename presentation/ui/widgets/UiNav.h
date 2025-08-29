/*
 * 文件名：UiNav.h
 * 职责：侧边导航栏组件，提供垂直布局的导航项显示、选中状态管理和过渡动画。
 * 依赖：UI组件接口、渲染系统、NavViewModel。
 * 线程：仅在UI线程使用。
 * 备注：支持双ViewModel模式（内置数据 vs 外部ViewModel），实现Fluent Design风格的导航体验。
 */

#pragma once
#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "nav_interface.h"

#include <algorithm>
#include <cmath>
#include <qbytearray.h>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qrect.h>

#include <qsize.h>
#include <qstring.h>
#include <utility>
#include <vector>

 // Forward declaration
namespace fj::presentation::binding {
    class INavDataProvider;
}

namespace Ui {

	/// 导航栏色彩方案配置
	struct NavPalette {
		QColor railBg;         // 导航栏背景色
		QColor itemHover;      // 悬停状态色
		QColor itemPressed;    // 按下状态色
		QColor itemSelected;   // 选中状态色
		QColor iconColor;      // 图标颜色
		QColor labelColor;     // 标签文字色
		QColor indicator;      // 选中指示器色
	};

	/// 导航项数据结构 (UI层内部使用，与binding::NavItem兼容)
	struct UiNavItem {
		QString id;           // 导航项唯一标识
		QString svgLight;     // 亮色主题图标路径
		QString svgDark;      // 暗色主题图标路径
		QString label;        // 显示标签文本
	};

	/// 导航栏组件：垂直布局的侧边导航栏，支持图标+文字的导航项
	/// 
	/// 功能特性：
	/// - 双数据源模式（内置NavItem数组 or 外部NavViewModel）
	/// - 流畅的选中状态动画过渡
	/// - 悬停/按下/选中的交互反馈
	/// - 主题适配的图标和色彩切换
	/// - 选中指示器的位置动画
	/// 
	/// 交互行为：
	/// - 鼠标悬停显示高亮背景
	/// - 点击切换选中状态并触发回调
	/// - 选中指示器平滑移动到新位置
	class NavRail final : public IUiComponent
	{
	public:
		/// 功能：设置导航项数据（内置模式）
		/// 参数：items — 导航项列表
		/// 说明：切换到内置数据模式，清除外部DataProvider绑定
		void setItems(std::vector<UiNavItem> items);

		/// 功能：获取导航项总数
		/// 返回：当前导航项数量
		/// 说明：自动选择数据源（DataProvider优先，否则使用内置数据）
		int  count() const noexcept { return m_dataProvider ? dataProviderCount() : static_cast<int>(m_items.size()); }

		/// 功能：绑定外部DataProvider（外部模式）
		/// 参数：provider — 导航数据提供者指针
		/// 说明：切换到外部数据模式，导航项数据由DataProvider提供
		void setDataProvider(fj::presentation::binding::INavDataProvider* provider);

		/// 功能：设置暗色主题状态
		/// 参数：dark — 是否启用暗色主题
		void setDarkTheme(const bool dark) { m_isDark = dark; }

		/// 功能：设置导航栏色彩方案
		/// 参数：p — 包含各状态颜色的色彩方案
		void setPalette(const NavPalette& p) { m_pal = p; }

		/// 功能：设置图标逻辑尺寸
		/// 参数：s — 图标逻辑像素尺寸（最小20）
		void setIconLogicalSize(const int s) { m_iconLogical = s > 0 ? s : 20; }

		/// 功能：设置导航项高度
		/// 参数：h — 单个导航项的高度（最小44）
		void setItemHeight(const int h) { m_itemH = h > 24 ? h : 44; }

		/// 功能：设置导航栏宽度（折叠/展开状态）
		/// 参数：collapsedW — 折叠状态宽度（最小40）
		/// 参数：expandedW — 展开状态宽度（最小为折叠宽度+40）
		/// 说明：自动调整展开插值参数范围
		void setWidths(const int collapsedW, const int expandedW) {
			m_collapsedW = std::max(40, collapsedW);
			m_expandedW = std::max(m_collapsedW + 40, expandedW);
			m_expandT = std::clamp(m_expandT, 0.0f, 1.0f);
		}

		/// 功能：计算当前实际宽度
		/// 返回：基于展开插值的当前宽度
		/// 说明：在折叠宽度和展开宽度之间线性插值
		int  currentWidth() const {
			return static_cast<int>(std::lround(static_cast<float>(m_collapsedW) + static_cast<float>(m_expandedW - m_collapsedW) * m_expandT));
		}

		void setLabelFontPx(const int px) { m_labelFontPx = std::max(10, px); }

		void setSelectedIndex(int idx);
		int  selectedIndex() const noexcept { return m_selected; }

		void toggleExpanded();
		bool expanded() const noexcept { return m_expandT > 0.5f; }

		void updateLayout(const QSize& windowSize) override { m_rect = QRect(0, 0, currentWidth(), windowSize.height()); }
		void updateResourceContext(IconCache& iconCache, QOpenGLFunctions* gl, const float devicePixelRatio) override {
			m_cache = &iconCache; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
		}
		void append(Render::FrameData& fd) const override;
		bool onMousePress(const QPoint& pos) override;
		bool onMouseMove(const QPoint& pos) override;
		bool onMouseRelease(const QPoint& pos) override;
		bool tick() override;

		void onThemeChanged(const bool isDark) override {
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

		void syncFromProviderInstant();
		int dataProviderCount() const noexcept;

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

		std::vector<UiNavItem> m_items;
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

		fj::presentation::binding::INavDataProvider* m_dataProvider{ nullptr };

		QString m_svgToggleExpand{ ":/icons/nav_toggle_expand.svg" };
		QString m_svgToggleCollapse{ ":/icons/nav_toggle_collapse.svg" };
	};

} // namespace Ui