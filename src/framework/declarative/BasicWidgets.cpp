#include "BasicWidgets.h"
#include "UiBoxLayout.h"
#include "RenderData.hpp"
#include "IconLoader.h"
#include <memory>
#include <qfont.h>
#include <qopenglfunctions.h>

namespace UI {

// 简单的文本组件实现
class TextComponent : public IUiComponent {
public:
    TextComponent(const QString& text, const QColor& color, int fontSize, QFont::Weight weight, Qt::Alignment align)
        : m_text(text), m_color(color), m_fontSize(fontSize), m_fontWeight(weight), m_alignment(align) {}
    
    void updateLayout(const QSize&) override {}
    
    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float dpr) override {
        m_loader = &loader;
        m_gl = gl;
        m_dpr = dpr;
    }
    
    void append(Render::FrameData& fd) const override {
        if (!m_loader || !m_gl || m_text.isEmpty()) return;
        
        QFont font;
        font.setPixelSize(std::lround(m_fontSize * m_dpr));
        font.setWeight(m_fontWeight);
        
        const QString key = QString("text_%1_%2_%3").arg(m_text).arg(m_fontSize).arg(m_color.name());
        const int tex = m_loader->ensureTextPx(key, font, m_text, m_color, m_gl);
        const QSize ts = m_loader->textureSizePx(tex);
        
        const float w = ts.width() / m_dpr;
        const float h = ts.height() / m_dpr;
        
        QRectF dst(m_bounds.x(), m_bounds.y(), w, h);
        
        // 应用对齐
        if (m_alignment & Qt::AlignHCenter) {
            dst.moveLeft(m_bounds.center().x() - w/2);
        } else if (m_alignment & Qt::AlignRight) {
            dst.moveLeft(m_bounds.right() - w);
        }
        
        if (m_alignment & Qt::AlignVCenter) {
            dst.moveTop(m_bounds.center().y() - h/2);
        } else if (m_alignment & Qt::AlignBottom) {
            dst.moveTop(m_bounds.bottom() - h);
        }
        
        fd.images.push_back(Render::ImageCmd{
            .dstRect = dst,
            .textureId = tex,
            .srcRectPx = QRectF(0, 0, ts.width(), ts.height()),
            .tint = QColor(255,255,255,255)
        });
    }
    
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return m_bounds; }
    
    void setBounds(const QRect& bounds) { m_bounds = bounds; }
    
private:
    QString m_text;
    QColor m_color;
    int m_fontSize;
    QFont::Weight m_fontWeight;
    Qt::Alignment m_alignment;
    QRect m_bounds;
    
    IconLoader* m_loader{nullptr};
    QOpenGLFunctions* m_gl{nullptr};
    float m_dpr{1.0f};
};

std::unique_ptr<IUiComponent> Text::build() const {
    auto comp = std::make_unique<TextComponent>(m_text, m_color, m_fontSize, m_fontWeight, m_alignment);
    applyDecorations(comp.get());
    return comp;
}

// 图标组件实现
class IconComponent : public IUiComponent {
public:
    IconComponent(const QString& path, const QColor& color, int size)
        : m_path(path), m_color(color), m_size(size) {}
    
    void updateLayout(const QSize&) override {}
    
    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float dpr) override {
        m_loader = &loader;
        m_gl = gl;
        m_dpr = dpr;
    }
    
    void append(Render::FrameData& fd) const override {
        if (!m_loader || !m_gl || m_path.isEmpty()) return;
        
        // 简化处理：绘制一个彩色矩形代表图标
        fd.roundedRects.push_back(Render::RoundedRectCmd{
            .rect = QRectF(m_bounds.center().x() - m_size/2.0f,
                          m_bounds.center().y() - m_size/2.0f,
                          m_size, m_size),
            .radiusPx = m_size/4.0f,
            .color = m_color
        });
    }
    
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return m_bounds; }
    
    void setBounds(const QRect& bounds) { m_bounds = bounds; }
    
private:
    QString m_path;
    QColor m_color;
    int m_size;
    QRect m_bounds;
    
    IconLoader* m_loader{nullptr};
    QOpenGLFunctions* m_gl{nullptr};
    float m_dpr{1.0f};
};

std::unique_ptr<IUiComponent> Icon::build() const {
    auto comp = std::make_unique<IconComponent>(m_path, m_color, m_size);
    applyDecorations(comp.get());
    return comp;
}

// 按钮组件实现
class ButtonComponent : public IUiComponent {
public:
    ButtonComponent(std::unique_ptr<IUiComponent> child, Button::ButtonStyle style)
        : m_child(std::move(child)), m_style(style) {}
    
