/*
 * 文件名：simple_popup_demo.cpp
 * 职责：简化的弹出控件演示，展示基本的弹出窗口功能。
 * 依赖：最小化依赖，仅使用必要的Qt组件。
 */

#include <QGuiApplication>
#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QTimer>
#include <QDebug>
#include <GL/gl.h>

// 最小化的弹出窗口演示
class SimplePopupWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    SimplePopupWindow(QWindow* parent = nullptr) : QOpenGLWindow(parent) {
        setFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
        if (parent) {
            setParent(parent);
        }
    }

    void showAt(const QPoint& globalPos, const QSize& size) {
        resize(size);
        setPosition(globalPos);
        show();
        requestActivate();
    }

protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void paintGL() override {
        // 绘制半透明背景
        glClearColor(0.0f, 0.5f, 1.0f, 0.8f);  // 半透明蓝色
        glClear(GL_COLOR_BUFFER_BIT);
        
        // 简单的渐变效果
        glBegin(GL_QUADS);
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        glVertex2f(-1.0f, -1.0f);
        glColor4f(0.8f, 0.9f, 1.0f, 0.9f);
        glVertex2f(1.0f, -1.0f);
        glColor4f(0.6f, 0.8f, 1.0f, 0.9f);
        glVertex2f(1.0f, 1.0f);
        glColor4f(0.9f, 0.95f, 1.0f, 0.9f);
        glVertex2f(-1.0f, 1.0f);
        glEnd();
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            qDebug() << "弹出窗口被点击";
            // 可以在这里添加关闭逻辑
            // hide();
        }
        QOpenGLWindow::mousePressEvent(e);
    }

    void keyPressEvent(QKeyEvent* e) override {
        if (e->key() == Qt::Key_Escape) {
            hide();
        }
        QOpenGLWindow::keyPressEvent(e);
    }
};

// 主演示窗口
class PopupDemoWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    PopupDemoWindow() {
        setTitle("弹出控件演示 - 点击窗口显示弹出");
        resize(400, 300);
        
        // 创建弹出窗口
        m_popup = std::make_unique<SimplePopupWindow>(this);
    }

protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void paintGL() override {
        // 绘制主窗口背景
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        // 绘制一个简单的按钮区域指示
        glBegin(GL_QUADS);
        glColor3f(0.4f, 0.6f, 0.8f);
        glVertex2f(-0.3f, -0.1f);
        glVertex2f(0.3f, -0.1f);
        glVertex2f(0.3f, 0.1f);
        glVertex2f(-0.3f, 0.1f);
        glEnd();
        
        // 绘制按钮边框
        glColor3f(1.0f, 1.0f, 1.0f);
        glLineWidth(2.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(-0.3f, -0.1f);
        glVertex2f(0.3f, -0.1f);
        glVertex2f(0.3f, 0.1f);
        glVertex2f(-0.3f, 0.1f);
        glEnd();
    }

    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            // 计算弹出位置（在鼠标位置附近）
            const QPoint globalPos = mapToGlobal(e->pos());
            const QPoint popupPos = globalPos + QPoint(10, 10);
            
            qDebug() << "显示弹出窗口在位置:" << popupPos;
            
            // 显示弹出窗口
            m_popup->showAt(popupPos, QSize(200, 150));
        }
        QOpenGLWindow::mousePressEvent(e);
    }

    void keyPressEvent(QKeyEvent* e) override {
        if (e->key() == Qt::Key_Space) {
            // 空格键在窗口外部显示弹出
            const QPoint centerPos = position() + QPoint(width() / 2, height() / 2);
            const QPoint outsidePos = centerPos + QPoint(50, -100);
            
            qDebug() << "在窗口外部显示弹出窗口:" << outsidePos;
            m_popup->showAt(outsidePos, QSize(250, 120));
        }
        QOpenGLWindow::keyPressEvent(e);
    }

private:
    std::unique_ptr<SimplePopupWindow> m_popup;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    qDebug() << "弹出控件演示程序";
    qDebug() << "- 点击主窗口显示弹出";
    qDebug() << "- 按空格键在窗口外部显示弹出";
    qDebug() << "- 按ESC键关闭弹出窗口";
    
    PopupDemoWindow window;
    window.show();
    
    return app.exec();
}

#include "simple_popup_demo.moc"