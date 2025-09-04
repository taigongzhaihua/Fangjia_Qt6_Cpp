/*
 * Popup.cpp - 弹出组件实现
 */

#include "Popup.h"
#include <QApplication>
#include <QScreen>
#include <QDebug>
#include <QObject>

Popup::Popup(QWindow* parentWindow)
    : m_parentWindow(parentWindow)
{
    // 立即创建弹出覆盖窗口
    m_overlay = std::make_unique<PopupOverlay>(parentWindow);
    
    // 连接信号
    QObject::connect(m_overlay.get(), &PopupOverlay::popupHidden,
            [this]() { onPopupHidden(); });
    
    // 设置可见性回调
    m_overlay->setOnVisibilityChanged([this](bool visible) {
        m_popupVisible = visible;
        if (m_onVisibilityChanged) {
            m_onVisibilityChanged(visible);
        }
    });
}

void Popup::setTrigger(std::unique_ptr<IUiComponent> trigger)
{
    m_trigger = std::move(trigger);
}

void Popup::setContent(std::unique_ptr<IUiComponent> content)
{
    // 直接设置到覆盖窗口
    m_overlay->setContent(std::move(content));
}

void Popup::setBackgroundColor(const QColor& color)
{
    m_overlay->setBackgroundColor(color);
}

void Popup::setCornerRadius(float radius)
{
    m_overlay->setCornerRadius(radius);
}

void Popup::showPopup()
{
    if (m_popupVisible) {
        return;
    }
    
    // 计算弹出位置
    QPoint globalPos = calculatePopupPosition();
    
    // 显示弹出窗口
    m_overlay->showAt(globalPos, m_popupSize);
    m_popupVisible = true;
}

void Popup::hidePopup()
{
    if (!m_popupVisible) {
        return;
    }
    
    m_overlay->hidePopup();
    m_popupVisible = false;
}

bool Popup::isPopupVisible() const
{
    return m_popupVisible && m_overlay->isPopupVisible();
}

void Popup::setViewportRect(const QRect& rect)
{
    m_viewport = rect;
}

void Popup::updateLayout(const QSize& windowSize)
{
    // 更新触发器布局
    if (m_trigger) {
        m_trigger->updateLayout(windowSize);
    }
}

void Popup::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio)
{
    // 更新触发器资源上下文
    if (m_trigger) {
        m_trigger->updateResourceContext(cache, gl, devicePixelRatio);
    }
}

void Popup::append(Render::FrameData& frameData) const
{
    // 只渲染触发器，弹出内容由覆盖窗口自己渲染
    if (m_trigger) {
        m_trigger->append(frameData);
    }
}

bool Popup::onMousePress(const QPoint& pos)
{
    // 检查是否点击了触发器
    if (m_trigger && m_trigger->onMousePress(pos)) {
        // 切换弹出窗口显示状态
        if (m_popupVisible) {
            hidePopup();
        } else {
            showPopup();
        }
        return true;
    }
    
    return false;
}

bool Popup::onMouseMove(const QPoint& pos)
{
    if (m_trigger) {
        return m_trigger->onMouseMove(pos);
    }
    return false;
}

bool Popup::onMouseRelease(const QPoint& pos)
{
    if (m_trigger) {
        return m_trigger->onMouseRelease(pos);
    }
    return false;
}

bool Popup::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
    if (m_trigger) {
        return m_trigger->onWheel(pos, angleDelta);
    }
    return false;
}

bool Popup::tick()
{
    bool needsUpdate = false;
    
    // 更新触发器
    if (m_trigger) {
        needsUpdate |= m_trigger->tick();
    }
    
    return needsUpdate;
}

void Popup::onThemeChanged(bool isDark)
{
    // 更新触发器主题
    if (m_trigger) {
        m_trigger->onThemeChanged(isDark);
    }
}

QPoint Popup::calculatePopupPosition() const
{
    if (!m_trigger || !m_parentWindow) {
        return QPoint(100, 100); // 默认位置
    }
    
    // 获取触发器的边界
    QRect triggerBounds = m_trigger->bounds();
    
    // 转换到全局坐标
    QPoint parentGlobalPos = m_parentWindow->position();
    QPoint triggerGlobalPos = parentGlobalPos + triggerBounds.topLeft();
    
    // 根据位置策略计算弹出位置
    QPoint popupPos;
    
    switch (m_placement) {
    case Placement::Bottom:
        popupPos = QPoint(triggerGlobalPos.x(), 
                         triggerGlobalPos.y() + triggerBounds.height());
        break;
        
    case Placement::Top:
        popupPos = QPoint(triggerGlobalPos.x(), 
                         triggerGlobalPos.y() - m_popupSize.height());
        break;
        
    case Placement::Right:
        popupPos = QPoint(triggerGlobalPos.x() + triggerBounds.width(), 
                         triggerGlobalPos.y());
        break;
        
    case Placement::Left:
        popupPos = QPoint(triggerGlobalPos.x() - m_popupSize.width(), 
                         triggerGlobalPos.y());
        break;
        
    case Placement::BottomLeft:
        popupPos = QPoint(triggerGlobalPos.x() - m_popupSize.width(),
                         triggerGlobalPos.y() + triggerBounds.height());
        break;
        
    case Placement::BottomRight:
        popupPos = QPoint(triggerGlobalPos.x() + triggerBounds.width(),
                         triggerGlobalPos.y() + triggerBounds.height());
        break;
        
    case Placement::TopLeft:
        popupPos = QPoint(triggerGlobalPos.x() - m_popupSize.width(),
                         triggerGlobalPos.y() - m_popupSize.height());
        break;
        
    case Placement::TopRight:
        popupPos = QPoint(triggerGlobalPos.x() + triggerBounds.width(),
                         triggerGlobalPos.y() - m_popupSize.height());
        break;
        
    case Placement::Center:
        if (QScreen* screen = QApplication::primaryScreen()) {
            QRect screenGeometry = screen->geometry();
            popupPos = QPoint(
                screenGeometry.center().x() - m_popupSize.width() / 2,
                screenGeometry.center().y() - m_popupSize.height() / 2
            );
        } else {
            popupPos = QPoint(100, 100);
        }
        break;
        
    default:
        popupPos = triggerGlobalPos;
        break;
    }
    
    // 应用偏移量
    popupPos += m_offset;
    
    // TODO: 屏幕边界检查和调整
    
    return popupPos;
}

void Popup::onPopupHidden()
{
    m_popupVisible = false;
}