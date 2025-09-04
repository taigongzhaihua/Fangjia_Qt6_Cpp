/*
 * 文件名：SimplePopup.cpp
 * 职责：简化的弹出控件实现
 */

#include "SimplePopup.h"
// #include "UiRoot.h"  // 暂时注释掉，测试时不需要
// #include "Renderer.h" // 暂时注释掉，使用简化渲染
#include <QApplication>
// #include <QDesktopWidget>  // Qt6中已弃用
#include <QScreen>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>

// SimplePopupWindow implementation
SimplePopupWindow::SimplePopupWindow(QWindow* parent)
    : QOpenGLWindow(NoPartialUpdate, parent)  // Qt6 constructor signature
{
    // 设置窗口属性
    setFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    
    // 设置动画定时器
    connect(&m_animTimer, &QTimer::timeout, this, &SimplePopupWindow::onAnimationTick);
    m_animTimer.setTimerType(Qt::PreciseTimer);
    m_animTimer.setInterval(16);  // ~60 FPS
    m_animClock.start();
}

SimplePopupWindow::~SimplePopupWindow()
{
    // 清理OpenGL资源
    if (context()) {
        makeCurrent();
        m_iconCache.releaseAll(this);
        doneCurrent();
    }
}

void SimplePopupWindow::setContent(std::unique_ptr<IUiComponent> content)
{
    m_content = std::move(content);
    
    // 如果OpenGL上下文已经准备好，立即更新资源
    if (context()) {
        updateContentResources();
        updateContentLayout();
    }
    
    m_needsRedraw = true;
    update();
}

void SimplePopupWindow::setBackgroundStyle(const QColor& color, float cornerRadius)
{
    m_backgroundColor = color;
    m_cornerRadius = cornerRadius;
    m_needsRedraw = true;
    update();
}

void SimplePopupWindow::showAt(const QPoint& globalPos, const QSize& size)
{
    // 设置窗口大小和位置
    resize(size);
    setPosition(globalPos);
    
    // 显示窗口
    show();
    
    // 启动动画
    m_animTimer.start();
    
    // 请求焦点
    requestActivate();
    
    // 通知可见性变化
    if (m_onVisibilityChanged) {
        m_onVisibilityChanged(true);
    }
}

void SimplePopupWindow::hidePopup()
{
    hide();
    m_animTimer.stop();
    
    // 通知可见性变化
    if (m_onVisibilityChanged) {
        m_onVisibilityChanged(false);
    }
}

bool SimplePopupWindow::isVisible() const
{
    return QOpenGLWindow::isVisible();
}

void SimplePopupWindow::initializeGL()
{
    qDebug() << "SimplePopupWindow::initializeGL start";
    
    initializeOpenGLFunctions();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 初始化内容资源
    updateContentResources();
    updateContentLayout();
    
    qDebug() << "SimplePopupWindow::initializeGL end";
}

void SimplePopupWindow::resizeGL(int w, int h)
{
    m_contentRect = QRect(0, 0, w, h);
    updateContentLayout();
    m_needsRedraw = true;
}

void SimplePopupWindow::paintGL()
{
    // 清除背景
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (!m_content) {
        return;
    }
    
    // 渲染内容
    renderContent();
    m_needsRedraw = false;
}

void SimplePopupWindow::mousePressEvent(QMouseEvent* event)
{
    if (m_content && event->button() == Qt::LeftButton) {
        if (m_content->onMousePress(event->pos())) {
            update();
            event->accept();
            return;
        }
    }
    QOpenGLWindow::mousePressEvent(event);
}

void SimplePopupWindow::mouseMoveEvent(QMouseEvent* event)
{
    if (m_content) {
        if (m_content->onMouseMove(event->pos())) {
            setCursor(Qt::PointingHandCursor);
            update();
        } else {
            setCursor(Qt::ArrowCursor);
        }
    }
    QOpenGLWindow::mouseMoveEvent(event);
}

void SimplePopupWindow::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_content && event->button() == Qt::LeftButton) {
        if (m_content->onMouseRelease(event->pos())) {
            update();
            event->accept();
            return;
        }
    }
    QOpenGLWindow::mouseReleaseEvent(event);
}

void SimplePopupWindow::keyPressEvent(QKeyEvent* event)
{
    if (event->key() == Qt::Key_Escape) {
        hidePopup();
        event->accept();
        return;
    }
    QOpenGLWindow::keyPressEvent(event);
}

