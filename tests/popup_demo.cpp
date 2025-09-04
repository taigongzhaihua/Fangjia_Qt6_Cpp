/*
 * 文件名：popup_demo.cpp
 * 职责：弹出控件功能演示程序，展示可超出窗口边界的弹出控件功能。
 * 依赖：Qt6 OpenGL、UiPopup、UiPopupWindow。
 * 线程：仅在UI线程使用。
 * 备注：独立的演示程序，用于验证弹出控件的基本功能。
 */

#include <QGuiApplication>
#include <QOpenGLWindow>
#include <QOpenGLFunctions>
#include <QMouseEvent>
#include <QDebug>
#include <memory>

// 引入必要的UI框架组件
#include "../presentation/ui/widgets/UiPopup.h"
#include "../presentation/ui/widgets/UiPopupWindow.h"
#include "../presentation/ui/base/UiComponent.hpp"
#include "../presentation/ui/base/UiContent.hpp"
#include "../presentation/ui/base/ILayoutable.hpp"
#include "../presentation/ui/containers/UiRoot.h"
#include "../infrastructure/gfx/Renderer.h"
#include "../infrastructure/gfx/IconCache.h"

// 简单的演示按钮组件
class DemoButton : public IUiComponent, public IUiContent, public ILayoutable
{
public:
    explicit DemoButton(const QString& text) : m_text(text) {}
    
    void setText(const QString& text) { m_text = text; }
    void setOnClick(std::function<void()> callback) { m_onClick = callback; }
    
    // ILayoutable
    QSize measure(const SizeConstraints& cs) override {
        return QSize(std::clamp(120, cs.minW, cs.maxW), 
                    std::clamp(32, cs.minH, cs.maxH));
    }
    
    void arrange(const QRect& finalRect) override {
        m_viewport = finalRect;
    }
    
    // IUiContent
    void setViewportRect(const QRect& r) override {
        m_viewport = r;
    }
    
    // IUiComponent
    void updateLayout(const QSize& windowSize) override {}
    
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override {
        m_cache = &cache;
        m_gl = gl;
        m_dpr = devicePixelRatio;
    }
    
    void append(Render::FrameData& fd) const override {
        if (!m_viewport.isValid()) return;
        
        // 绘制按钮背景
        QColor bgColor = m_hovered ? QColor(100, 149, 237) : QColor(70, 130, 180);
        if (m_pressed) bgColor = bgColor.darker(120);
        
        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(m_viewport),
            .radiusPx = 6.0f,
            .color = bgColor,
            .clipRect = QRectF(m_viewport)
        });
        
        // 绘制文本（简化实现，实际应该使用文本渲染系统）
        // 这里我们用一个简单的矩形表示文本区域
        const QRect textRect = m_viewport.adjusted(8, 4, -8, -4);
        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(textRect),
            .radiusPx = 2.0f,
            .color = QColor(255, 255, 255, 200),
            .clipRect = QRectF(m_viewport)
        });
    }
    
    bool onMousePress(const QPoint& pos) override {
        if (m_viewport.contains(pos)) {
            m_pressed = true;
            return true;
        }
        return false;
    }
    
    bool onMouseMove(const QPoint& pos) override {
        const bool wasHovered = m_hovered;
        m_hovered = m_viewport.contains(pos);
        return wasHovered != m_hovered;
    }
    
    bool onMouseRelease(const QPoint& pos) override {
        if (m_pressed && m_viewport.contains(pos)) {
            m_pressed = false;
            if (m_onClick) m_onClick();
            return true;
        }
        m_pressed = false;
        return false;
    }
    
    bool tick() override { return false; }
    QRect bounds() const override { return m_viewport; }
    void onThemeChanged(bool isDark) override { m_isDark = isDark; }
    
private:
    QString m_text;
    QRect m_viewport;
    bool m_pressed{ false };
    bool m_hovered{ false };
    bool m_isDark{ false };
    std::function<void()> m_onClick;
    
    IconCache* m_cache{ nullptr };
    QOpenGLFunctions* m_gl{ nullptr };
    float m_dpr{ 1.0f };
};

// 弹出内容演示组件
class PopupContent : public IUiComponent, public IUiContent, public ILayoutable
{
public:
    PopupContent() {
        // 创建几个演示按钮
        for (int i = 0; i < 3; ++i) {
            auto btn = std::make_unique<DemoButton>(QString("选项 %1").arg(i + 1));
            btn->setOnClick([i]() {
                qDebug() << "点击了选项" << (i + 1);
            });
            m_buttons.push_back(std::move(btn));
        }
    }
    
    // ILayoutable
    QSize measure(const SizeConstraints& cs) override {
        return QSize(std::clamp(150, cs.minW, cs.maxW),
                    std::clamp(100, cs.minH, cs.maxH));
    }
    
    void arrange(const QRect& finalRect) override {
        m_viewport = finalRect;
        layoutButtons();
    }
    
    // IUiContent
    void setViewportRect(const QRect& r) override {
        m_viewport = r;
        layoutButtons();
    }
    
    // IUiComponent
    void updateLayout(const QSize& windowSize) override {}
    
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override {
        m_cache = &cache;
        m_gl = gl;
        m_dpr = devicePixelRatio;
        
        for (auto& btn : m_buttons) {
            btn->updateResourceContext(cache, gl, devicePixelRatio);
        }
    }
    
    void append(Render::FrameData& fd) const override {
        for (const auto& btn : m_buttons) {
            btn->append(fd);
        }
    }
    
