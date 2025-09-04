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
    // Show at center by default when no position is specified
    showPopupAt(QPoint(100, 100));
}

void Popup::showPopupAt(const QPoint& position)
{
    if (m_popupVisible) {
        return;
    }
    
    // Show popup at specified position
    m_overlay->showAt(position, m_popupSize);
    m_popupVisible = true;
}

void Popup::showPopupAtPosition(const QRect& triggerRect)
{
    if (m_popupVisible) {
        return;
    }
    
    // Calculate popup position based on trigger rectangle
    QPoint popupPos = calculatePopupPosition(triggerRect);
    
    // Show popup at calculated position
    m_overlay->showAt(popupPos, m_popupSize);
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
    // Popup layout updated - no trigger to update
}

void Popup::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio)
{
    // Popup resources updated - no trigger to update
}

void Popup::append(Render::FrameData& frameData) const
{
    // Popup itself doesn't render anything - content is rendered by overlay window
}

bool Popup::onMousePress(const QPoint& pos)
{
    // Popup no longer handles trigger interactions - external controls should manage popup state
    return false;
}

bool Popup::onMouseMove(const QPoint& pos)
{
    // Popup no longer handles trigger interactions
    return false;
}

bool Popup::onMouseRelease(const QPoint& pos)
{
    // Popup no longer handles trigger interactions
    return false;
}

bool Popup::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
    // Popup no longer handles trigger interactions
    return false;
}

bool Popup::tick()
{
    // Popup no longer handles trigger updates
    return false;
}

void Popup::onThemeChanged(bool isDark)
{
    // Popup no longer handles trigger theme changes
}

QPoint Popup::calculatePopupPosition(const QRect& triggerRect) const
{
    if (!m_parentWindow) {
        return QPoint(100, 100); // 默认位置
    }
    
    // Convert trigger rect to global coordinates
    QPoint parentGlobalPos = m_parentWindow->position();
    QPoint triggerGlobalPos = parentGlobalPos + triggerRect.topLeft();
    
    // 根据位置策略计算弹出位置
    QPoint popupPos;
    
    switch (m_placement) {
    case Placement::Bottom:
        popupPos = QPoint(triggerGlobalPos.x(), 
                         triggerGlobalPos.y() + triggerRect.height());
        break;
        
    case Placement::Top:
        popupPos = QPoint(triggerGlobalPos.x(), 
                         triggerGlobalPos.y() - m_popupSize.height());
        break;
        
    case Placement::Right:
        popupPos = QPoint(triggerGlobalPos.x() + triggerRect.width(), 
                         triggerGlobalPos.y());
        break;
        
    case Placement::Left:
        popupPos = QPoint(triggerGlobalPos.x() - m_popupSize.width(), 
                         triggerGlobalPos.y());
        break;
        
    case Placement::BottomLeft:
        popupPos = QPoint(triggerGlobalPos.x() - m_popupSize.width(),
                         triggerGlobalPos.y() + triggerRect.height());
        break;
        
    case Placement::BottomRight:
        popupPos = QPoint(triggerGlobalPos.x() + triggerRect.width(),
                         triggerGlobalPos.y() + triggerRect.height());
        break;
        
    case Placement::TopLeft:
        popupPos = QPoint(triggerGlobalPos.x() - m_popupSize.width(),
                         triggerGlobalPos.y() - m_popupSize.height());
        break;
        
    case Placement::TopRight:
        popupPos = QPoint(triggerGlobalPos.x() + triggerRect.width(),
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