    void updateLayout(const QSize& windowSize) override {
        if (m_child) m_child->updateLayout(windowSize);
    }
    
    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float dpr) override {
        if (m_child) m_child->updateResourceContext(loader, gl, dpr);
    }
    
    void append(Render::FrameData& fd) const override {
        // 绘制按钮背景
        QColor bgColor;
        switch (m_style) {
            case Button::ButtonStyle::Primary:
                bgColor = m_pressed ? QColor(0, 102, 204) : (m_hovered ? QColor(0, 122, 255) : QColor(0, 112, 235));
                break;
            case Button::ButtonStyle::Secondary:
                bgColor = m_pressed ? QColor(90, 100, 110) : (m_hovered ? QColor(108, 117, 125) : QColor(100, 110, 120));
                break;
            case Button::ButtonStyle::Text:
                bgColor = m_pressed ? QColor(0, 0, 0, 20) : (m_hovered ? QColor(0, 0, 0, 10) : Qt::transparent);
                break;
            case Button::ButtonStyle::Outlined:
                bgColor = Qt::transparent;
                break;
        }
        
        if (bgColor.alpha() > 0) {
            fd.roundedRects.push_back(Render::RoundedRectCmd{
                .rect = QRectF(m_bounds),
                .radiusPx = 4.0f,
                .color = bgColor
            });
        }
        
        // 绘制边框（仅限Outlined样式）
        if (m_style == Button::ButtonStyle::Outlined) {
            // 简化：用一个半透明矩形模拟边框
            fd.roundedRects.push_back(Render::RoundedRectCmd{
                .rect = QRectF(m_bounds).adjusted(1, 1, -1, -1),
                .radiusPx = 3.0f,
                .color = QColor(0, 0, 0, 50)
            });
        }
        
        // 绘制子组件
        if (m_child) m_child->append(fd);
    }
    
    bool onMousePress(const QPoint& pos) override {
        if (m_bounds.contains(pos)) {
            m_pressed = true;
            return true;
        }
        return false;
    }
    
    bool onMouseMove(const QPoint& pos) override {
        bool wasHovered = m_hovered;
        m_hovered = m_bounds.contains(pos);
        return wasHovered != m_hovered;
    }
    
    bool onMouseRelease(const QPoint& pos) override {
        bool wasPressed = m_pressed;
        m_pressed = false;
        
        if (wasPressed && m_bounds.contains(pos) && m_onTap) {
            m_onTap();
            return true;
        }
        return wasPressed;
    }
    
    bool tick() override {
        return m_child ? m_child->tick() : false;
    }
    
    QRect bounds() const override { return m_bounds; }
    
    void setBounds(const QRect& bounds) { 
        m_bounds = bounds;
        if (m_child) {
            // 子组件使用相同的边界
            if (auto* comp = dynamic_cast<TextComponent*>(m_child.get())) {
                comp->setBounds(bounds);
            } else if (auto* comp = dynamic_cast<IconComponent*>(m_child.get())) {
                comp->setBounds(bounds);
            }
        }
    }
    
    void setOnTap(std::function<void()> handler) { m_onTap = std::move(handler); }
    
private:
    std::unique_ptr<IUiComponent> m_child;
    Button::ButtonStyle m_style;
    QRect m_bounds;
    bool m_hovered{false};
    bool m_pressed{false};
    std::function<void()> m_onTap;
};

std::unique_ptr<IUiComponent> Button::build() const {
    auto childComp = m_child ? m_child->build() : nullptr;
    auto comp = std::make_unique<ButtonComponent>(std::move(childComp), m_style);
    
    // 应用装饰器
    if (m_decorations.onTap) {
        comp->setOnTap(m_decorations.onTap);
    }
    
    applyDecorations(comp.get());
    return comp;
}

// 容器组件实现
std::unique_ptr<IUiComponent> Container::build() const {
    auto layout = std::make_unique<UiBoxLayout>(UiBoxLayout::Direction::Vertical);
    
    if (m_child) {
        auto childComp = m_child->build();
        layout->addChild(childComp.release(), 1.0f, 
            m_alignment == Alignment::Center ? UiBoxLayout::Alignment::Center : 
            m_alignment == Alignment::Start ? UiBoxLayout::Alignment::Start :
            m_alignment == Alignment::End ? UiBoxLayout::Alignment::End :
            UiBoxLayout::Alignment::Stretch);
    }
    
    // 应用装饰器
    if (m_decorations.backgroundColor.alpha() > 0) {
        layout->setBackgroundColor(m_decorations.backgroundColor);
        layout->setCornerRadius(m_decorations.backgroundRadius);
    }
    
    if (m_decorations.padding != QMargins()) {
        layout->setMargins(m_decorations.padding);
    }
    
    return layout;
}

} // namespace UI