#pragma once
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"

#include <algorithm>
#include <cmath>
#include <qbytearray.h>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qhash.h>
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
		QColor railBg;       // 导航栏背景
		QColor itemHover;    // 悬停背景
		QColor itemPressed;  // 按下背景
		QColor itemSelected; // 选中背景（高亮）
		QColor iconColor;    // 图标颜色
		QColor labelColor;   // 文字颜色
		QColor indicator;    // 指示条颜色
	};

	struct NavItem {
		QString id;          // 业务标识
		QString svgLight;    // 浅色主题 SVG
		QString svgDark;     // 深色主题 SVG
		QString label;       // 标签文本（展开时显示）
	};

	// 抽象为通用 UI 组件
	class NavRail final : public IUiComponent
	{
	public:
		void setItems(std::vector<NavItem> items);
#if defined(_MSC_VER)
#endif
		int  count() const noexcept { return m_vm ? vmCount() : static_cast<int>(m_items.size()); }

		// 接入轻量 ViewModel（用于承载业务真值）
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

		// 选中项（未接入 VM 时使用）
		void setSelectedIndex(int idx);
		int  selectedIndex() const noexcept { return m_selected; }

		// 展开/折叠（未接入 VM 时使用）
		void toggleExpanded();
		bool expanded() const noexcept { return m_expandT > 0.5f; }

		// IUiComponent 接口实现
		void updateLayout(const QSize& windowSize) override { m_rect = QRect(0, 0, currentWidth(), windowSize.height()); }
		void updateResourceContext(IconLoader& iconLoader, QOpenGLFunctions* gl, float devicePixelRatio) override {
			m_loader = &iconLoader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
		}
		void append(Render::FrameData& fd) const override;
		bool onMousePress(const QPoint& pos) override;
		bool onMouseMove(const QPoint& pos) override;
		bool onMouseRelease(const QPoint& pos) override; // 激活在内部处理动画/或驱动 VM
		bool tick() override;

		QRect bounds() const override { return m_rect; }

		// 动画是否活跃（供外部检测）
		bool hasActiveAnimation() const {
			return m_animIndicator.active || m_animExpand.active;
		}

		// 设置“展开/收起”按钮的 SVG 资源路径（可选，已有默认值）
		void setToggleSvgPaths(QString expand, QString collapse) {
			m_svgToggleExpand = std::move(expand);
			m_svgToggleCollapse = std::move(collapse);
		}

	private:
		// 返回索引 i 对应的 item 的矩形：
		// - "设置" 固定在底部
		// - 其余项目自顶部（在“展开/收起”按钮下方）向下排列
		QRectF itemRectF(int i) const;

		// 顶部“展开/收起”按钮区域：放在最顶部，左右留白 8px，尺寸 32x32
		QRectF toggleRectF() const;

		// 顶部列表的起始 Y（位于“展开/收起”按钮下方留边之后）
		qreal topItemsStartY() const;

		// 查找“设置”项索引；找不到时返回 -1
		int findSettingsIndex() const;

		QByteArray svgDataCached(const QString& path) const;
		QString iconCacheKey(const QString& baseKey, int px, bool dark) const;

		// 文本纹理缓存键加入颜色，避免主题切换复用旧颜色
		QString textCacheKey(const QString& baseKey, int px, const QColor& color) const;

		static float easeInOut(float t) { t = std::clamp(t, 0.0f, 1.0f); return t * t * (3.0f - 2.0f * t); }

		// 标量动画
		struct ScalarAnim {
			bool active{ false };
			float start{ 0 }, end{ 0 };
			qint64 startMs{ 0 };
			int durationMs{ 0 };
		};

		void startIndicatorAnim(float toY, int durationMs = 220);
		void startExpandAnim(float toT, int durationMs = 220);

		// 初次从 VM 同步视图状态（无动画跳到位）
		void syncFromVmInstant();
		// VM 存在时的项目数
		int vmCount() const noexcept;

	private:
		QRect m_rect;                      // 导航栏矩形（逻辑像素，宽度由 currentWidth() 决定）

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
		int  m_selected{ -1 }; // 仅用于“未接 VM”模式，以及视图层的高亮对齐

		// “展开/收起”按钮 hover/press 状态
		bool m_toggleHovered{ false };
		bool m_togglePressed{ false };

		// 指示条中心 Y（逻辑像素）
		float m_indicatorY{ -1.0f };

		// 展开插值 0..1
		float m_expandT{ 0.0f };

		// 动画
		ScalarAnim m_animIndicator;
		ScalarAnim m_animExpand;
		QElapsedTimer m_clock; // 为本控件动画计时

		// 资源上下文（append 用）
		IconLoader* m_loader{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };

		// 轻量 ViewModel
		NavViewModel* m_vm{ nullptr };

		mutable QHash<QString, QByteArray> m_svgCache; // path -> bytes

		// toggle 的 SVG 路径（默认提供）
		QString m_svgToggleExpand{ ":/icons/nav_toggle_expand.svg" };     // 收起状态时显示“向右展开”
		QString m_svgToggleCollapse{ ":/icons/nav_toggle_collapse.svg" }; // 展开状态时显示“向左收起”
	};

} // namespace Ui