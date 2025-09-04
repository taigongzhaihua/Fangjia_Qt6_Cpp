/*
 * PopupOverlay.h - 简单直接的弹出窗口实现
 * 
 * 设计原则：
 * - 直接继承QOpenGLWindow，避免复杂包装
 * - 立即初始化，无延迟创建
 * - 简单的事件处理和渲染
 * - 最小化API，专注核心功能
 */

#pragma once

#include <qopenglwindow.h>
#include <qopenglfunctions.h>
#include <qtimer.h>
#include <qcolor.h>
#include <memory>
#include <functional>
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "IconCache.h"
#include "RenderData.hpp"

class PopupOverlay : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    /// 构造函数 - 立即创建所有必要资源
    explicit PopupOverlay(QWindow* parent = nullptr);
    ~PopupOverlay() override = default;

    /// 设置弹出内容（立即设置，不延迟）
    void setContent(std::unique_ptr<IUiComponent> content);
    
    /// 设置背景样式
    void setBackgroundColor(const QColor& color) { m_backgroundColor = color; }
    void setCornerRadius(float radius) { m_cornerRadius = radius; }
    
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

signals:
    /// 弹出窗口被隐藏
    void popupHidden();

protected:
    // OpenGL lifecycle
    void initializeGL() override;
    void resizeGL(int w, int h) override;
    void paintGL() override;
    
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
    void updateContentLayout();
    void renderBackground();
    void renderContent();

private:
    // Content
    std::unique_ptr<IUiComponent> m_content;
    
    // Visual style
    QColor m_backgroundColor{255, 255, 255, 240};
    float m_cornerRadius{8.0f};
    
    // Rendering
    IconCache m_iconCache;
    QRect m_contentRect;
    bool m_initialized{false};
    
    // Animation timer
    QTimer m_renderTimer;
    
    // Callbacks
    std::function<void(bool)> m_onVisibilityChanged;
};