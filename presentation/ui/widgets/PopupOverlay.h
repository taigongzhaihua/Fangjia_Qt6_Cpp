/*
 * PopupOverlay.h - 简单直接的弹出窗口实现
 *
 * 设计原则：
 * - 使用 QOpenGLWidget 包装在透明 QWidget 中替代 QOpenGLWindow
 * - 立即初始化，无延迟创建
 * - 简单的事件处理和渲染
 * - 最小化API，专注核心功能
 */

#pragma once

#include "IconCache.h"
#include "RenderData.hpp"
#include "Renderer.h"
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include <functional>
#include <memory>
#include <qcolor.h>
#include <QOpenGLFunctions>
#include <QOpenGLWidget>
#include <qtimer.h>
#include <QWidget>

 // Forward declaration
class PopupOpenGLRenderer;

class PopupOverlay : public QWidget
{
	Q_OBJECT

public:
	/// 构造函数 - 立即创建所有必要资源
	explicit PopupOverlay(QWidget* parent = nullptr);
	~PopupOverlay() override;

	/// 设置弹出内容（立即设置，不延迟）
	void setContent(std::unique_ptr<IUiComponent> content);

	/// 设置背景样式
	void setBackgroundColor(const QColor& color) { m_backgroundColor = color; }
	void setCornerRadius(float radius) { m_cornerRadius = radius; }
	void setShadowSize(float shadowSize) { m_shadowSize = shadowSize; }

	/// 显示弹出窗口
	void showAt(const QPoint& globalPos, const QSize& size);

	/// 隐藏弹出窗口
	void hidePopup();

	/// 检查是否可见
	bool isPopupVisible() const { return isVisible(); }

	/// 设置可见性变化回调
	void setOnVisibilityChanged(std::function<void(bool)> callback) {
		m_onVisibilityChanged = std::move(callback);
	}

	/// 转发主题变化到内容
	void forwardThemeChange(bool isDark);

	/// 设置初始主题（用于首次显示时确保正确主题）
	void setTheme(bool isDark) { m_isDarkTheme = isDark; }

signals:
	/// 弹出窗口被隐藏
	void popupHidden();

protected:
	// Event handling
	void mousePressEvent(QMouseEvent* event) override;
	void mouseMoveEvent(QMouseEvent* event) override;
	void mouseReleaseEvent(QMouseEvent* event) override;
	void keyPressEvent(QKeyEvent* event) override;
	void focusOutEvent(QFocusEvent* event) override;
	void hideEvent(QHideEvent* event) override;

private slots:
	void onRenderTick();

private:
	// Internal OpenGL widget for rendering
	class PopupOpenGLRenderer : public QOpenGLWidget, public QOpenGLFunctions
	{
	public:
		explicit PopupOpenGLRenderer(PopupOverlay* parent);

	protected:
		void initializeGL() override;
		void resizeGL(int w, int h) override;
		void paintGL() override;

	private:
		PopupOverlay* m_popup;
	};

	void updateContentLayout();
	void renderBackground();
	void renderContent();
	bool eventFilter(QObject* obj, QEvent* event) override; // For global mouse events

private:
	// Content
	std::unique_ptr<IUiComponent> m_content;

	// OpenGL renderer widget
	PopupOpenGLRenderer* m_openglRenderer;

	// Visual style
	QColor m_backgroundColor{ 255, 255, 255, 255 };
	float m_cornerRadius{ 6.0f };
	float m_shadowSize{ 16.0f }; // Shadow size for calculating window margins

	// Rendering
	IconCache m_iconCache;
	Renderer m_renderer;
	QRect m_contentRect;
	QRect m_actualContentRect; // The actual content area within the expanded window
	bool m_initialized{ false };
	bool m_needsContentLayoutUpdate{ false }; // Track if content layout needs update

	// Animation timer
	QTimer m_renderTimer;

	// Callbacks
	std::function<void(bool)> m_onVisibilityChanged;

	// Global mouse event filter for click-outside detection
	bool m_installEventFilter{ false };

	// Theme state tracking
	bool m_isDarkTheme{ false };

	// Internal OpenGL rendering methods for use by PopupOpenGLRenderer
	void initializeGL();
	void resizeGL(int w, int h);
	void paintGL();
};