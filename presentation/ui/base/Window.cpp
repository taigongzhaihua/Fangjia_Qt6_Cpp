#include "Window.h"

#include <qevent.h>
#include <qlogging.h>
#include <qopenglcontext.h>
#include <qsize.h>
#include <GL/gl.h>
#include <RenderData.hpp>

Window::Window(UpdateBehavior updateBehavior)
    : QOpenGLWindow(updateBehavior)
{
    qDebug() << "Window::Window() - Base window created";
    
    // 设置动画计时器：目标60fps (约16.67ms间隔)
    m_animationTimer.setInterval(16);
    m_animationTimer.setSingleShot(false);
    connect(&m_animationTimer, &QTimer::timeout, this, &Window::onAnimationFrame);
    
    qDebug() << "Window::Window() - Animation timer configured";
}

Window::~Window()
{
    qDebug() << "Window::~Window() - Cleaning up base window";
    
    // 停止动画循环
    if (m_animationActive) {
        stopAnimationLoop();
    }
    
    qDebug() << "Window::~Window() - Base window cleanup complete";
}

void Window::initializeGL()
{
    try {
        qDebug() << "Window::initializeGL() - Initializing OpenGL context";
        
        // 初始化OpenGL函数表
        initializeOpenGLFunctions();
        
        // 设置基础OpenGL状态
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        // 初始化渲染器
        qDebug() << "Window::initializeGL() - Initializing renderer";
        m_renderer.initializeGL(this);
        
        // 调用派生类的UI初始化
        qDebug() << "Window::initializeGL() - Calling derived class initializeUI()";
        initializeUI();
        
        qDebug() << "Window::initializeGL() - OpenGL initialization complete";
    }
    catch (const std::exception& e) {
        qCritical() << "Exception in Window::initializeGL():" << e.what();
        throw;
    }
}

void Window::resizeGL(int w, int h)
{
    qDebug() << "Window::resizeGL() - Resizing to" << w << "x" << h;
    
    // 更新帧缓冲区尺寸
    m_framebufferWidth = w;
    m_framebufferHeight = h;
    
    // 设置OpenGL视口
    glViewport(0, 0, w, h);
    
    // 更新UI根容器的资源上下文
    auto* gl = QOpenGLContext::currentContext()->functions();
    m_uiRoot.updateResourceContext(&m_iconCache, gl, static_cast<float>(devicePixelRatio()));
    
    // 调用派生类的布局更新
    updateLayout();
    
    qDebug() << "Window::resizeGL() - Resize complete";
}

void Window::paintGL()
{
    // 1. 清屏
    const QColor clearColor = getClearColor();
    glClearColor(clearColor.redF(), clearColor.greenF(), clearColor.blueF(), 1.0f);
    glClear(GL_COLOR_BUFFER_BIT);
    
    // 2. 收集渲染命令
    Render::FrameData frameData;
    m_uiRoot.append(frameData);
    
    // 3. 执行渲染
    m_renderer.drawFrame(frameData, m_iconCache, static_cast<float>(devicePixelRatio()));
}

void Window::mousePressEvent(QMouseEvent* e)
{
    if (m_uiRoot.onMousePress(e->pos())) {
        requestRedraw();
        e->accept();
        return;
    }
    
    // 如果UI系统没有处理，传递给基类
    QOpenGLWindow::mousePressEvent(e);
}

void Window::mouseMoveEvent(QMouseEvent* e)
{
    if (m_uiRoot.onMouseMove(e->pos())) {
        requestRedraw();
        e->accept();
        return;
    }
    
    QOpenGLWindow::mouseMoveEvent(e);
}

void Window::mouseReleaseEvent(QMouseEvent* e)
{
    if (m_uiRoot.onMouseRelease(e->pos())) {
        requestRedraw();
        e->accept();
        return;
    }
    
    QOpenGLWindow::mouseReleaseEvent(e);
}

void Window::mouseDoubleClickEvent(QMouseEvent* e)
{
    // For double-click, we can use the same press logic since most UI components
    // handle double-clicks as special press events
    if (m_uiRoot.onMousePress(e->pos())) {
        requestRedraw();
        e->accept();
        return;
    }
    
    QOpenGLWindow::mouseDoubleClickEvent(e);
}

void Window::wheelEvent(QWheelEvent* e)
{
    if (m_uiRoot.onWheel(e->position().toPoint(), e->angleDelta())) {
        requestRedraw();
        e->accept();
        return;
    }
    
    QOpenGLWindow::wheelEvent(e);
}

void Window::keyPressEvent(QKeyEvent* e)
{
    if (m_uiRoot.onKeyPress(e->key(), e->modifiers())) {
        e->accept();
        return;
    }
    
    QOpenGLWindow::keyPressEvent(e);
}

void Window::keyReleaseEvent(QKeyEvent* e)
{
    if (m_uiRoot.onKeyRelease(e->key(), e->modifiers())) {
        e->accept();
        return;
    }
    
    QOpenGLWindow::keyReleaseEvent(e);
}

bool Window::onAnimationTick(qint64 deltaTime)
{
    // 默认实现：让UI根容器处理动画
    return m_uiRoot.tick();
}

QColor Window::getClearColor() const
{
    // 默认清屏颜色：中性灰色
    return QColor::fromRgbF(0.2f, 0.2f, 0.2f);
}

void Window::startAnimationLoop()
{
    if (!m_animationActive) {
        qDebug() << "Window::startAnimationLoop() - Starting animation";
        m_animationActive = true;
        m_animationClock.start();
        m_animationTimer.start();
    }
}

void Window::stopAnimationLoop()
{
    if (m_animationActive) {
        qDebug() << "Window::stopAnimationLoop() - Stopping animation";
        m_animationActive = false;
        m_animationTimer.stop();
    }
}

void Window::requestRedraw()
{
    update();
}

void Window::onAnimationFrame()
{
    if (!m_animationActive) {
        return;
    }
    
    // 计算帧间隔时间
    const qint64 deltaTime = m_animationClock.restart();
    
    // 调用派生类的动画更新
    const bool needsMore = onAnimationTick(deltaTime);
    
    // 如果不需要更多帧，停止动画循环
    if (!needsMore) {
        stopAnimationLoop();
    }
    
    // 请求重绘
    requestRedraw();
}