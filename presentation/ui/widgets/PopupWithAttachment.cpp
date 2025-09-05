/*
 * PopupWithAttachment.cpp - 带依附对象支持的弹出组件包装器实现
 */

#include "PopupWithAttachment.h"
#include <QDebug>

PopupWithAttachment::PopupWithAttachment(QWindow* parentWindow)
{
    // 创建基础弹出组件
    m_popup = std::make_unique<Popup>(parentWindow);
}

void PopupWithAttachment::setContent(std::unique_ptr<IUiComponent> content)
{
    m_popup->setContent(std::move(content));
}

void PopupWithAttachment::setAttachmentObject(IUiComponent* attachmentObject)
{
    m_attachmentObject = attachmentObject;
}

void PopupWithAttachment::setPopupSize(const QSize& size)
{
    m_popup->setPopupSize(size);
}

void PopupWithAttachment::setPlacement(Popup::Placement placement)
{
    m_popup->setPlacement(placement);
}

void PopupWithAttachment::setOffset(const QPoint& offset)
{
    m_popup->setOffset(offset);
}

void PopupWithAttachment::setBackgroundColor(const QColor& color)
{
    m_popup->setBackgroundColor(color);
}

void PopupWithAttachment::setCornerRadius(float radius)
{
    m_popup->setCornerRadius(radius);
}

void PopupWithAttachment::showPopup()
{
    if (!m_attachmentObject) {
        qWarning() << "PopupWithAttachment::showPopup() called without attachment object";
        m_popup->showPopup();
        return;
    }
    
    // 根据依附对象计算位置并显示
    QRect attachmentRect = calculateAttachmentRect();
    m_popup->showPopupAtPosition(attachmentRect);
}

void PopupWithAttachment::hidePopup()
{
    m_popup->hidePopup();
}

bool PopupWithAttachment::isOpen() const
{
    return m_popup->isOpen();
}

bool PopupWithAttachment::isPopupVisible() const
{
    return m_popup->isPopupVisible();
}

void PopupWithAttachment::showPopupAt(const QPoint& position)
{
    m_popup->showPopupAt(position);
}

void PopupWithAttachment::showPopupAtPosition(const QRect& triggerRect)
{
    m_popup->showPopupAtPosition(triggerRect);
}

void PopupWithAttachment::setOnVisibilityChanged(std::function<void(bool)> callback)
{
    m_popup->setOnVisibilityChanged(std::move(callback));
}

QRect PopupWithAttachment::calculateAttachmentRect() const
{
    if (!m_attachmentObject) {
        return QRect(100, 100, 50, 30); // 默认矩形
    }
    
    // 使用依附对象的边界作为参考矩形
    return m_attachmentObject->bounds();
}