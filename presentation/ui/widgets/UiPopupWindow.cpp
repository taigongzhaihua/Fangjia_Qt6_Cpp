#include "UiPopupWindow.h"

#include <QMouseEvent>
#include <QWheelEvent>
#include <QKeyEvent>
#include <QFocusEvent>
#include <QShowEvent>
#include <QHideEvent>
#include <QScreen>
#include <QGuiApplication>
#include <QDebug>
#include <GL/gl.h>

UiPopupWindow::UiPopupWindow(QWindow* parent, UpdateBehavior updateBehavior)
    : QOpenGLWindow(updateBehavior)
    , m_backgroundColor(255, 255, 255, 240)  // 半透明白色背景
{
    // 设置父窗口关系
    if (parent) {
        setParent(parent);
    }

    // 设置窗口属性
    setFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    
    // 设置动画定时器
    connect(&m_animTimer, &QTimer::timeout, this, &UiPopupWindow::onAnimationTick);
    m_animTimer.setTimerType(Qt::PreciseTimer);
    m_animTimer.setInterval(16);  // ~60 FPS
    m_animClock.start();
}

UiPopupWindow::~UiPopupWindow()
{
    // 清理OpenGL资源
    if (context()) {
        makeCurrent();
        m_iconCache.releaseAll(this);
        m_renderer.releaseGL();
        doneCurrent();
    }
}

void UiPopupWindow::setContent(IUiComponent* content)
{
    if (m_content == content) return;

    // 移除旧内容
    if (m_content) {
        m_uiRoot.remove(m_content);
    }

    // 设置新内容
    m_content = content;
    if (m_content) {
        m_uiRoot.add(m_content);
        updateLayout();
        updateResourceContext();
    }
}

void UiPopupWindow::setBackgroundColor(const QColor& color)
{
    m_backgroundColor = color;
    update();
}

void UiPopupWindow::showAt(const QPoint& globalPos, const QSize& size)
{
    // 设置窗口大小和位置
    resize(size);
    setPosition(globalPos);
    
    // 确保窗口在屏幕范围内
    if (screen()) {
        const QRect screenGeometry = screen()->availableGeometry();
        QPoint adjustedPos = globalPos;
        
        // 调整X坐标
        if (adjustedPos.x() + size.width() > screenGeometry.right()) {
            adjustedPos.setX(screenGeometry.right() - size.width());
        }
        if (adjustedPos.x() < screenGeometry.left()) {
            adjustedPos.setX(screenGeometry.left());
        }
        
        // 调整Y坐标
        if (adjustedPos.y() + size.height() > screenGeometry.bottom()) {
            adjustedPos.setY(screenGeometry.bottom() - size.height());
        }
        if (adjustedPos.y() < screenGeometry.top()) {
            adjustedPos.setY(screenGeometry.top());
        }
        
        setPosition(adjustedPos);
    }
    
    // 显示窗口
    show();
    
    // 更新布局和资源
    updateLayout();
    updateResourceContext();
    
    // 请求焦点
    requestActivate();
}

void UiPopupWindow::hidePopup()
{
    hide();
    emit popupHidden();
}

void UiPopupWindow::applyTheme(bool isDark)
{
    m_isDark = isDark;
    
    // 更新背景颜色
    if (isDark) {
        m_backgroundColor = QColor(45, 45, 48, 240);  // 半透明深色背景
    } else {
        m_backgroundColor = QColor(255, 255, 255, 240);  // 半透明白色背景
    }
    
    // 传播主题变化到内容
    if (m_content) {
        m_uiRoot.propagateThemeChange(isDark);
    }
    
    update();
}

void UiPopupWindow::initializeGL()
{
    qDebug() << "UiPopupWindow::initializeGL start";
    
    initializeOpenGLFunctions();
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    
    m_renderer.initializeGL(this);
    
    updateLayout();
    updateResourceContext();
    
    qDebug() << "UiPopupWindow::initializeGL end";
}

void UiPopupWindow::resizeGL(int w, int h)
{
    m_fbWpx = w;
    m_fbHpx = h;
    m_renderer.resize(w, h);
    updateLayout();
}

