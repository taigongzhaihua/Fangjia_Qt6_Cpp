#pragma once
#include "ILayoutable.hpp"
#include "IFocusContainer.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <algorithm>
#include <cstdint>
#include <qmargins.h>
#include <qopenglfunctions.h>
#include <qrect.h>
#include <RenderData.hpp>
#include <vector>

// WPF 风格网格：行列定义 + 任意单元格定位 + 跨行列 + Star 占比
class UiGrid : public IUiComponent, public IUiContent, public ILayoutable, public IFocusContainer {
public:
	enum class Align : uint8_t { Start, Center, End, Stretch };

	struct TrackDef {
		enum class Type : uint8_t { Auto, Pixel, Star } type{ Type::Auto };
		float value{ 0.0f }; // Pixel->像素；Star->权重；Auto->忽略
		static TrackDef Auto() { return { Type::Auto, 0.0f }; }
		static TrackDef Px(const int px) { return { Type::Pixel, static_cast<float>(std::max(0, px)) }; }
		static TrackDef Star(const float w = 1.0f) { return { Type::Star, std::max(0.0f, w) }; }
	};

	struct Child {
		IUiComponent* component{ nullptr };
		int row{ 0 }, col{ 0 };
		int rowSpan{ 1 }, colSpan{ 1 };
		Align hAlign{ Align::Stretch }, vAlign{ Align::Stretch };
		bool visible{ true };
	};

	UiGrid() = default;
	~UiGrid() override = default;

	// IUiContent
	void setViewportRect(const QRect& r) override { m_viewport = r; }

	// 网格定义
	void setRowDefs(std::vector<TrackDef> rows) { m_rows = std::move(rows); }
	void setColDefs(std::vector<TrackDef> cols) { m_cols = std::move(cols); }
	void setRowSpacing(const int px) { m_rowSpacing = std::max(0, px); }
	void setColSpacing(const int px) { m_colSpacing = std::max(0, px); }

	void setMargins(const QMargins& m) { m_margins = m; }
	void setPadding(const QMargins& p) { m_padding = p; }

	// 子项管理
	void clearChildren();
	void addChild(IUiComponent* c, int row, int col, int rowSpan = 1, int colSpan = 1,
		Align hAlign = Align::Stretch, Align vAlign = Align::Stretch);

	// ILayoutable
	QSize measure(const SizeConstraints& cs) override;
	void arrange(const QRect& finalRect) override { setViewportRect(finalRect); }

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;

	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
	bool tick() override;

	QRect bounds() const override { return m_viewport; }
	void onThemeChanged(bool isDark) override;

	// IFocusContainer
	void enumerateFocusables(std::vector<IFocusable*>& out) const override;

private:
	QRect contentRect() const;

	// 尺寸求解
	std::vector<int> computeColumnWidths(int contentW) const;
	std::vector<int> computeRowHeights(int contentH, const std::vector<int>& colW) const;
	std::vector<int> computeRowHeightsIntrinsic(const std::vector<int>& colW) const;

	// 子项测量工具
	QSize measureChildNatural(IUiComponent* c) const;
	QSize measureChildWidthBound(IUiComponent* c, int maxW) const;

	// 放置到单元格（考虑对齐）
	QRect placeInCell(const QRect& cell, const QSize& desired, Align h, Align v) const;

	// 确保行列定义长度足够（不足用 Auto 填充）
	void ensureTrackSize(int minRows, int minCols) const;

private:
	mutable std::vector<TrackDef> m_rows;
	mutable std::vector<TrackDef> m_cols;

	std::vector<Child> m_children;
	std::vector<QRect> m_childRects;

	// 外形
	QRect m_viewport;
	QMargins m_margins{ 0,0,0,0 };
	QMargins m_padding{ 0,0,0,0 };
	int m_rowSpacing{ 8 };
	int m_colSpacing{ 8 };

	// 上下文
	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	// 捕获
	IUiComponent* m_capture{ nullptr };
};