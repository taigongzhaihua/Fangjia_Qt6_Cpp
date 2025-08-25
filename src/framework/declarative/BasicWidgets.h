#pragma once
#include "Widget.h"
#include <qstring.h>
#include <qfont.h>

namespace UI {

// 文本组件
class Text : public Widget {
public:
    explicit Text(QString text) : m_text(std::move(text)) {}
    
    std::shared_ptr<Text> color(QColor c) {
        m_color = c;
        return std::static_pointer_cast<Text>(shared_from_this());
    }
    
    std::shared_ptr<Text> fontSize(int size) {
        m_fontSize = size;
        return std::static_pointer_cast<Text>(shared_from_this());
    }
    
    std::shared_ptr<Text> fontWeight(QFont::Weight weight) {
        m_fontWeight = weight;
        return std::static_pointer_cast<Text>(shared_from_this());
    }
    
    std::shared_ptr<Text> align(Qt::Alignment align) {
        m_alignment = align;
        return std::static_pointer_cast<Text>(shared_from_this());
    }
    
    std::unique_ptr<IUiComponent> build() const override;
    
private:
    QString m_text;
    QColor m_color{0, 0, 0};
    int m_fontSize{14};
    QFont::Weight m_fontWeight{QFont::Normal};
    Qt::Alignment m_alignment{Qt::AlignLeft};
};

// 图标组件
class Icon : public Widget {
public:
    explicit Icon(QString path) : m_path(std::move(path)) {}
    
    std::shared_ptr<Icon> color(QColor c) {
        m_color = c;
        return std::static_pointer_cast<Icon>(shared_from_this());
    }
    
    std::shared_ptr<Icon> size(int s) {
        m_size = s;
        return std::static_pointer_cast<Icon>(shared_from_this());
    }
    
    std::unique_ptr<IUiComponent> build() const override;
    
private:
    QString m_path;
    QColor m_color{0, 0, 0};
    int m_size{24};
};

// 按钮组件
class Button : public Widget {
public:
    explicit Button(WidgetPtr child) : m_child(std::move(child)) {}
    
    std::shared_ptr<Button> style(ButtonStyle s) {
        m_style = s;
        return std::static_pointer_cast<Button>(shared_from_this());
    }
    
    std::unique_ptr<IUiComponent> build() const override;
    
    enum class ButtonStyle {
        Primary,
        Secondary,
        Text,
        Outlined
    };
    
private:
    WidgetPtr m_child;
    ButtonStyle m_style{ButtonStyle::Primary};
};

// 容器组件
class Container : public Widget {
public:
    explicit Container(WidgetPtr child = nullptr) : m_child(std::move(child)) {}
    
    std::shared_ptr<Container> child(WidgetPtr c) {
        m_child = std::move(c);
        return std::static_pointer_cast<Container>(shared_from_this());
    }
    
    std::shared_ptr<Container> alignment(Alignment align) {
        m_alignment = align;
        return std::static_pointer_cast<Container>(shared_from_this());
    }
    
    std::unique_ptr<IUiComponent> build() const override;
    
private:
    WidgetPtr m_child;
    Alignment m_alignment{Alignment::Center};
};

} // namespace UI