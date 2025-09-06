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
    m_hasContent = (content != nullptr);
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

void Popup::setShadowSize(float shadowSize)
{
    m_overlay->setShadowSize(shadowSize);
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
    // Forward resource context to overlay window if it's available
    // This ensures the popup content has access to the same resources
    if (m_overlay && m_overlay->isVisible()) {
        // The overlay manages its own resource context, but we can sync if needed
        // For now, this is handled by the overlay's updateContentLayout method
    }
}

void Popup::append(Render::FrameData& frameData) const
{
    // Popup acts as a placeholder in parent window - render a minimal placeholder
    // The actual content is rendered in the PopupOverlay window
    // This provides a reference position for popup placement
    if (!m_viewport.isEmpty()) {
        // Draw a minimal placeholder border when popup is visible (for debugging/reference)
        if (m_popupVisible) {
            Render::RoundedRectCmd placeholderCmd;
            placeholderCmd.rect = QRectF(m_viewport);
            placeholderCmd.radiusPx = 2.0f;
            placeholderCmd.color = QColor(128, 128, 128, 64); // Light gray, semi-transparent
            placeholderCmd.clipRect = QRectF(); // No clipping needed
            
            frameData.roundedRects.push_back(placeholderCmd);
        }
    }
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
    // Forward tick calls to overlay content if popup is visible
    // This ensures animations in the popup content continue to run
    if (m_overlay && m_overlay->isVisible() && m_hasContent) {
        // The overlay handles content ticking in its own render loop
        // We return false since this component itself has no animations
        return false;
    }
    return false;
}

void Popup::onThemeChanged(bool isDark)
{
    // Forward theme changes to overlay content
    if (m_overlay && m_hasContent) {
        m_overlay->forwardThemeChange(isDark);
    }
    applyTheme(isDark);
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