void SimplePopupWindow::focusOutEvent(QFocusEvent* event)
{
    // 如果失去焦点，可以选择隐藏弹出窗口
    // hidePopup();
    QOpenGLWindow::focusOutEvent(event);
}

void SimplePopupWindow::onAnimationTick()
{
    if (m_content && m_content->tick()) {
        update();
    }
}

void SimplePopupWindow::updateContentLayout()
{
    if (m_content && !m_contentRect.isEmpty()) {
        m_content->updateLayout(m_contentRect.size());
        
        // 如果内容实现了IUiContent接口，设置视口
        if (auto* content = dynamic_cast<IUiContent*>(m_content.get())) {
            content->setViewportRect(m_contentRect);
        }
    }
}

void SimplePopupWindow::updateContentResources()
{
    if (m_content && context()) {
        m_content->updateResourceContext(m_iconCache, this, static_cast<float>(devicePixelRatio()));
    }
}

void SimplePopupWindow::renderContent()
{
    if (!m_content) {
        return;
    }
    
    // 创建渲染数据
    Render::FrameData frameData;
    
    // 绘制背景
    if (m_backgroundColor.alpha() > 0) {
        frameData.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(m_contentRect),
            .radiusPx = m_cornerRadius,
            .color = m_backgroundColor,
            .clipRect = QRectF(m_contentRect)
        });
    }
    
    // 添加内容渲染数据
    m_content->append(frameData);
    
    // 使用简化的渲染器渲染
    // 注意：这里需要一个简化的渲染实现，因为原有的Renderer可能太复杂
    renderFrameData(frameData);
}

void SimplePopupWindow::renderFrameData(const Render::FrameData& frameData)
{
    // 简化的渲染实现
    // 这里只实现基本的矩形渲染，足以验证弹出功能
    
    for (const auto& cmd : frameData.roundedRects) {
        // 设置颜色
        float r = cmd.color.redF();
        float g = cmd.color.greenF();
        float b = cmd.color.blueF();
        float a = cmd.color.alphaF();
        
        glColor4f(r, g, b, a);
        
        // 绘制矩形（简化实现，不支持圆角）
        const QRectF& rect = cmd.rect;
        glBegin(GL_QUADS);
        glVertex2f(rect.left(), rect.top());
        glVertex2f(rect.right(), rect.top());
        glVertex2f(rect.right(), rect.bottom());
        glVertex2f(rect.left(), rect.bottom());
        glEnd();
    }
}

// SimplePopup implementation
SimplePopup::SimplePopup(QWindow* parentWindow)
    : m_parentWindow(parentWindow)
{
    // 立即创建弹出窗口（而不是延迟创建）
    m_popupWindow = std::make_unique<SimplePopupWindow>(parentWindow);
    
    // 设置回调来处理弹出窗口隐藏
    m_popupWindow->setOnVisibilityChanged([this](bool visible) {
        if (!visible) {
            onPopupHidden();
        }
        if (m_onVisibilityChanged) {
            m_onVisibilityChanged(visible);
        }
    });
}

SimplePopup::~SimplePopup() = default;

void SimplePopup::setTrigger(std::unique_ptr<IUiComponent> trigger)
{
    m_trigger = std::move(trigger);
}

void SimplePopup::setPopupContent(std::unique_ptr<IUiComponent> content)
{
    m_popupContent = std::move(content);
    
    // 立即设置到弹出窗口（而不是延迟设置）
    if (m_popupWindow && m_popupContent) {
        // 创建内容的副本（因为我们需要保留原始引用）
        // 注意：这里可能需要根据实际的组件接口调整
        m_popupWindow->setContent(std::move(m_popupContent));
    }
}

void SimplePopup::setBackgroundStyle(const QColor& color, float cornerRadius)
{
    m_backgroundColor = color;
    m_cornerRadius = cornerRadius;
    
    if (m_popupWindow) {
        m_popupWindow->setBackgroundStyle(color, cornerRadius);
    }
}

void SimplePopup::showPopup()
{
    if (!m_popupWindow || !m_trigger) {
        qDebug() << "SimplePopup::showPopup: Missing popup window or trigger";
        return;
    }
    
    // 计算弹出位置
    QPoint globalPos = calculatePopupPosition();
    
    // 显示弹出窗口
    m_popupWindow->showAt(globalPos, m_popupSize);
    
    qDebug() << "SimplePopup: 弹出窗口已显示在位置" << globalPos << "大小" << m_popupSize;
}

