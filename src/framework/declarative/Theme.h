#pragma once
#include <qcolor.h>
#include <functional>

namespace UI {

// 主题数据
struct ThemeData {
    // 颜色
    QColor primary{0, 122, 255};
    QColor secondary{108, 117, 125};
    QColor background{255, 255, 255};
    QColor surface{248, 249, 250};
    QColor error{220, 53, 69};
    QColor onPrimary{255, 255, 255};
    QColor onSecondary{255, 255, 255};
    QColor onBackground{33, 37, 41};
    QColor onSurface{33, 37, 41};
    QColor onError{255, 255, 255};
    
    // 文字样式
    struct {
        int h1{32};
        int h2{24};
        int h3{20};
        int body1{16};
        int body2{14};
        int caption{12};
    } fontSize;
    
    // 间距
    struct {
        int xs{4};
        int sm{8};
        int md{16};
        int lg{24};
        int xl{32};
    } spacing;
    
    // 圆角
    struct {
        float sm{4.0f};
        float md{8.0f};
        float lg{16.0f};
    } radius;
    
    // 生成深色主题
    static ThemeData dark() {
        ThemeData theme;
        theme.primary = QColor(66, 165, 245);
        theme.background = QColor(18, 18, 18);
        theme.surface = QColor(33, 33, 33);
        theme.onBackground = QColor(255, 255, 255);
        theme.onSurface = QColor(255, 255, 255);
        return theme;
    }
    
    // 生成浅色主题
    static ThemeData light() {
        return ThemeData{}; // 使用默认值
    }
};

// 主题组件
class Theme : public Widget {
public:
    Theme(ThemeData data, WidgetPtr child) 
        : m_data(std::move(data)), m_child(std::move(child)) {}
    
    std::unique_ptr<IUiComponent> build() const override {
        // 将主题数据传递给子组件
        return m_child->build();
    }
    
    static const ThemeData& of() {
        // 返回当前主题
        static ThemeData currentTheme;
        return currentTheme;
    }
    
private:
    ThemeData m_data;
    WidgetPtr m_child;
};

// 主题构建器
class ThemedBuilder : public Widget {
public:
    using BuildFunc = std::function<WidgetPtr(const ThemeData&)>;
    
    explicit ThemedBuilder(BuildFunc builder) : m_builder(std::move(builder)) {}
    
    std::unique_ptr<IUiComponent> build() const override {
        return m_builder(Theme::of())->build();
    }
    
private:
    BuildFunc m_builder;
};

} // namespace UI