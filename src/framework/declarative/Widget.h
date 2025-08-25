#pragma once
#include "UiComponent.hpp"
#include "RenderData.hpp"
#include <memory>
#include <vector>
#include <functional>
#include <qcolor.h>
#include <qmargins.h>
#include <qsize.h>

namespace UI {

// 前向声明
class Widget;
using WidgetPtr = std::shared_ptr<Widget>;
using WidgetList = std::vector<WidgetPtr>;
using WidgetBuilder = std::function<WidgetPtr()>;

// 基础Widget类 - 所有声明式组件的基类
class Widget : public std::enable_shared_from_this<Widget> {
public:
    virtual ~Widget() = default;
    
    // 构建实际的UI组件
    virtual std::unique_ptr<IUiComponent> build() const = 0;
    
    // 链式调用的修饰器
    WidgetPtr padding(int all);
    WidgetPtr padding(int horizontal, int vertical);
    WidgetPtr padding(int left, int top, int right, int bottom);
    
    WidgetPtr margin(int all);
    WidgetPtr margin(int horizontal, int vertical);
    WidgetPtr margin(int left, int top, int right, int bottom);
    
    WidgetPtr background(QColor color, float radius = 0.0f);
    WidgetPtr border(QColor color, float width = 1.0f, float radius = 0.0f);
    
    WidgetPtr size(int width, int height);
    WidgetPtr width(int w);
    WidgetPtr height(int h);
    
    WidgetPtr visible(bool v);
    WidgetPtr opacity(float o);
    
    // 事件处理
    WidgetPtr onTap(std::function<void()> handler);
    WidgetPtr onHover(std::function<void(bool)> handler);
    
protected:
    // 装饰器数据
    struct Decorations {
        QMargins padding{0, 0, 0, 0};
        QMargins margin{0, 0, 0, 0};
        QColor backgroundColor{Qt::transparent};
        float backgroundRadius{0.0f};
        QColor borderColor{Qt::transparent};
        float borderWidth{0.0f};
        float borderRadius{0.0f};
        QSize fixedSize{-1, -1};
        bool isVisible{true};
        float opacity{1.0f};
        std::function<void()> onTap;
        std::function<void(bool)> onHover;
    } m_decorations;
    
    // 应用装饰器到组件
    void applyDecorations(IUiComponent* component) const;
};

// 便捷的智能指针创建
template<typename T, typename... Args>
std::shared_ptr<T> make_widget(Args&&... args) {
    return std::make_shared<T>(std::forward<Args>(args)...);
}

} // namespace UI