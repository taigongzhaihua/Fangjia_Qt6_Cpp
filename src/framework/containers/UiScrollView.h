#pragma once
#include "ILayoutable.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <RenderData.hpp>

class IconCache;
class QOpenGLFunctions;

/**
 * @brief 垂直滚动容器，支持鼠标拖拽和滚动条操作
 *
 * UiScrollView 是一个垂直滚动容器，用于承载超出可视区域的内容。
 *
 * 特性：
 * - 垂直滚动，仅管理一个子组件
 * - 使用"移动子组件 viewport 顶坐标"实现滚动，无需矩阵变换
 * - 支持鼠标左键拖拽内容进行滚动
 * - 右侧滚动条（track + thumb）渲染，支持 hover/press 态
 * - 拖拽滚动条 thumb；点击 track 快速定位
 * - 主题感知：浅色/深色两套默认调色
 * - 若子项实现 ILayoutable，则用 widthBounded 方式测量以得到内容高度
 *
 * 滚动实现原理：
 * - arrange/setViewportRect 时：将子项 viewport 设为 {left, top - scrollY, width, contentHeight}
 * - 绘制时先 append 子项，再通过 RenderUtils::applyParentClip 裁剪到自身 viewport
 * - 在右侧绘制滚动条（仅当 contentHeight > viewport.height() 时显示）
 */
class UiScrollView final : public IUiComponent, public IUiContent, public ILayoutable {
public:
	UiScrollView();
	~UiScrollView() override = default;

	// 子组件管理
	void setChild(IUiComponent* child) { m_child = child; }
	IUiComponent* child() const noexcept { return m_child; }

	// 滚动控制
	void setScrollY(int scrollY);
	int scrollY() const noexcept { return m_scrollY; }
	int maxScrollY() const noexcept;
	int contentHeight() const noexcept { return m_contentHeight; }

	// IUiContent
	void setViewportRect(const QRect& r) override;

	// ILayoutable
	QSize measure(const SizeConstraints& cs) override;
	void arrange(const QRect& finalRect) override;

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
	void applyTheme(bool isDark) override;

private:
	// 布局计算
	void updateChildLayout() const;
	void measureContent();
	QRect getChildViewport() const;
	QRect getScrollbarRect() const;
	QRect getScrollbarThumbRect() const;

	// 滚动条交互
	bool isScrollbarVisible() const;
	bool isPointInScrollbar(const QPoint& pos) const;
	bool isPointInThumb(const QPoint& pos) const;
	void startThumbDrag(const QPoint& pos);
	void handleTrackClick(const QPoint& pos);

	// 内容拖拽
	void startContentDrag(const QPoint& pos);

	// 滚动条渲染
	void renderScrollbar(Render::FrameData& fd) const;

	// 滚动条动画控制
	void showScrollbar();  // 立即显示滚动条

	// 滚动限制
	void clampScrollY();

private:
	// 子组件
	IUiComponent* m_child{ nullptr }; // 非拥有

	// 视口与布局
	QRect m_viewport;
	int m_contentHeight{ 0 };
	int m_scrollY{ 0 };

	// 交互状态
	enum class DragMode { None, Content, Thumb };
	DragMode m_dragMode{ DragMode::None };
	QPoint m_dragStartPos;
	int m_dragStartScrollY{ 0 };
	int m_dragStartThumbY{ 0 };

	// 悬停状态
	bool m_thumbHovered{ false };
	bool m_thumbPressed{ false };

	// 滚动条配置 - 更符合 Fluent 设计
	static constexpr int SCROLLBAR_WIDTH = 6;   // 更细的滚动条
	static constexpr int THUMB_MIN_HEIGHT = 20;
	static constexpr int THUMB_RADIUS = 3;      // 圆角半径

	// 滚动条动画/显隐
	float m_thumbAlpha{ 0.0f };     // 0..1（乘以颜色 alpha 使用）
	bool  m_animActive{ false };
	qint64 m_lastInteractMs{ 0 };
	mutable QElapsedTimer m_animClock;  // 动画计时器

	// 动画参数
	static constexpr qint64 FADE_DELAY_MS = 900;   // 静默后开始淡出的延时
	static constexpr qint64 FADE_DURATION_MS = 300; // 淡出持续时间
	static constexpr float BASE_ALPHA = 0.3f;      // 基础半透明度（空闲时）

	// 主题颜色
	QColor m_trackColor;
	QColor m_thumbColor;
	QColor m_thumbHoverColor;
	QColor m_thumbPressColor;

	// 资源上下文
	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };
};