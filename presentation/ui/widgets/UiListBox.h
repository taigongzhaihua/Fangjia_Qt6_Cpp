#pragma once
#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "ILayoutable.hpp"

#include <algorithm>
#include <functional>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qrect.h>
#include <qstring.h>
#include <vector>

class UiListBox : public IUiComponent, public IUiContent, public ILayoutable
{
public:
	// 函数式模型接口：与UiTreeList保持一致，支持ViewModel绑定
	struct ModelFns {
		std::function<QVector<QString>()> items;
		std::function<int()> selectedIndex;
		std::function<void(int)> setSelectedIndex;
		std::function<void(int)> onActivated; // 双击或回车激活
	};

	struct Palette {
		QColor bg;              // 背景色
		QColor itemHover;       // 悬停背景
		QColor itemPressed;     // 按下背景
		QColor itemSelected;    // 选中背景
		QColor textPrimary;     // 主文字颜色
		QColor textSecondary;   // 次级文字颜色
		QColor separator;       // 分隔线颜色
		QColor indicator;       // 选中指示条颜色
	};

	UiListBox();
	~UiListBox() override = default;

	// 简单接口：直接设置字符串列表
	void setItems(const std::vector<QString>& items);
	void setSelectedIndex(int index);
	int selectedIndex() const noexcept { return m_selectedIndex; }
	void setOnActivated(std::function<void(int)> callback) { m_onActivated = std::move(callback); }

	// 函数式模型接口（与UiTreeList保持一致）
	void setModelFns(const ModelFns& fns) { m_modelFns = fns; reloadData(); }

	// 外观配置
	void setPalette(const Palette& p) { m_pal = p; }
	void setItemHeight(const int h) { m_itemHeight = std::max(24, h); }

	// 滚动支持（用于UiScrollView）
	void setScrollOffset(const int y) { m_scrollY = y; }
	[[nodiscard]] int scrollOffset() const noexcept { return m_scrollY; }
	[[nodiscard]] int contentHeight() const;

	// 供上层在模型数据变化后手动刷新
	void reloadData();

	// IUiContent
	void setViewportRect(const QRect& r) override { m_viewport = r; reloadData(); }

	// ILayoutable
	QSize measure(const SizeConstraints& cs) override;
	void arrange(const QRect& finalRect) override;
	QRect bounds() const override { return m_viewport; }

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
	bool tick() override;
	void onThemeChanged(bool isDark) override;

private:
	struct VisibleItem {
		int index{ -1 };
		QRect rect;
	};

	void updateVisibleItems();
	int hitTestItem(const QPoint& pos) const;
	void updateDefaultPalette(bool isDark);

private:
	// 数据模型
	std::vector<QString> m_items;
	int m_selectedIndex{ -1 };
	std::function<void(int)> m_onActivated;
	ModelFns m_modelFns;

	// 外观配置
	Palette m_pal;
	int m_itemHeight{ 36 };
	int m_scrollY{ 0 };

	// 视觉状态
	std::vector<VisibleItem> m_visibleItems;
	int m_hoverIndex{ -1 };
	int m_pressedIndex{ -1 };

	// 视口和渲染上下文
	QRect m_viewport;
	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	QElapsedTimer m_animClock;
};