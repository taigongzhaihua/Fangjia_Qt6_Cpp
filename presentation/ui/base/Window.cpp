/*
 * 文件名：Window.cpp
 * 职责：基础窗口类实现，提供窗口循环和通用事件处理。
 */

#include "Window.hpp"
#include <qcolor.h>
#include <qlogging.h>
#include <GL/gl.h>

namespace fj::presentation::ui::base {

Window::Window(UpdateBehavior updateBehavior)
    : QOpenGLWindow(updateBehavior)
{
    // 启动动画定时器
    m_animTimer.setInterval(16); // 约60fps
    connect(&m_animTimer, &QTimer::timeout, this, &Window::onAnimationTimerTick);
    
    updateThemeColors();
}

Window::~Window() = default;

void Window::setTheme(Theme theme)
{
    if (m_theme != theme) {
        m_theme = theme;
        updateThemeColors();
        onThemeChanged(theme);
        emit themeChanged(theme);
        update(); // 触发重绘
    }
}

void Window::saveWindowGeometry()
{
    // 基类默认实现为空 - 子类可重写以保存到配置
    qDebug() << "Window::saveWindowGeometry - base implementation (no-op)";
}

void Window::restoreWindowGeometry()
{
    // 基类默认实现为空 - 子类可重写以从配置恢复
    qDebug() << "Window::restoreWindowGeometry - base implementation (no-op)";
}

void Window::initializeGL()
{
    initializeOpenGLFunctions();
    
    qDebug() << "Window: Initializing base OpenGL context";
    
    // 基础OpenGL设置
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LEQUAL);
    
    // 调用子类初始化
    initializeWindowGL();
    
    // 启动动画循环
    startAnimationLoop();
}

void Window::resizeGL(int w, int h)
{
    m_fbWpx = w;
    m_fbHpx = h;
    
    glViewport(0, 0, w, h);
    
    // 调用子类布局更新
    updateWindowLayout(w, h);
}

void Window::paintGL()
{
    // 清除背景色
    const auto& color = m_clearColor;
    glClearColor(color.redF(), color.greenF(), color.blueF(), color.alphaF());
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    // 调用子类渲染
    renderWindow();
}

void Window::mousePressEvent(QMouseEvent* e)
{
    // 基类默认行为 - 子类可重写以添加处理逻辑
    QOpenGLWindow::mousePressEvent(e);
}

void Window::mouseMoveEvent(QMouseEvent* e)
{
    // 基类默认行为 - 子类可重写以添加处理逻辑
    QOpenGLWindow::mouseMoveEvent(e);
}

void Window::mouseReleaseEvent(QMouseEvent* e)
{
    // 基类默认行为 - 子类可重写以添加处理逻辑
    QOpenGLWindow::mouseReleaseEvent(e);
}

void Window::mouseDoubleClickEvent(QMouseEvent* e)
{
    // 基类默认行为 - 子类可重写以添加处理逻辑
    QOpenGLWindow::mouseDoubleClickEvent(e);
}

void Window::wheelEvent(QWheelEvent* e)
{
    // 基类默认行为 - 子类可重写以添加处理逻辑
    QOpenGLWindow::wheelEvent(e);
}

void Window::keyPressEvent(QKeyEvent* e)
{
    // 基类默认行为 - 子类可重写以添加处理逻辑
    QOpenGLWindow::keyPressEvent(e);
}

void Window::keyReleaseEvent(QKeyEvent* e)
{
    // 基类默认行为 - 子类可重写以添加处理逻辑
    QOpenGLWindow::keyReleaseEvent(e);
}

void Window::onAnimationTimerTick()
{
    const bool needsMoreAnimation = onAnimationTick();
    
    if (needsMoreAnimation) {
        update(); // 触发重绘
    }
}

void Window::updateThemeColors()
{
    // 根据主题设置清除颜色
    if (m_theme == Theme::Dark) {
        m_clearColor = QColor(32, 32, 32); // 暗色背景
    } else {
        m_clearColor = QColor(248, 248, 248); // 亮色背景  
    }
}

void Window::startAnimationLoop()
{
    if (!m_animTimer.isActive()) {
        m_animClock.start();
        m_animTimer.start();
        qDebug() << "Window: Animation loop started";
    }
}

void Window::stopAnimationLoop()
{
    if (m_animTimer.isActive()) {
        m_animTimer.stop();
        qDebug() << "Window: Animation loop stopped";
    }
}

} // namespace fj::presentation::ui::base