#pragma once
#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <algorithm>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qelapsedtimer.h>
#include <qmargins.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qtypes.h>
#include <unordered_map>
#include <vector>

// 前向声明
class TabViewModel;

// 通用 TabView 组件：支持 ViewModel 数据驱动
class UiTabView final : public IUiComponent, public IUiContent
{
public:
	struct Palette {
		QColor barBg;            // 选项卡条背景
		QColor contentBg;        // 内容区背景
		QColor tabHover;         // 悬停背景
		QColor tabSelectedBg;    // 选中背景
		QColor indicator;        // 选中指示条颜色
		QColor label;            // 标签文字颜色
		QColor labelSelected;    // 选中标签文字颜色
	};

	enum class IndicatorStyle {
		Bottom,  // 底部指示条
		Top,     // 顶部指示条
		Full     // 整个背景高亮
	};

	UiTabView() = default;
	~UiTabView() override = default;

	// 接入 ViewModel（推荐方式）
	void setViewModel(TabViewModel* vm);
	[[nodiscard]] TabViewModel* viewModel() const noexcept { return m_vm; }

	// 兼容旧接口（未接 VM 时使用）
	void setTabs(const QStringList& labels);
	void setSelectedIndex(int idx);
	[[nodiscard]] int selectedIndex() const noexcept;

	// IUiContent
	void setViewportRect(const QRect& r) override { m_viewport = r; }

	// 外观配置
	void setPalette(const Palette& p) { m_pal = p; }
	void setIndicatorStyle(IndicatorStyle style) { m_indicatorStyle = style; }
	void setTabHeight(int h) { m_tabHeight = std::max(24, h); }
	void setAnimationDuration(int ms) { m_animDuration = std::max(50, ms); }

	// 新增：布局与间距配置
	void setMargins(const QMargins& m) { m_margin = m; }
	void setPadding(const QMargins& p) { m_padding = p; }
	void setTabBarMargin(const QMargins& m) { m_tabBarMargin = m; }
	void setTabBarPadding(const QMargins& p) { m_tabBarPadding = p; }
	void setContentMargin(const QMargins& m) { m_contentMargin = m; }
	void setContentPadding(const QMargins& p) { m_contentPadding = p; }
	void setTabBarSpacing(qreal s) { m_tabBarSpacing = std::max<qreal>(0, s); }
	void setSpacing(qreal s) { m_spacing = std::max<qreal>(0, s); }

	// Tab内容管理接口
	void setContent(int tabIdx, IUiComponent* content);
	void setContents(const std::vector<IUiComponent*>& contents);
	IUiComponent* content(int tabIdx) const;

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool tick() override;
	QRect bounds() const override { return m_viewport; }

	// 新增：主题自动下发
	void onThemeChanged(bool isDark) override;

	// 供外部查询动画状态
	[[nodiscard]] bool hasActiveAnimation() const { return m_animHighlight.active; }

private:
	QRectF tabBarRectF() const;
	QRectF tabRectF(int i) const;
	QRectF contentRectF() const;

	int tabCount() const;
	QString tabLabel(int i) const;

	void syncFromVmInstant();
	void startHighlightAnim(float toCenterX);

	// 新增：确保当前选中内容拥有 viewport 与资源上下文
	void ensureCurrentContentSynced() const;

	static QString textCacheKey(const QString& baseKey, int px, const QColor& color);
	static float easeInOut(float t);

private:
	QRect m_viewport;
	TabViewModel* m_vm{ nullptr };

	QMargins m_margin{ 0,0,0,0 };
	QMargins m_padding{ 0,0,0,0 };
	QMargins m_tabBarMargin{ 0,0,0,0 };
	QMargins m_tabBarPadding{ 8,6,8,6 };
	QMargins m_contentMargin{ 0,0,0,0 };
	QMargins m_contentPadding{ 4,4,4,4 };
	qreal m_tabBarSpacing{ 4 };
	qreal m_spacing{ 8 };

	// 兼容模式数据（未接 VM 时使用）
	QStringList m_fallbackTabs;
	int m_fallbackSelected{ 0 };

	// 交互状态
	int m_hover{ -1 };
	int m_pressed{ -1 };
	int m_viewSelected{ 0 }; // 视图层选中索引

	// 高亮中心 X（逻辑像素）
	float m_highlightCenterX{ -1.0f };

	// 动画
	struct ScalarAnim {
		bool active{ false };
		float start{ 0 }, end{ 0 };
		qint64 startMs{ 0 };
		int durationMs{ 0 };
	} m_animHighlight;
	QElapsedTimer m_clock;

	// 外观配置（默认浅色）
	Palette m_pal{
		.barBg = QColor(0,0,0,0),
		.tabHover = QColor(0,0,0,16),
		.tabSelectedBg = QColor(0,0,0,22),
		.indicator = QColor(0,122,255,220),
		.label = QColor(50,60,70,255),
		.labelSelected = QColor(20,32,48,255)
	};

	IndicatorStyle m_indicatorStyle{ IndicatorStyle::Bottom };
	int m_tabHeight{ 46 };
	int m_animDuration{ 150 };

	// 资源上下文
	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };
	std::unordered_map<int, IUiComponent*> m_tabContents;
};