/*
 * PopupOverlay.cpp - 弹出窗口实现
 */

#include "PopupOverlay.h"
#include "Renderer.h"
#include <QMouseEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QDebug>

PopupOverlay::PopupOverlay(QWindow* parent)
    : QOpenGLWindow(NoPartialUpdate, parent)
{
    // 设置窗口属性
    setFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    
    // 设置渲染定时器
    m_renderTimer.setSingleShot(false);
    m_renderTimer.setInterval(16); // ~60 FPS
    connect(&m_renderTimer, &QTimer::timeout, this, &PopupOverlay::onRenderTick);
}

PopupOverlay::~PopupOverlay()
{
    // 确保在正确的OpenGL上下文中释放资源
    if (m_initialized) {
        makeCurrent();
        m_renderer.releaseGL();
        m_iconCache.releaseAll(this);
    }
}

void PopupOverlay::setContent(std::unique_ptr<IUiComponent> content)
{
    m_content = std::move(content);
    
    // 如果窗口已初始化，立即更新布局
    if (m_initialized) {
        updateContentLayout();
    }
}

void PopupOverlay::showAt(const QPoint& globalPos, const QSize& size)
{
    // 设置窗口位置和大小
    setGeometry(globalPos.x(), globalPos.y(), size.width(), size.height());
    
    // 显示窗口
    show();
    
    // 开始渲染循环
    if (!m_renderTimer.isActive()) {
        m_renderTimer.start();
    }
    
    // 触发可见性回调
    if (m_onVisibilityChanged) {
        m_onVisibilityChanged(true);
    }
}

void PopupOverlay::hidePopup()
{
    // 停止渲染循环
    m_renderTimer.stop();
    
    // 隐藏窗口
    hide();
    
    // 触发可见性回调
    if (m_onVisibilityChanged) {
        m_onVisibilityChanged(false);
    }
    
    // 发送隐藏信号
    emit popupHidden();
}

void PopupOverlay::initializeGL()
{
    initializeOpenGLFunctions();
    
    // 启用混合以支持透明度
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    // 初始化渲染器
    m_renderer.initializeGL(this);
    
    m_initialized = true;
    
    // 如果有内容，立即更新布局
    if (m_content) {
        updateContentLayout();
    }
}

void PopupOverlay::resizeGL(int w, int h)
{
    // 设置视口
    glViewport(0, 0, w, h);
    
    // 更新内容矩形
    m_contentRect = QRect(0, 0, w, h);
    
    // 更新渲染器视口
    m_renderer.resize(w, h);
    
    // 更新内容布局
    updateContentLayout();
}

void PopupOverlay::paintGL()
{
    // 清除背景为透明
    glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    if (!m_content) {
        return;
    }
    
    // 渲染背景
    renderBackground();
    
    // 渲染内容
    renderContent();
}

void PopupOverlay::mousePressEvent(QMouseEvent* event)
{
    if (m_content && m_content->onMousePress(event->pos())) {
        event->accept();
        return;
    }
    
    // 点击空白区域隐藏弹出窗口
    hidePopup();
    event->accept();
}

void PopupOverlay::mouseMoveEvent(QMouseEvent* event)
{
    if (m_content) {
        m_content->onMouseMove(event->pos());
    }
    event->accept();
}

void PopupOverlay::mouseReleaseEvent(QMouseEvent* event)
{
    if (m_content) {
        m_content->onMouseRelease(event->pos());
    }
    event->accept();
}

void PopupOverlay::keyPressEvent(QKeyEvent* event)
{
    // ESC键隐藏弹出窗口
    if (event->key() == Qt::Key_Escape) {
        hidePopup();
        event->accept();
        return;
    }
    
    QOpenGLWindow::keyPressEvent(event);
}

void PopupOverlay::focusOutEvent(QFocusEvent* event)
{
    // 失去焦点时隐藏弹出窗口
    Q_UNUSED(event)
    hidePopup();
}

void PopupOverlay::hideEvent(QHideEvent* event)
{
    Q_UNUSED(event)
    
    // 停止渲染
    m_renderTimer.stop();
    
    // 触发可见性回调
    if (m_onVisibilityChanged) {
        m_onVisibilityChanged(false);
    }
}

void PopupOverlay::onRenderTick()
{
    // 更新内容动画状态
    bool needsUpdate = false;
    
    if (m_content) {
        needsUpdate = m_content->tick();
    }
    
    // 如果需要更新，触发重绘
    if (needsUpdate) {
        update();
    }
}

void PopupOverlay::updateContentLayout()
{
    if (!m_content || !m_initialized) {
        return;
    }
    
    // 更新资源上下文
    m_content->updateResourceContext(m_iconCache, this, devicePixelRatio());
    
    // 更新布局
    m_content->updateLayout(size());
    
    // 设置内容视口（如果支持的话）
    if (auto* contentInterface = dynamic_cast<IUiContent*>(m_content.get())) {
        contentInterface->setViewportRect(m_contentRect);
    }
}

void PopupOverlay::renderBackground()
{
    // 使用渲染器绘制圆角背景矩形
    Render::RoundedRectCmd bgCmd;
    bgCmd.rect = QRectF(0, 0, width(), height());  // 填充整个窗口
    bgCmd.radiusPx = m_cornerRadius;
    bgCmd.color = m_backgroundColor;
    bgCmd.clipRect = QRectF();  // 不需要剪裁
    
    Render::FrameData bgFrameData;
    bgFrameData.roundedRects.push_back(bgCmd);
    
    m_renderer.drawFrame(bgFrameData, m_iconCache, devicePixelRatio());
}

void PopupOverlay::renderContent()
{
    if (!m_content) {
        return;
    }
    
    // 创建渲染数据容器
    Render::FrameData frameData;
    
    // 让内容添加渲染数据
    m_content->append(frameData);
    
    // 使用渲染器绘制frameData
    m_renderer.drawFrame(frameData, m_iconCache, devicePixelRatio());
}

void PopupOverlay::forwardThemeChange(bool isDark)
{
    if (m_content) {
        m_content->onThemeChanged(isDark);
    }
}