/*
 * 文件名：minimal_popup_test.cpp
 * 职责：最小化的弹出控件测试，验证架构设计
 */

#include <QGuiApplication>
#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QKeyEvent>
#include <QDebug>
#include <QTimer>
#include <memory>
#include <functional>

// 最小化的测试实现，不依赖复杂的UI框架

/// 简化的弹出窗口
class MinimalPopupWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    MinimalPopupWindow(QWindow* parent = nullptr) : QOpenGLWindow(NoPartialUpdate, parent) {
        setFlags(Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint);
    }

    void setContent(const QString& text) { m_content = text; }

    void showAt(const QPoint& pos, const QSize& size) {
        resize(size);
        setPosition(pos);
        show();
        requestActivate();
        qDebug() << "MinimalPopupWindow: 显示在位置" << pos << "大小" << size;
    }

    void hidePopup() {
        hide();
        qDebug() << "MinimalPopupWindow: 隐藏";
    }

protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void paintGL() override {
        // 绘制半透明背景
        glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 绘制背景矩形
        glColor4f(1.0f, 1.0f, 1.0f, 0.9f);
        glBegin(GL_QUADS);
        glVertex2f(5, 5);
        glVertex2f(width() - 5, 5);
        glVertex2f(width() - 5, height() - 5);
        glVertex2f(5, height() - 5);
        glEnd();

        // 绘制边框
        glColor4f(0.5f, 0.5f, 0.5f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(5, 5);
        glVertex2f(width() - 5, 5);
        glVertex2f(width() - 5, height() - 5);
        glVertex2f(5, height() - 5);
        glEnd();

        // 绘制内容标识（简单的矩形）
        glColor4f(0.2f, 0.6f, 0.8f, 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(20, 20);
        glVertex2f(width() - 20, 20);
        glVertex2f(width() - 20, height() - 20);
        glVertex2f(20, height() - 20);
        glEnd();
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) {
            hidePopup();
        }
    }

private:
    QString m_content;
};

/// 简化的弹出控件
class MinimalPopup
{
public:
    MinimalPopup(QWindow* parent) : m_parentWindow(parent) {
        m_popupWindow = std::make_unique<MinimalPopupWindow>(parent);
    }

    void setTriggerArea(const QRect& area) { m_triggerArea = area; }
    void setPopupSize(const QSize& size) { m_popupSize = size; }
    void setContent(const QString& content) { m_popupWindow->setContent(content); }

    bool handleMousePress(const QPoint& pos) {
        if (m_triggerArea.contains(pos)) {
            qDebug() << "MinimalPopup: 触发器被点击";
            return true;
        }
        return false;
    }

    bool handleMouseRelease(const QPoint& pos) {
        if (m_triggerArea.contains(pos)) {
            showPopup();
            return true;
        }
        return false;
    }

    void showPopup() {
        if (!m_parentWindow) return;

        QPoint globalPos = m_parentWindow->mapToGlobal(
            QPoint(m_triggerArea.x(), m_triggerArea.bottom())
        );
        m_popupWindow->showAt(globalPos, m_popupSize);
    }

    void hidePopup() {
        m_popupWindow->hidePopup();
    }

    QRect getTriggerArea() const { return m_triggerArea; }

private:
    QWindow* m_parentWindow;
    std::unique_ptr<MinimalPopupWindow> m_popupWindow;
    QRect m_triggerArea;
    QSize m_popupSize{ 200, 100 };
};

/// 测试主窗口
class TestMainWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    TestMainWindow() : QOpenGLWindow() {
        resize(600, 400);
        setTitle("Minimal Popup Test - 简化弹出控件测试");

        // 创建弹出控件
        m_popup = std::make_unique<MinimalPopup>(this);
        m_popup->setTriggerArea(QRect(50, 50, 120, 40));
        m_popup->setPopupSize(QSize(200, 100));
        m_popup->setContent("Test Popup Content");

        qDebug() << "测试主窗口创建完成";
    }

protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    }

    void paintGL() override {
        // 清除背景
        glClearColor(0.9f, 0.9f, 0.9f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);

        // 绘制触发器按钮
        QRect trigger = m_popup->getTriggerArea();
        QColor btnColor = m_buttonHovered ? QColor(100, 149, 237) : QColor(70, 130, 180);

        glColor4f(btnColor.redF(), btnColor.greenF(), btnColor.blueF(), 1.0f);
        glBegin(GL_QUADS);
        glVertex2f(trigger.left(), trigger.top());
        glVertex2f(trigger.right(), trigger.top());
        glVertex2f(trigger.right(), trigger.bottom());
        glVertex2f(trigger.left(), trigger.bottom());
        glEnd();

        // 绘制按钮边框
        glColor4f(0.0f, 0.0f, 0.0f, 1.0f);
        glBegin(GL_LINE_LOOP);
        glVertex2f(trigger.left(), trigger.top());
        glVertex2f(trigger.right(), trigger.top());
        glVertex2f(trigger.right(), trigger.bottom());
        glVertex2f(trigger.left(), trigger.bottom());
        glEnd();

        // 绘制按钮文本标识（白色矩形）
        QRect textArea = trigger.adjusted(8, 4, -8, -4);
        glColor4f(1.0f, 1.0f, 1.0f, 0.8f);
        glBegin(GL_QUADS);
        glVertex2f(textArea.left(), textArea.top());
        glVertex2f(textArea.right(), textArea.top());
        glVertex2f(textArea.right(), textArea.bottom());
        glVertex2f(textArea.left(), textArea.bottom());
        glEnd();
    }

    void mousePressEvent(QMouseEvent* event) override {
        if (m_popup->handleMousePress(event->pos())) {
            m_buttonPressed = true;
            update();
        }
    }

    void mouseMoveEvent(QMouseEvent* event) override {
        bool wasHovered = m_buttonHovered;
        m_buttonHovered = m_popup->getTriggerArea().contains(event->pos());
        if (wasHovered != m_buttonHovered) {
            update();
        }
    }

    void mouseReleaseEvent(QMouseEvent* event) override {
        if (m_buttonPressed) {
            m_buttonPressed = false;
            if (m_popup->handleMouseRelease(event->pos())) {
                // 按钮被点击
                qDebug() << "主窗口：按钮被点击，应该显示弹出窗口";
            }
            update();
        }
    }

    void keyPressEvent(QKeyEvent* event) override {
        if (event->key() == Qt::Key_Escape) {
            m_popup->hidePopup();
        } else if (event->key() == Qt::Key_Space) {
            m_popup->showPopup();
        }
    }

private:
    std::unique_ptr<MinimalPopup> m_popup;
    bool m_buttonHovered = false;
    bool m_buttonPressed = false;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);

    qDebug() << "=== 简化弹出控件测试程序 ===";
    qDebug() << "操作说明:";
    qDebug() << "1. 点击蓝色按钮显示弹出窗口";
    qDebug() << "2. 按ESC键关闭弹出窗口";
    qDebug() << "3. 按空格键直接显示弹出窗口";

    TestMainWindow window;
    window.show();

    return app.exec();
}

//#include "minimal_popup_test.moc"