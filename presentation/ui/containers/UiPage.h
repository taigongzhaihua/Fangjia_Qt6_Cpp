/*
 * 文件名：UiPage.h
 * 职责：页面容器组件，提供标题区域、内容区域和滚轮事件转发机制。
 * 依赖：UI组件接口、渲染系统、图标缓存。
 * 线程：仅在UI线程使用。
 * 备注：支持主题色彩方案，实现内容区域的滚轮事件转发，管理单一内容组件。
 */

#pragma once
#include "IconCache.h"
#include "IFocusContainer.hpp"
#include "ILayoutable.hpp"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <qcolor.h>
#include <qmargins.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <qstringliteral.h>
#include <utility>

 /// 页面容器：提供标题显示和内容区域管理的标准页面布局
 /// 
 /// 功能：
 /// - 页面标题显示与样式配置
 /// - 内容组件的生命周期管理和事件转发
 /// - 主题色彩方案应用（背景、标题色、正文色）
 /// - 滚轮事件在内容区域的精确转发
 /// 
 /// 布局结构：
 /// ┌─────────────────────┐
 /// │ 标题区域            │
 /// ├─────────────────────┤
 /// │                     │ ← 内容区域（转发事件到内容组件）
 /// │ 内容组件            │
 /// │                     │
 /// └─────────────────────┘
class UiPage : public IUiComponent, public IUiContent, public ILayoutable, public IFocusContainer
{
public:
	/// 页面色彩方案配置
	struct Palette {
		QColor cardBg;         // 卡片背景色
		QColor headingColor;   // 标题文字色
		QColor bodyColor;      // 正文文字色
	};

	UiPage() = default;
	~UiPage() override = default;

	/// 功能：设置页面标题
	/// 参数：title — 页面标题文本
	void setTitle(QString title) { m_title = std::move(title); }

	/// 功能：获取页面标题
	/// 返回：当前页面标题
	QString title() const { return m_title; }

	/// 功能：设置页面色彩方案
	/// 参数：p — 包含各元素颜色的色彩方案
	void setPalette(const Palette& p) { m_pal = p; }

	/// 功能：获取当前色彩方案
	/// 返回：当前页面的色彩方案
	const Palette& palette() const { return m_pal; }

	/// 功能：设置页面视口矩形
	/// 参数：r — 视口矩形（逻辑像素坐标）
	void setViewportRect(const QRect& r) override { m_viewport = r; }

	/// 功能：设置页面内容组件
	/// 参数：content — 内容组件指针（不转移所有权）
	/// 说明：页面负责将事件转发给内容组件
	void setContent(IUiComponent* content) { m_content = content; }

	/// 功能：获取当前内容组件
	/// 返回：内容组件指针
	IUiComponent* content() const { return m_content; }

	// ILayoutable interface implementation
	QSize measure(const SizeConstraints& cs) override;
	void arrange(const QRect& finalRect) override;

	// IUiComponent接口实现
	void updateLayout(const QSize& windowSize) override;
	void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
	void append(Render::FrameData& fd) const override;
	bool onMousePress(const QPoint& pos) override;
	bool onMouseMove(const QPoint& pos) override;
	bool onMouseRelease(const QPoint& pos) override;

	/// 功能：处理滚轮事件并转发到内容区域
	/// 参数：pos — 鼠标位置（逻辑像素坐标）
	/// 参数：angleDelta — 滚轮角度增量
	/// 返回：是否在内容区域内并被处理
	/// 说明：仅在内容区域内的滚轮事件会被转发给内容组件
	bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;

	bool tick() override;
	QRect bounds() const override { return m_viewport; }

	/// 功能：主题变化处理
	/// 参数：isDark — 是否为暗色主题
	/// 说明：更新页面色彩方案并传播给内容组件
	void onThemeChanged(bool isDark) override;

	// IFocusContainer
	void enumerateFocusables(std::vector<IFocusable*>& out) const override;

	/// 功能：设置暗色主题状态
	/// 参数：dark — 是否启用暗色主题
	void setDarkTheme(const bool dark) { m_isDark = dark; }
	bool isDarkTheme() const { return m_isDark; }

	QRectF cardRectF() const;
	QRectF contentRectF() const;

	void setMargins(const QMargins& m) { m_margins = m; }
	QMargins margins() const { return m_margins; }

	void setPadding(const QMargins& p) { m_padding = p; }
	QMargins padding() const { return m_padding; }

	void setCornerRadius(const float r) { m_cornerRadius = r; }
	float cornerRadius() const { return m_cornerRadius; }

	// 页面生命周期钩子
	virtual void onAppear() {}
	virtual void onDisappear() {}

protected:
	virtual void initializeContent() {}
	virtual void applyPageTheme(bool isDark);

protected:
	QMargins m_margins{ 8, 0, 8, 8 };
	QMargins m_padding{ 16,0,16,8 };
	float m_cornerRadius = 8.0f;

	QRect m_viewport;

	QString m_title{ QStringLiteral("页面") };
	Palette m_pal{
		.cardBg = QColor(255,255,255,240),
		.headingColor = QColor(32, 38, 46, 255),
		.bodyColor = QColor(60, 70, 84, 220)
	};

	IUiComponent* m_content{ nullptr };

	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };
	bool m_isDark{ false };

	static constexpr int kTitleAreaH = 84;
};