void UiPopupWindow::paintGL()
{
    // 设置背景色
    glClearColor(
        m_backgroundColor.redF(), 
        m_backgroundColor.greenF(), 
        m_backgroundColor.blueF(), 
        m_backgroundColor.alphaF()
    );
    glClear(GL_COLOR_BUFFER_BIT);

    // 绘制圆角背景（如果需要）
    if (m_cornerRadius > 0.0f) {
        Render::FrameData backgroundData;
        backgroundData.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(0, 0, width(), height()),
            .radiusPx = m_cornerRadius,
            .color = m_backgroundColor,
            .clipRect = QRectF(0, 0, width(), height())
        });
        m_renderer.drawFrame(backgroundData, m_iconCache, static_cast<float>(devicePixelRatio()));
    }

    // 绘制内容
    if (m_content) {
        Render::FrameData frameData;
        m_uiRoot.append(frameData);
        m_renderer.drawFrame(frameData, m_iconCache, static_cast<float>(devicePixelRatio()));
    }
}

void UiPopupWindow::mousePressEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && m_content) {
        if (m_uiRoot.onMousePress(e->pos())) {
            update();
            e->accept();
            return;
        }
    }
    QOpenGLWindow::mousePressEvent(e);
}

void UiPopupWindow::mouseMoveEvent(QMouseEvent* e)
{
    if (m_content) {
        const bool handled = m_uiRoot.onMouseMove(e->pos());
        setCursor(handled ? Qt::PointingHandCursor : Qt::ArrowCursor);
        if (handled) {
            update();
        }
    }
    QOpenGLWindow::mouseMoveEvent(e);
}

void UiPopupWindow::mouseReleaseEvent(QMouseEvent* e)
{
    if (e->button() == Qt::LeftButton && m_content) {
        const bool handled = m_uiRoot.onMouseRelease(e->pos());
        if (handled) {
            // 启动动画定时器（如果需要）
            if (!m_animTimer.isActive()) {
                m_animClock.start();
                m_animTimer.start();
            }
            update();
            e->accept();
            return;
        }
    }
    QOpenGLWindow::mouseReleaseEvent(e);
}

void UiPopupWindow::wheelEvent(QWheelEvent* e)
{
    if (m_content) {
        if (m_uiRoot.onWheel(e->position().toPoint(), e->angleDelta())) {
            // 启动动画定时器（如果需要）
            if (!m_animTimer.isActive()) {
                m_animClock.start();
                m_animTimer.start();
            }
            update();
            e->accept();
            return;
        }
    }
    QOpenGLWindow::wheelEvent(e);
}

void UiPopupWindow::keyPressEvent(QKeyEvent* e)
{
    // Esc键关闭弹出窗口
    if (e->key() == Qt::Key_Escape) {
        hidePopup();
        e->accept();
        return;
    }
    
    if (m_content) {
        if (m_uiRoot.onKeyPress(e->key(), e->modifiers())) {
            if (!m_animTimer.isActive()) {
                m_animClock.start();
                m_animTimer.start();
            }
            update();
            e->accept();
            return;
        }
    }
    QOpenGLWindow::keyPressEvent(e);
}

void UiPopupWindow::keyReleaseEvent(QKeyEvent* e)
{
    if (m_content) {
        if (m_uiRoot.onKeyRelease(e->key(), e->modifiers())) {
            if (!m_animTimer.isActive()) {
                m_animClock.start();
                m_animTimer.start();
            }
            update();
            e->accept();
            return;
        }
    }
    QOpenGLWindow::keyReleaseEvent(e);
}

void UiPopupWindow::focusOutEvent(QFocusEvent* e)
{
    // 失去焦点时隐藏弹出窗口（可选行为）
    // hidePopup();
    QOpenGLWindow::focusOutEvent(e);
}

void UiPopupWindow::showEvent(QShowEvent* e)
{
    QOpenGLWindow::showEvent(e);
    updateLayout();
    updateResourceContext();
}

void UiPopupWindow::hideEvent(QHideEvent* e)
{
    QOpenGLWindow::hideEvent(e);
    emit popupHidden();
}

void UiPopupWindow::onAnimationTick()
{
    bool hasAnimation = false;
    
    if (m_content) {
        hasAnimation = m_uiRoot.tick();
    }
    
    if (!hasAnimation) {
        m_animTimer.stop();
    }
    
    update();
}

void UiPopupWindow::updateLayout()
{
    const QSize winSize = size();
    
    if (m_content) {
        m_uiRoot.updateLayout(winSize);
    }
}

void UiPopupWindow::updateResourceContext()
{
    if (m_content && context()) {
        m_uiRoot.updateResourceContext(m_iconCache, this, static_cast<float>(devicePixelRatio()));
    }
}