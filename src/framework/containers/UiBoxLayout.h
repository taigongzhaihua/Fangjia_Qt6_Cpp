#pragma once
#include "IconLoader.h"
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

// 通用盒式布局容器：支持水平/垂直排列子控件和主题传播
class UiBoxLayout : public IUiComponent, public IUiContent
{
public:
	enum class Direction {
		Horizontal,  // 水平排列
		Vertical     // 垂直排列
	};

	enum class Alignment {
		Start,       // 左对齐（水平）或顶对齐（垂直）
		Center,      // 居中对齐
		End,         // 右对齐（水平）或底对齐（垂直）
		Stretch      // 拉伸填充
	};

	struct ChildItem {
		IUiComponent* component{ nullptr };
		float weight{ 0.0f };      // 权重（0表示固定大小，>0表示按比例分配剩余空间）
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

	void setSpacing(int spacing) { m_spacing = std::max(0, spacing); }
	[[nodiscard]] int spacing() const noexcept { return m_spacing; }

	void setMargins(const QMargins& margins) { m_margins = margins; }
	[[nodiscard]] const QMargins& margins() const noexcept { return m_margins; }

	// 背景
	void setBackgroundColor(const QColor& color) { m_bgColor = color; }
	void setCornerRadius(float radius) { m_cornerRadius = std::max(0.0f, radius); }

	// IUiContent
	void setViewportRect(const QRect& r) override;

	// IUiComponent
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
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
	QRect bounds() const override { return m_viewport; }
	// IUiComponent - 添加主题支持
	void onThemeChanged(bool isDark) override;

	// 设置是否为深色主题
	void applyTheme(bool dark) override { m_isDark = dark; IUiComponent::applyTheme(dark); }
	bool isDarkTheme() const { return m_isDark; }
	// 子控件可见性
	void setChildVisible(size_t index, bool visible);
	bool isChildVisible(size_t index) const;

protected:
	// 计算布局
	void calculateLayout();
	QRect childRect(size_t index) const;

	// 获取内容区域（去除边距后）
	QRect contentRect() const;

private:
	Direction m_direction{ Direction::Vertical };
	QRect m_viewport;
	QMargins m_margins{ 0, 0, 0, 0 };
	int m_spacing{ 0 };

	bool m_isDark{ false };  // 添加主题状态

	QColor m_bgColor{ Qt::transparent };
	float m_cornerRadius{ 0.0f };

	std::vector<ChildItem> m_children;
	std::vector<QRect> m_childRects;  // 缓存计算后的子控件矩形

	// 捕获状态（用于事件分发）
	IUiComponent* m_capturedChild{ nullptr };
};

// 便捷类型别名
using UiVBoxLayout = UiBoxLayout;  // 垂直布局

class UiHBoxLayout : public UiBoxLayout {
public:
	UiHBoxLayout() : UiBoxLayout(Direction::Horizontal) {}
};