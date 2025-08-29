#pragma once
#include "IconCache.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <algorithm>
#include <qcolor.h>
#include <qmargins.h>
#include <qnamespace.h>
#include <qopenglfunctions.h>
#include <qrect.h>
#include <vector>

class UiBoxLayout : public IUiComponent, public IUiContent
{
public:
	enum class Direction { Horizontal, Vertical };

	// 新增：主轴尺寸模式
	enum class SizeMode {
		Weighted, // 旧行为：按权重分配剩余空间
		Natural   // 新行为：按首选尺寸顺序排布，剩余留白，超出裁切
	};

	enum class Alignment { Start, Center, End, Stretch };

	enum class MainAlignment {
		Start, Center, End, SpaceBetween, SpaceAround, SpaceEvenly
	};

	struct ChildItem {
		IUiComponent* component{ nullptr };
		float weight{ 0.0f };
		Alignment alignment{ Alignment::Start };
		bool visible{ true };
	};

	explicit UiBoxLayout(Direction dir = Direction::Vertical);
	~UiBoxLayout() override = default;

	// 子控件管理
	void addChild(IUiComponent* component, float weight = 0.0f, Alignment align = Alignment::Start);
	void insertChild(size_t index, IUiComponent* component, float weight = 0.0f, Alignment align = Alignment::Start);
	void removeChild(IUiComponent* component);
	void removeChildAt(size_t index);
	void clearChildren();

	[[nodiscard]] size_t childCount() const noexcept { return m_children.size(); }
	[[nodiscard]] IUiComponent* childAt(size_t index) const;

	// 布局属性
	void setDirection(Direction dir);
	[[nodiscard]] Direction direction() const noexcept { return m_direction; }

	void setSpacing(const int spacing) { m_spacing = std::max(0, spacing); }
	[[nodiscard]] int spacing() const noexcept { return m_spacing; }

	void setMargins(const QMargins& margins) { m_margins = margins; }
	[[nodiscard]] const QMargins& margins() const noexcept { return m_margins; }

	void setMainAlignment(const MainAlignment a) { m_mainAlign = a; }
	[[nodiscard]] MainAlignment mainAlignment() const noexcept { return m_mainAlign; }

	// 新增：主轴尺寸模式
	void setSizeMode(const SizeMode m) { m_sizeMode = m; }
	[[nodiscard]] SizeMode sizeMode() const noexcept { return m_sizeMode; }
	UiBoxLayout& withSizeMode(const SizeMode m) { setSizeMode(m); return *this; }

	// 背景
	void setBackgroundColor(const QColor& color) { m_bgColor = color; }
	void setCornerRadius(const float radius) { m_cornerRadius = std::max(0.0f, radius); }

	// IUiContent
	void setViewportRect(const QRect& r) override;

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;
	bool tick() override;
	void setChildWeight(size_t index, float weight);
	float childWeight(size_t index) const;
	void setChildAlignment(size_t index, Alignment align);
	Alignment childAlignment(size_t index) const;
	UiBoxLayout& withSpacing(int spacing);
	UiBoxLayout& withMargins(const QMargins& margins);
	UiBoxLayout& withBackground(const QColor& color, float radius);
	UiBoxLayout& withMainAlignment(const MainAlignment a) { setMainAlignment(a); return *this; }

	QRect bounds() const override { return m_viewport; }
	void onThemeChanged(bool isDark) override;

	void applyTheme(const bool dark) override { m_isDark = dark; IUiComponent::applyTheme(dark); }
	bool isDarkTheme() const { return m_isDark; }

	void setChildVisible(size_t index, bool visible);
	bool isChildVisible(size_t index) const;

protected:
	void calculateLayout();
	QRect childRect(size_t index) const;
	QRect contentRect() const;

private:
	Direction m_direction{ Direction::Vertical };
	SizeMode  m_sizeMode{ SizeMode::Weighted }; // 新增：默认保持兼容
	QRect m_viewport;
	QMargins m_margins{ 0, 0, 0, 0 };
	int m_spacing{ 0 };
	MainAlignment m_mainAlign{ MainAlignment::Start };

	bool m_isDark{ false };

	QColor m_bgColor{ Qt::transparent };
	float m_cornerRadius{ 0.0f };

	std::vector<ChildItem> m_children;
	std::vector<QRect> m_childRects;

	IUiComponent* m_capturedChild{ nullptr };
};

using UiVBoxLayout = UiBoxLayout;

class UiHBoxLayout : public UiBoxLayout {
public:
	UiHBoxLayout() : UiBoxLayout(Direction::Horizontal) {}
};