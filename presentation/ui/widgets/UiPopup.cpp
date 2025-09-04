#include "UiPopup.h"
#include "../base/RenderUtils.hpp"
#include <QDebug>
#include <QGuiApplication>
#include <algorithm>

UiPopup::UiPopup(QWindow* parentWindow)
    : m_parentWindow(parentWindow)
{
    // 创建弹出窗口实例
    m_popupWindow = std::make_unique<UiPopupWindow>(parentWindow);
    
    // 连接弹出窗口隐藏信号
    QObject::connect(m_popupWindow.get(), &UiPopupWindow::popupHidden,
                     [this]() { onPopupHidden(); });
}

UiPopup::~UiPopup() = default;

void UiPopup::setTrigger(IUiComponent* trigger)
{
    m_trigger = trigger;
}

void UiPopup::setPopupContent(IUiComponent* content)
{
    m_popupContent = content;
    if (m_popupWindow) {
        m_popupWindow->setContent(content);
        updatePopupTheme();
    }
}

void UiPopup::setPopupStyle(const QColor& backgroundColor, float cornerRadius)
{
    m_backgroundColor = backgroundColor;
    m_cornerRadius = cornerRadius;
    
    if (m_popupWindow) {
        m_popupWindow->setBackgroundColor(backgroundColor);
        m_popupWindow->setCornerRadius(cornerRadius);
    }
}

void UiPopup::showPopup()
{
    if (!m_popupWindow || !m_popupContent) return;
    
    // 计算弹出位置
    const QPoint globalPos = calculatePopupPosition();
    
    // 显示弹出窗口
    m_popupWindow->showAt(globalPos, m_popupSize);
    
    // 触发回调
    if (m_onVisibilityChanged) {
        m_onVisibilityChanged(true);
    }
}

void UiPopup::hidePopup()
{
    if (m_popupWindow) {
        m_popupWindow->hidePopup();
    }
}

bool UiPopup::isPopupVisible() const
{
    return m_popupWindow && m_popupWindow->isPopupVisible();
}

void UiPopup::setViewportRect(const QRect& r)
{
    m_viewport = r;
    
    // 如果有触发器，设置其视口
    if (m_trigger) {
        if (auto* content = dynamic_cast<IUiContent*>(m_trigger)) {
            content->setViewportRect(r);
        }
    }
}

void UiPopup::updateLayout(const QSize& windowSize)
{
    // 更新触发器布局
    if (m_trigger) {
        m_trigger->updateLayout(windowSize);
    }
}

void UiPopup::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio)
{
    m_cache = &cache;
    m_gl = gl;
    m_dpr = devicePixelRatio;
    
    // 更新触发器资源上下文
    if (m_trigger) {
        m_trigger->updateResourceContext(cache, gl, devicePixelRatio);
    }
}

void UiPopup::append(Render::FrameData& fd) const
{
    // 只渲染触发器内容，弹出内容由弹出窗口自己渲染
    if (m_trigger) {
        const int rr0 = static_cast<int>(fd.roundedRects.size());
        const int im0 = static_cast<int>(fd.images.size());
        
        m_trigger->append(fd);
        
        // 应用视口裁剪
        RenderUtils::applyParentClip(fd, rr0, im0, QRectF(m_viewport));
    }
}

bool UiPopup::onMousePress(const QPoint& pos)
{
    if (!m_viewport.contains(pos)) return false;
    
    // 先让触发器处理事件
    bool handled = false;
    if (m_trigger) {
        handled = m_trigger->onMousePress(pos);
    }
    
    // 如果触发器处理了事件且当前弹出窗口隐藏，则显示弹出窗口
    if (handled && !isPopupVisible()) {
        showPopup();
    }
    // 如果触发器处理了事件且当前弹出窗口可见，则隐藏弹出窗口
    else if (handled && isPopupVisible()) {
        hidePopup();
    }
    
    return handled;
}

bool UiPopup::onMouseMove(const QPoint& pos)
{
    if (!m_viewport.contains(pos)) return false;
    
    // 转发给触发器
    if (m_trigger) {
        return m_trigger->onMouseMove(pos);
    }
    
    return false;
}

bool UiPopup::onMouseRelease(const QPoint& pos)
{
    if (!m_viewport.contains(pos)) return false;
    
    // 转发给触发器
    if (m_trigger) {
        return m_trigger->onMouseRelease(pos);
    }
    
    return false;
}

bool UiPopup::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
    if (!m_viewport.contains(pos)) return false;
    
    // 转发给触发器
    if (m_trigger) {
        return m_trigger->onWheel(pos, angleDelta);
    }
    
    return false;
}

bool UiPopup::tick()
{
    bool hasAnimation = false;
    
    // 触发器动画
    if (m_trigger) {
        hasAnimation = m_trigger->tick() || hasAnimation;
    }
    
    return hasAnimation;
}

void UiPopup::onThemeChanged(bool isDark)
{
    m_isDark = isDark;
    
    // 更新触发器主题
    if (m_trigger) {
        m_trigger->onThemeChanged(isDark);
    }
    
    // 更新弹出窗口主题
    updatePopupTheme();
}

QPoint UiPopup::calculatePopupPosition() const
{
    if (!m_parentWindow || !m_trigger) {
        return QPoint(0, 0);
    }
    
    // 获取触发器的全局位置
    const QRect triggerBounds = m_trigger->bounds();
    const QPoint parentGlobalPos = m_parentWindow->position();
    const QPoint triggerGlobalPos = parentGlobalPos + triggerBounds.topLeft();
    
    QPoint popupPos;
    
    switch (m_placement) {
    case Placement::Bottom:
        popupPos = QPoint(triggerGlobalPos.x(), triggerGlobalPos.y() + triggerBounds.height());
        break;
        
    case Placement::Top:
        popupPos = QPoint(triggerGlobalPos.x(), triggerGlobalPos.y() - m_popupSize.height());
        break;
        
    case Placement::Right:
        popupPos = QPoint(triggerGlobalPos.x() + triggerBounds.width(), triggerGlobalPos.y());
        break;
        
    case Placement::Left:
        popupPos = QPoint(triggerGlobalPos.x() - m_popupSize.width(), triggerGlobalPos.y());
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
        
    case Placement::Custom:
    default:
        popupPos = triggerGlobalPos;
        break;
    }
    
    // 应用偏移量
    popupPos += m_offset;
    
    return popupPos;
}

void UiPopup::onPopupHidden()
{
    // 触发回调
    if (m_onVisibilityChanged) {
        m_onVisibilityChanged(false);
    }
}

void UiPopup::updatePopupTheme()
{
    if (!m_popupWindow) return;
    
    // 根据当前主题更新弹出窗口样式
    QColor bgColor;
    if (m_isDark) {
        bgColor = QColor(45, 45, 48, 240);  // 半透明深色
    } else {
        bgColor = QColor(255, 255, 255, 240);  // 半透明浅色
    }
    
    m_popupWindow->setBackgroundColor(bgColor);
    m_popupWindow->applyTheme(m_isDark);
}