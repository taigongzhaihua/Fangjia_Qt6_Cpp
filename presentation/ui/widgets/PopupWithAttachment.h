/*
 * PopupWithAttachment.h - 带依附对象支持的弹出组件包装器
 * 
 * 设计原则：
 * - 包装基础 Popup 类，添加依附对象功能
 * - 自动根据依附对象计算弹出位置
 * - 维护依附对象与弹出窗口的关联关系
 * - 继承UI组件接口以融入UI系统，但不在父窗口渲染内容
 */

#pragma once

#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "Popup.h"
#include <memory>
#include <functional>

class PopupWithAttachment : public IUiComponent, public IUiContent
{
public:
    /// 构造函数 - 创建带依附对象支持的弹出组件
    explicit PopupWithAttachment(QWindow* parentWindow);
    ~PopupWithAttachment() override = default;

    /// 设置弹出内容
    void setContent(std::unique_ptr<IUiComponent> content);
    
    /// 设置依附对象 - 作为弹出位置参考
    void setAttachmentObject(IUiComponent* attachmentObject);
    
    /// 配置弹出窗口
    void setPopupSize(const QSize& size);
    void setPlacement(Popup::Placement placement);
    void setOffset(const QPoint& offset);
    void setBackgroundColor(const QColor& color);
    void setCornerRadius(float radius);
    
    /// 程序控制显示/隐藏
    void showPopup();        // 显示在依附对象位置
    void hidePopup();
    bool isOpen() const;
    bool isPopupVisible() const;
    
    /// 程序控制显示/隐藏 - 带位置参数
    void showPopupAt(const QPoint& position);
    void showPopupAtPosition(const QRect& triggerRect);
    
    /// 设置可见性变化回调
    void setOnVisibilityChanged(std::function<void(bool)> callback);

    // IUiContent interface - 不在父窗口渲染内容
    void setViewportRect(const QRect& rect) override;

    // IUiComponent interface - 不在父窗口渲染内容
    void updateLayout(const QSize& windowSize) override;
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
    void append(Render::FrameData& frameData) const override;
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
    bool tick() override;
    QRect bounds() const override;
    void onThemeChanged(bool isDark) override;

private:
    /// 根据依附对象计算弹出位置
    QRect calculateAttachmentRect() const;

private:
    // 核心弹出组件
    std::unique_ptr<Popup> m_popup;
    
    // 依附对象引用
    IUiComponent* m_attachmentObject{nullptr};
    
    // 视口状态（用于UI接口兼容）
    QRect m_viewport;
};