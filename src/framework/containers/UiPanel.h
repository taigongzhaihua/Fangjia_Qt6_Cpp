#pragma once
#include "ILayoutable.hpp"  // 新增
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <algorithm>
#include <cstdint>
#include <IconCache.h>
#include <qcolor.h>
#include <qmargins.h>
#include <qnamespace.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>
#include <vector>

// 顺序容器：按“子项实际尺寸”依次排布
class UiPanel : public IUiComponent, public IUiContent, public ILayoutable { // 新增 ILayoutable
public:
	enum class Orientation :uint8_t { Horizontal, Vertical };
	enum class CrossAlign :uint8_t { Start, Center, End, Stretch };

	struct Child {
		IUiComponent* component{ nullptr };
		CrossAlign crossAlign{ CrossAlign::Stretch };
		bool visible{ true };
	};

	explicit UiPanel(Orientation o = Orientation::Vertical);
	~UiPanel() override = default;

	// 管理
	void addChild(IUiComponent* c, CrossAlign a = CrossAlign::Stretch);
	void clearChildren();

	// 外观与布局
	void setViewportRect(const QRect& r) override { m_viewport = r; }
	void setOrientation(Orientation o) { m_orient = o; }
	void setMargins(const QMargins& m) { m_margins = m; }
	void setPadding(const QMargins& p) { m_padding = p; }
	void setSpacing(int px) { m_spacing = std::max(0, px); }
	void setBackground(QColor c, float radius = 0.0f) { m_bg = c; m_radius = std::max(0.0f, radius); }

	// ILayoutable（新增）
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

	QRect bounds() const override { return m_viewport; }
	void onThemeChanged(bool isDark) override;

private:
	QRect contentRect() const;
	QSize measureChild(IUiComponent* c, int crossAvail) const;
	QRect placeChild(const QRect& area, int cur, const QSize& desired, CrossAlign a) const;

private:
	Orientation m_orient{ Orientation::Vertical };
	std::vector<Child> m_children;
	std::vector<QRect> m_childRects;

	// 视口/上下文
	QRect m_viewport;
	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	// 外观
	QMargins m_margins{ 0,0,0,0 };
	QMargins m_padding{ 0,0,0,0 };
	int m_spacing{ 0 };
	QColor m_bg{ Qt::transparent };
	float  m_radius{ 0.0f };

	// 捕获
	IUiComponent* m_capture{ nullptr };
};