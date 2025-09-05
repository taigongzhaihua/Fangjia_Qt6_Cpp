/*
 * Popup.h - 主要的弹出组件实现
 * 
 * 设计原则：
 * - 管理触发器和弹出内容
 * - 直接使用PopupOverlay，无复杂包装
 * - 立即创建资源，避免延迟初始化
 * - 简单的位置计算和管理
 */

#pragma once

#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "PopupOverlay.h"
#include <memory>
#include <functional>
#include <qsize.h>
#include <qpoint.h>
#include <qcolor.h>
#include <qwindow.h>

class Popup : public IUiComponent, public IUiContent
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
        Center       // 屏幕中央
    };

public:
    /// 构造函数 - 立即创建所有必要组件
    explicit Popup(QWindow* parentWindow);
    ~Popup() override = default;


    
    /// 设置弹出内容
    void setContent(std::unique_ptr<IUiComponent> content);
    
    /// 配置弹出窗口
    void setPopupSize(const QSize& size) { m_popupSize = size; }
    void setPlacement(Placement placement) { m_placement = placement; }
    void setOffset(const QPoint& offset) { m_offset = offset; }
    void setBackgroundColor(const QColor& color);
    void setCornerRadius(float radius);
    void setShadowSize(float shadowSize);
    
    /// 程序控制显示/隐藏
    void showPopup();
    void hidePopup();
    bool isPopupVisible() const;
    
    /// 程序控制显示/隐藏 - 带位置参数
    void showPopupAt(const QPoint& position);
    void showPopupAtPosition(const QRect& triggerRect);
    
    /// 设置可见性变化回调
    void setOnVisibilityChanged(std::function<void(bool)> callback) {
        m_onVisibilityChanged = std::move(callback);
    }

    // IUiContent interface
    void setViewportRect(const QRect& rect) override;

    // IUiComponent interface
    void updateLayout(const QSize& windowSize) override;
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
    void append(Render::FrameData& frameData) const override;
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
    bool tick() override;
    QRect bounds() const override { return m_viewport; }
    void onThemeChanged(bool isDark) override;

private:
    /// 计算弹出窗口的全局位置
    QPoint calculatePopupPosition(const QRect& triggerRect) const;
    
    /// 处理弹出窗口隐藏
    void onPopupHidden();

private:
    // 父窗口
    QWindow* m_parentWindow;
    
    // 组件
    std::unique_ptr<PopupOverlay> m_overlay;
    
    // 配置
    QSize m_popupSize{200, 150};
    Placement m_placement{Placement::Bottom};
    QPoint m_offset{0, 0};
    
    // 状态
    QRect m_viewport;
    bool m_popupVisible{false};
    bool m_hasContent{false};  // Track if content has been set
    
    // 回调
    std::function<void(bool)> m_onVisibilityChanged;
};