    bool onMousePress(const QPoint& pos) override {
        for (auto& btn : m_buttons) {
            if (btn->onMousePress(pos)) return true;
        }
        return false;
    }
    
    bool onMouseMove(const QPoint& pos) override {
        bool handled = false;
        for (auto& btn : m_buttons) {
            handled = btn->onMouseMove(pos) || handled;
        }
        return handled;
    }
    
    bool onMouseRelease(const QPoint& pos) override {
        for (auto& btn : m_buttons) {
            if (btn->onMouseRelease(pos)) return true;
        }
        return false;
    }
    
    bool tick() override { return false; }
    QRect bounds() const override { return m_viewport; }
    void onThemeChanged(bool isDark) override {
        for (auto& btn : m_buttons) {
            btn->onThemeChanged(isDark);
        }
    }
    
private:
    void layoutButtons() {
        if (!m_viewport.isValid() || m_buttons.empty()) return;
        
        const int btnHeight = 30;
        const int spacing = 5;
        const int totalHeight = static_cast<int>(m_buttons.size()) * btnHeight + 
                               (static_cast<int>(m_buttons.size()) - 1) * spacing;
        
        int y = m_viewport.y() + (m_viewport.height() - totalHeight) / 2;
        
        for (auto& btn : m_buttons) {
            const QRect btnRect(m_viewport.x() + 10, y, m_viewport.width() - 20, btnHeight);
            btn->setViewportRect(btnRect);
            y += btnHeight + spacing;
        }
    }
    
private:
    std::vector<std::unique_ptr<DemoButton>> m_buttons;
    QRect m_viewport;
    
    IconCache* m_cache{ nullptr };
    QOpenGLFunctions* m_gl{ nullptr };
    float m_dpr{ 1.0f };
};

// 演示窗口
class PopupDemoWindow : public QOpenGLWindow, protected QOpenGLFunctions
{
    Q_OBJECT

public:
    PopupDemoWindow() {
        setTitle("弹出控件演示");
        resize(400, 300);
        
        // 初始化UI组件
        initializeUI();
    }
    
protected:
    void initializeGL() override {
        initializeOpenGLFunctions();
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        m_renderer.initializeGL(this);
        updateLayout();
        updateResourceContext();
    }
    
    void resizeGL(int w, int h) override {
        m_renderer.resize(w, h);
        updateLayout();
    }
    
    void paintGL() override {
        glClearColor(0.2f, 0.2f, 0.2f, 1.0f);
        glClear(GL_COLOR_BUFFER_BIT);
        
        Render::FrameData frameData;
        m_uiRoot.append(frameData);
        m_renderer.drawFrame(frameData, m_iconCache, static_cast<float>(devicePixelRatio()));
    }
    
    void mousePressEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            if (m_uiRoot.onMousePress(e->pos())) {
                update();
                e->accept();
                return;
            }
        }
        QOpenGLWindow::mousePressEvent(e);
    }
    
    void mouseMoveEvent(QMouseEvent* e) override {
        const bool handled = m_uiRoot.onMouseMove(e->pos());
        if (handled) update();
        QOpenGLWindow::mouseMoveEvent(e);
    }
    
    void mouseReleaseEvent(QMouseEvent* e) override {
        if (e->button() == Qt::LeftButton) {
            const bool handled = m_uiRoot.onMouseRelease(e->pos());
            if (handled) {
                update();
                e->accept();
                return;
            }
        }
        QOpenGLWindow::mouseReleaseEvent(e);
    }
    
private:
    void initializeUI() {
        // 创建触发按钮
        m_triggerButton = std::make_unique<DemoButton>("显示弹出菜单");
        
        // 创建弹出内容
        m_popupContent = std::make_unique<PopupContent>();
        
        // 创建弹出控件
        m_popup = std::make_unique<UiPopup>(this);
        m_popup->setTrigger(m_triggerButton.get());
        m_popup->setPopupContent(m_popupContent.get());
        m_popup->setPopupSize(QSize(180, 120));
        m_popup->setPlacement(UiPopup::Placement::Bottom);
        m_popup->setPopupStyle(QColor(255, 255, 255, 240), 8.0f);
        
        // 设置弹出回调
        m_popup->setOnPopupVisibilityChanged([](bool visible) {
            qDebug() << "弹出窗口可见性变化:" << visible;
        });
        
        // 将弹出控件添加到UI根
        m_uiRoot.add(m_popup.get());
    }
    
    void updateLayout() {
        const QSize winSize = size();
        
        // 设置触发按钮位置（窗口中心）
        const QRect buttonRect(winSize.width() / 2 - 60, winSize.height() / 2 - 16, 120, 32);
        m_popup->setViewportRect(buttonRect);
        
        m_uiRoot.updateLayout(winSize);
    }
    
    void updateResourceContext() {
        m_uiRoot.updateResourceContext(m_iconCache, this, static_cast<float>(devicePixelRatio()));
    }
    
private:
    UiRoot m_uiRoot;
    Renderer m_renderer;
    IconCache m_iconCache;
    
    std::unique_ptr<DemoButton> m_triggerButton;
    std::unique_ptr<PopupContent> m_popupContent;
    std::unique_ptr<UiPopup> m_popup;
};

int main(int argc, char *argv[])
{
    QGuiApplication app(argc, argv);
    
    qDebug() << "启动弹出控件演示程序...";
    
    PopupDemoWindow window;
    window.show();
    
    return app.exec();
}

#include "popup_demo.moc"