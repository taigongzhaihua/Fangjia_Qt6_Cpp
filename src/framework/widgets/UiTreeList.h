#pragma once
#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <algorithm>
#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qrect.h>
#include <qstring.h>
#include <vector>

class UiTreeList : public IUiComponent, public IUiContent
{
public:
	// 通用模型接口：由上层（如 UiFormulaView 内部的 VM 适配器）实现
	struct NodeInfo {
		QString label;
		int     level{ 0 };
		bool    expanded{ false };
	};

	class Model {
	public:
		virtual ~Model() = default;
		// 根层节点 id 列表
		virtual QVector<int> rootIndices() const = 0;
		// 子节点 id 列表
		virtual QVector<int> childIndices(int nodeId) const = 0;
		// 节点元信息
		virtual NodeInfo nodeInfo(int nodeId) const = 0;

		// 选中状态
		virtual int selectedId() const = 0;
		virtual void setSelectedId(int nodeId) = 0;

		// 展开/折叠
		virtual void setExpanded(int nodeId, bool on) = 0;
	};

	struct Palette {
		QColor bg;              // 背景色
		QColor itemHover;       // 悬停背景
		QColor itemSelected;    // 选中背景
		QColor expandIcon;      // 展开/折叠图标颜色
		QColor textPrimary;     // 主文字颜色
		QColor textSecondary;   // 次级文字颜色
		QColor separator;       // 分隔线颜色
		QColor indicator;       // 选中指示条颜色
	};

	UiTreeList();
	~UiTreeList() override = default;

	// 注入通用模型
	void setModel(Model* m) { m_model = m; reloadData(); }

	// 外观配置
	void setPalette(const Palette& p) { m_pal = p; }
	void setViewportRect(const QRect& r) override { m_viewport = r; reloadData(); }
	void setItemHeight(int h) { m_itemHeight = std::max(24, h); }
	void setIndentWidth(int w) { m_indentWidth = std::max(16, w); }

	// 滚动支持
	void setScrollOffset(int y) { m_scrollY = y; }
	[[nodiscard]] int scrollOffset() const noexcept { return m_scrollY; }
	[[nodiscard]] int contentHeight() const;

	// 供上层在模型数据变化后手动刷新
	void reloadData();

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
	bool tick() override; // 新增：实现动画推进（当前返回 false）
	QRect bounds() const override { return m_viewport; }

private:
	// 计算可见的节点列表（考虑展开状态）
	struct VisibleNode {
		int index;      // 模型层的节点 id
		int depth;      // 显示深度（用于缩进）
		QRect rect;     // 显示矩形
	};

	void updateVisibleNodes();
	[[nodiscard]] QRect nodeRect(int visibleIdx) const;
	[[nodiscard]] QRect expandIconRect(const QRect& nodeRect, int depth) const;

private:
	Model* m_model{ nullptr };
	QRect m_viewport;
	Palette m_pal{
		.bg = QColor(255,255,255,245),
		.itemHover = QColor(0,0,0,8),
		.itemSelected = QColor(0,122,255,20),
		.expandIcon = QColor(100,100,100,200),
		.textPrimary = QColor(32,38,46,255),
		.textSecondary = QColor(100,110,120,200),
		.separator = QColor(0,0,0,20),
		.indicator = QColor(0,122,255,200)
	};

	int m_itemHeight{ 36 };
	int m_indentWidth{ 20 };
	int m_scrollY{ 0 };

	std::vector<VisibleNode> m_visibleNodes;
	int m_hover{ -1 };
	int m_pressed{ -1 };

	// 资源上下文
	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	QElapsedTimer m_animClock;
};