void SimplePopup::hidePopup()
{
    if (m_popupWindow) {
        m_popupWindow->hidePopup();
    }
}

bool SimplePopup::isPopupVisible() const
{
    return m_popupWindow && m_popupWindow->isVisible();
}

void SimplePopup::setViewportRect(const QRect& r)
{
    m_viewport = r;
    
    // 立即更新触发器视口
    if (m_trigger) {
        if (auto* triggerContent = dynamic_cast<IUiContent*>(m_trigger.get())) {
            triggerContent->setViewportRect(r);
        }
    }
}

void SimplePopup::updateLayout(const QSize& windowSize)
{
    // 立即更新触发器布局
    if (m_trigger) {
        m_trigger->updateLayout(windowSize);
    }
}

void SimplePopup::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio)
{
    m_cache = &cache;
    m_gl = gl;
    m_dpr = devicePixelRatio;
    
    // 立即更新触发器资源上下文
    if (m_trigger) {
        m_trigger->updateResourceContext(cache, gl, devicePixelRatio);
    }
}

void SimplePopup::append(Render::FrameData& fd) const
{
    // 始终渲染触发器（无条件）
    if (m_trigger) {
        m_trigger->append(fd);
    }
}

bool SimplePopup::onMousePress(const QPoint& pos)
{
    // 直接转发给触发器，无复杂判断
    if (m_trigger) {
        return m_trigger->onMousePress(pos);
    }
    return false;
}

bool SimplePopup::onMouseMove(const QPoint& pos)
{
    // 直接转发给触发器
    if (m_trigger) {
        return m_trigger->onMouseMove(pos);
    }
    return false;
}

bool SimplePopup::onMouseRelease(const QPoint& pos)
{
    // 直接转发给触发器
    if (m_trigger) {
        bool handled = m_trigger->onMouseRelease(pos);
        
        // 如果触发器处理了事件，显示弹出窗口
        if (handled && !isPopupVisible()) {
            showPopup();
        }
        
        return handled;
    }
    return false;
}

bool SimplePopup::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
    // 直接转发给触发器
    if (m_trigger) {
        return m_trigger->onWheel(pos, angleDelta);
    }
    return false;
}

bool SimplePopup::tick()
{
    bool needsRedraw = false;
    
    // 处理触发器动画
    if (m_trigger) {
        needsRedraw |= m_trigger->tick();
    }
    
    return needsRedraw;
}

void SimplePopup::onThemeChanged(bool isDark)
{
    m_isDark = isDark;
    
    // 更新触发器主题
    if (m_trigger) {
        m_trigger->onThemeChanged(isDark);
    }
}

QPoint SimplePopup::calculatePopupPosition() const
{
    if (!m_parentWindow || m_viewport.isEmpty()) {
        return QPoint(100, 100);  // 默认位置
    }
    
    // 获取触发器在全局坐标系中的位置
    QPoint triggerGlobalPos = m_parentWindow->mapToGlobal(m_viewport.topLeft());
    QSize triggerSize = m_viewport.size();
    
    QPoint popupPos;
    
    switch (m_placement) {
    case Placement::Bottom:
        popupPos = QPoint(triggerGlobalPos.x(), triggerGlobalPos.y() + triggerSize.height());
        break;
    case Placement::Top:
        popupPos = QPoint(triggerGlobalPos.x(), triggerGlobalPos.y() - m_popupSize.height());
        break;
    case Placement::Right:
        popupPos = QPoint(triggerGlobalPos.x() + triggerSize.width(), triggerGlobalPos.y());
        break;
    case Placement::Left:
        popupPos = QPoint(triggerGlobalPos.x() - m_popupSize.width(), triggerGlobalPos.y());
        break;
    case Placement::BottomRight:
        popupPos = QPoint(triggerGlobalPos.x() + triggerSize.width(), 
                         triggerGlobalPos.y() + triggerSize.height());
        break;
    default:
        popupPos = QPoint(triggerGlobalPos.x(), triggerGlobalPos.y() + triggerSize.height());
        break;
    }
    
    // 应用偏移量
    popupPos += m_offset;
    
    return popupPos;
}

void SimplePopup::onPopupHidden()
{
    // 弹出窗口隐藏时的处理
    qDebug() << "SimplePopup: 弹出窗口已隐藏";
}