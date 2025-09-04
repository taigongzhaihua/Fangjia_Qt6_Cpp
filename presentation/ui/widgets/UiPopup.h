/*
 * 文件名：UiPopup.h
 * 职责：弹出控件组件，集成UiPopupWindow到现有UI框架中。
 * 依赖：UiPopupWindow、UI组件接口。
 * 线程：仅在UI线程使用。
 * 备注：作为UI组件的适配器，管理弹出窗口的生命周期和显示逻辑。
 */

#pragma once

#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "UiPopupWindow.h"
#include <functional>
#include <IconCache.h>
#include <memory>
#include <qcolor.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qwindow.h>
#include <RenderData.hpp>

 /// 弹出控件：管理可超出窗口边界的弹出内容
 /// 
 /// 功能：
 /// - 集成UiPopupWindow到现有UI组件系统
 /// - 提供触发器和内容管理
 /// - 支持自定义显示位置和大小
 /// - 处理弹出窗口的生命周期
 /// 
 /// 使用模式：
 /// 1. 设置触发器内容（可选）
 /// 2. 设置弹出内容
 /// 3. 通过交互或程序控制显示/隐藏
class UiPopup : public IUiComponent, public IUiContent
{
public:
	/// 弹出位置策略
	enum class Placement {
		Bottom,      // 在触发器下方
		Top,         // 在触发器上方
		Right,       // 在触发器右侧
		Left,        // 在触发器左侧
		BottomLeft,  // 在触发器左下方
		BottomRight, // 在触发器右下方
		TopLeft,     // 在触发器左上方
		TopRight,    // 在触发器右上方
		Custom       // 自定义位置
	};

public:
	explicit UiPopup(QWindow* parentWindow);
	~UiPopup() override;

	/// 设置触发器内容（通常是按钮或其他可点击元素）
	/// 参数：trigger — 触发器组件指针（不转移所有权）
	void setTrigger(IUiComponent* trigger);

	/// 设置弹出内容
	/// 参数：content — 弹出内容组件指针（不转移所有权）
	void setPopupContent(IUiComponent* content);

	/// 设置弹出窗口大小
	void setPopupSize(const QSize& size) { m_popupSize = size; }

	/// 设置弹出位置策略
	void setPlacement(Placement placement) { m_placement = placement; }

	/// 设置自定义偏移量
	void setOffset(const QPoint& offset) { m_offset = offset; }

	/// 设置背景颜色和圆角
	void setPopupStyle(const QColor& backgroundColor, float cornerRadius = 8.0f);

	/// 设置点击外部时是否关闭弹出窗口
	void setCloseOnClickOutside(bool close) { m_closeOnClickOutside = close; }

	/// 程序控制显示弹出窗口
	void showPopup();

	/// 程序控制隐藏弹出窗口
	void hidePopup();

	/// 检查弹出窗口是否可见
	bool isPopupVisible() const;

	/// 设置弹出窗口显示/隐藏回调
	void setOnPopupVisibilityChanged(std::function<void(bool)> callback) {
		m_onVisibilityChanged = callback;
	}

	// IUiContent
	void setViewportRect(const QRect& r) override;

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

private:
	/// 计算弹出窗口的全局位置
	QPoint calculatePopupPosition() const;

	/// 处理弹出窗口隐藏事件
	void onPopupHidden();

	/// 更新弹出窗口主题
	void updatePopupTheme();

private:
	// 窗口管理
	QWindow* m_parentWindow{ nullptr };                    // 父窗口引用
	std::unique_ptr<UiPopupWindow> m_popupWindow;          // 弹出窗口实例

	// 内容管理
	IUiComponent* m_trigger{ nullptr };                    // 触发器组件（非拥有）
	IUiComponent* m_popupContent{ nullptr };               // 弹出内容组件（非拥有）

	// 视口管理
	QRect m_viewport;                                       // 当前视口

	// 弹出配置
	QSize m_popupSize{ 200, 150 };                         // 弹出窗口大小
	Placement m_placement{ Placement::Bottom };             // 弹出位置策略
	QPoint m_offset{ 0, 0 };                               // 位置偏移量
	bool m_closeOnClickOutside{ true };                    // 点击外部关闭

	// 视觉样式
	QColor m_backgroundColor{ 255, 255, 255, 240 };       // 背景颜色
	float m_cornerRadius{ 8.0f };                          // 圆角半径
	bool m_isDark{ false };                                // 当前主题

	// 资源上下文
	IconCache* m_cache{ nullptr };
	QOpenGLFunctions* m_gl{ nullptr };
	float m_dpr{ 1.0f };

	// 回调函数
	std::function<void(bool)> m_onVisibilityChanged;       // 可见性变化回调
};