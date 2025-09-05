/*
 * PopupWithAttachment.h - 带依附对象支持的弹出组件包装器
 * 
 * 设计原则：
 * - 包装基础 Popup 类，添加依附对象功能
 * - 自动根据依附对象计算弹出位置
 * - 维护依附对象与弹出窗口的关联关系
 * - 不继承UI接口，纯粹的控制类
 */

#pragma once

#include "UiComponent.hpp"
#include "Popup.h"
#include <memory>
#include <functional>

class PopupWithAttachment
{
public:
    /// 构造函数 - 创建带依附对象支持的弹出组件
    explicit PopupWithAttachment(QWindow* parentWindow);
    ~PopupWithAttachment() = default;

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

private:
    /// 根据依附对象计算弹出位置
    QRect calculateAttachmentRect() const;

private:
    // 核心弹出组件
    std::unique_ptr<Popup> m_popup;
    
    // 依附对象引用
    IUiComponent* m_attachmentObject{nullptr};
};