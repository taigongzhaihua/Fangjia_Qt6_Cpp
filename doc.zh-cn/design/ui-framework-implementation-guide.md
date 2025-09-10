# UI框架重构实施指南

## 1. 实施概述

本文档为UI框架重构的具体实施提供详细指南，包括代码示例、实施步骤和最佳实践。

## 2. 核心基础类实现

### 2.1 UIElement基础类

```cpp
// presentation/ui/core/UIElement.hpp
#pragma once
#include <QPoint>
#include <QSize>
#include <QRect>
#include <memory>
#include <functional>

// 前向声明
class IRenderContext;
class InputEvent;
class PropertyChangeArgs;
class SizeConstraints;

// 基础UI元素接口
class IUIElement {
public:
    virtual ~IUIElement() = default;
    
    // === 布局系统 ===
    virtual QSize measureSize(const SizeConstraints& constraints) = 0;
    virtual void arrange(const QRect& finalRect) = 0;
    
    // === 渲染系统 ===
    virtual void render(IRenderContext& context) = 0;
    
    // === 命中测试 ===
    virtual bool hitTest(const QPoint& point) = 0;
    
    // === 事件处理 ===
    virtual bool onInput(const InputEvent& event) = 0;
    
    // === 属性系统 ===
    virtual void onPropertyChanged(const PropertyChangeArgs& args) = 0;
    
    // === 基础属性 ===
    virtual QRect bounds() const = 0;
    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible) = 0;
    virtual bool isEnabled() const = 0;
    virtual void setEnabled(bool enabled) = 0;
    
    // === 父子关系 ===
    virtual IUIElement* parent() const = 0;
    virtual void setParent(IUIElement* parent) = 0;
};

// UIElement基础实现
class UIElement : public IUIElement {
private:
    QRect m_bounds;
    bool m_visible = true;
    bool m_enabled = true;
    IUIElement* m_parent = nullptr;
    
protected:
    // 布局相关
    virtual QSize onMeasure(const SizeConstraints& constraints);
    virtual void onArrange(const QRect& finalRect);
    
    // 渲染相关
    virtual void onRender(IRenderContext& context);
    
    // 输入处理
    virtual bool onInputEvent(const InputEvent& event);
    
    // 属性变化
    virtual void onPropertyChange(const QString& propertyName, const QVariant& value);

public:
    UIElement() = default;
    virtual ~UIElement() = default;
    
    // IUIElement实现
    QSize measureSize(const SizeConstraints& constraints) override;
    void arrange(const QRect& finalRect) override;
    void render(IRenderContext& context) override;
    bool hitTest(const QPoint& point) override;
    bool onInput(const InputEvent& event) override;
    void onPropertyChanged(const PropertyChangeArgs& args) override;
    
    // 基础属性实现
    QRect bounds() const override { return m_bounds; }
    bool isVisible() const override { return m_visible; }
    void setVisible(bool visible) override;
    bool isEnabled() const override { return m_enabled; }
    void setEnabled(bool enabled) override;
    
    // 父子关系
    IUIElement* parent() const override { return m_parent; }
    void setParent(IUIElement* parent) override { m_parent = parent; }
    
protected:
    void setBounds(const QRect& bounds) { m_bounds = bounds; }
    void invalidateVisual();
    void invalidateLayout();
};
```

### 2.2 FrameworkElement扩展

```cpp
// presentation/ui/core/FrameworkElement.hpp
#pragma once
#include "UIElement.hpp"
#include <QString>
#include <QVariant>
#include <unordered_map>

class Style;
class Template;
class Theme;
class DataBinding;

class IFrameworkElement : public IUIElement {
public:
    // === 样式系统 ===
    virtual void applyTemplate() = 0;
    virtual void applyStyle(const Style& style) = 0;
    
    // === 数据绑定 ===
    virtual void updateDataBinding() = 0;
    virtual void addBinding(const QString& property, std::unique_ptr<DataBinding> binding) = 0;
    
    // === 主题系统 ===
    virtual void onThemeChanged(const Theme& theme) = 0;
    
    // === 属性系统 ===
    virtual void setProperty(const QString& name, const QVariant& value) = 0;
    virtual QVariant getProperty(const QString& name) const = 0;
};

class FrameworkElement : public UIElement, public IFrameworkElement {
private:
    std::unordered_map<QString, QVariant> m_properties;
    std::unordered_map<QString, std::unique_ptr<DataBinding>> m_bindings;
    std::unique_ptr<Style> m_style;
    std::unique_ptr<Template> m_template;
    
protected:
    // 模板和样式应用
    virtual void onApplyTemplate();
    virtual void onStyleChanged(const Style& oldStyle, const Style& newStyle);
    virtual void onThemeChange(const Theme& theme);
    
public:
    FrameworkElement() = default;
    virtual ~FrameworkElement() = default;
    
    // IFrameworkElement实现
    void applyTemplate() override;
    void applyStyle(const Style& style) override;
    void updateDataBinding() override;
    void addBinding(const QString& property, std::unique_ptr<DataBinding> binding) override;
    void onThemeChanged(const Theme& theme) override;
    
    // 属性系统
    void setProperty(const QString& name, const QVariant& value) override;
    QVariant getProperty(const QString& name) const override;
    
    // 样式和模板
    void setStyle(std::unique_ptr<Style> style);
    void setTemplate(std::unique_ptr<Template> templateObj);
    
protected:
    // 属性变化通知
    virtual void onPropertyChanged(const QString& propertyName, const QVariant& oldValue, const QVariant& newValue);
};
```

## 3. 渲染系统实现

### 3.1 统一渲染上下文

```cpp
// presentation/ui/core/RenderContext.hpp
#pragma once
#include <QRect>
#include <QColor>
#include <QString>
#include <QFont>
#include <QImage>
#include <QTransform>
#include <stack>

struct Brush {
    QColor color;
    // 可扩展为渐变、纹理等
};

struct Pen {
    QColor color;
    float width = 1.0f;
    // 可扩展为虚线样式等
};

class IRenderContext {
public:
    virtual ~IRenderContext() = default;
    
    // === 基础绘制 ===
    virtual void drawRect(const QRect& rect, const Brush& brush) = 0;
    virtual void drawRectOutline(const QRect& rect, const Pen& pen) = 0;
    virtual void drawText(const QRect& rect, const QString& text, const QFont& font, const QColor& color) = 0;
    virtual void drawImage(const QRect& rect, const QImage& image) = 0;
    virtual void drawRoundedRect(const QRect& rect, float radius, const Brush& brush) = 0;
    
    // === 高级绘制 ===
    virtual void drawEllipse(const QRect& rect, const Brush& brush) = 0;
    virtual void drawLine(const QPoint& start, const QPoint& end, const Pen& pen) = 0;
    virtual void drawPath(const QPainterPath& path, const Brush& brush, const Pen& pen) = 0;
    
    // === 状态管理 ===
    virtual void pushClip(const QRect& clipRect) = 0;
    virtual void popClip() = 0;
    virtual void pushTransform(const QTransform& transform) = 0;
    virtual void popTransform() = 0;
    virtual void pushOpacity(float opacity) = 0;
    virtual void popOpacity() = 0;
    
    // === 度量 ===
    virtual QSize measureText(const QString& text, const QFont& font) = 0;
    virtual QRect textBounds(const QString& text, const QFont& font, const QRect& layoutRect) = 0;
};

// OpenGL实现
class OpenGLRenderContext : public IRenderContext {
private:
    std::stack<QRect> m_clipStack;
    std::stack<QTransform> m_transformStack;
    std::stack<float> m_opacityStack;
    
public:
    OpenGLRenderContext();
    virtual ~OpenGLRenderContext() = default;
    
    // IRenderContext实现
    void drawRect(const QRect& rect, const Brush& brush) override;
    void drawRectOutline(const QRect& rect, const Pen& pen) override;
    void drawText(const QRect& rect, const QString& text, const QFont& font, const QColor& color) override;
    void drawImage(const QRect& rect, const QImage& image) override;
    void drawRoundedRect(const QRect& rect, float radius, const Brush& brush) override;
    
    void drawEllipse(const QRect& rect, const Brush& brush) override;
    void drawLine(const QPoint& start, const QPoint& end, const Pen& pen) override;
    void drawPath(const QPainterPath& path, const Brush& brush, const Pen& pen) override;
    
    void pushClip(const QRect& clipRect) override;
    void popClip() override;
    void pushTransform(const QTransform& transform) override;
    void popTransform() override;
    void pushOpacity(float opacity) override;
    void popOpacity() override;
    
    QSize measureText(const QString& text, const QFont& font) override;
    QRect textBounds(const QString& text, const QFont& font, const QRect& layoutRect) override;
};
```

## 4. 控件封装示例：Button重构

### 4.1 Button主类

```cpp
// presentation/ui/controls/button/Button.hpp
#pragma once
#include "../primitives/ContentControl.hpp"
#include "ButtonRenderer.hpp"
#include "ButtonAnimations.hpp"
#include <QString>
#include <QFont>
#include <QColor>
#include <functional>

enum class ButtonState {
    Normal,
    Hovered,
    Pressed,
    Disabled
};

class Button : public ContentControl {
private:
    // 状态管理
    ButtonState m_state = ButtonState::Normal;
    bool m_isPressed = false;
    bool m_isHovered = false;
    
    // 样式属性
    QString m_text;
    QFont m_font;
    QColor m_foreground;
    QColor m_background;
    QColor m_hoverBackground;
    QColor m_pressedBackground;
    float m_cornerRadius = 4.0f;
    
    // 功能组件
    std::unique_ptr<ButtonRenderer> m_renderer;
    std::unique_ptr<ButtonAnimations> m_animations;
    
    // 事件处理
    std::function<void()> m_clickHandler;
    
protected:
    // FrameworkElement重写
    void onRender(IRenderContext& context) override;
    bool onInputEvent(const InputEvent& event) override;
    void onPropertyChange(const QString& propertyName, const QVariant& value) override;
    
    // 状态管理
    void updateState();
    void setState(ButtonState newState);
    
public:
    Button();
    explicit Button(const QString& text);
    virtual ~Button() = default;
    
    // === 文本属性 ===
    QString text() const { return m_text; }
    void setText(const QString& text);
    
    QFont font() const { return m_font; }
    void setFont(const QFont& font);
    
    // === 颜色属性 ===
    QColor foreground() const { return m_foreground; }
    void setForeground(const QColor& color);
    
    QColor background() const { return m_background; }
    void setBackground(const QColor& color);
    
    QColor hoverBackground() const { return m_hoverBackground; }
    void setHoverBackground(const QColor& color);
    
    QColor pressedBackground() const { return m_pressedBackground; }
    void setPressedBackground(const QColor& color);
    
    // === 外观属性 ===
    float cornerRadius() const { return m_cornerRadius; }
    void setCornerRadius(float radius);
    
    // === 状态查询 ===
    ButtonState state() const { return m_state; }
    bool isPressed() const { return m_isPressed; }
    bool isHovered() const { return m_isHovered; }
    
    // === 事件处理 ===
    void setClickHandler(std::function<void()> handler);
    
    // 信号（可选，如果需要Qt信号）
    Signal<> clicked;
    
protected:
    virtual void onClick();
    virtual void onStateChanged(ButtonState oldState, ButtonState newState);
};
```

### 4.2 Button渲染器

```cpp
// presentation/ui/controls/button/ButtonRenderer.hpp
#pragma once
#include "../../core/RenderContext.hpp"
#include <QRect>
#include <QColor>
#include <QString>
#include <QFont>

class Button; // 前向声明
enum class ButtonState;

class ButtonRenderer {
private:
    // 渲染辅助方法
    static void renderBackground(IRenderContext& context, const QRect& bounds, 
                                 const QColor& color, float cornerRadius);
    static void renderBorder(IRenderContext& context, const QRect& bounds, 
                             const QColor& borderColor, float borderWidth, float cornerRadius);
    static void renderText(IRenderContext& context, const QRect& bounds, 
                           const QString& text, const QFont& font, const QColor& color);
    
    // 状态相关颜色获取
    static QColor getBackgroundColor(const Button& button, ButtonState state);
    static QColor getForegroundColor(const Button& button, ButtonState state);
    
public:
    // 主要渲染方法
    static void render(const Button& button, IRenderContext& context);
    
    // 分层渲染方法
    static void renderButtonBackground(const Button& button, IRenderContext& context);
    static void renderButtonContent(const Button& button, IRenderContext& context);
    static void renderButtonOverlay(const Button& button, IRenderContext& context);
    
    // 度量方法
    static QSize measureButton(const Button& button, const SizeConstraints& constraints);
    static QRect getContentBounds(const Button& button);
};
```

### 4.3 Button动画系统

```cpp
// presentation/ui/controls/button/ButtonAnimations.hpp
#pragma once
#include "../../animation/Animation.hpp"
#include <QColor>

class Button;

class ButtonAnimations {
private:
    std::unique_ptr<ColorAnimation> m_backgroundAnimation;
    std::unique_ptr<ScaleAnimation> m_scaleAnimation;
    std::unique_ptr<OpacityAnimation> m_opacityAnimation;
    
public:
    ButtonAnimations(Button* owner);
    virtual ~ButtonAnimations() = default;
    
    // === 状态转换动画 ===
    void animateToNormal();
    void animateToHovered();
    void animateToPressed();
    void animateToDisabled();
    
    // === 点击动画 ===
    void playClickAnimation();
    void playRippleEffect(const QPoint& clickPosition);
    
    // === 控制方法 ===
    void stopAllAnimations();
    bool isAnimating() const;
    void setAnimationDuration(int milliseconds);
    void setEasingCurve(EasingCurve curve);
    
    // === 动画配置 ===
    struct AnimationConfig {
        int hoverDuration = 150;
        int pressDuration = 100;
        int clickDuration = 200;
        EasingCurve hoverEasing = EasingCurve::QuadOut;
        EasingCurve pressEasing = EasingCurve::QuadIn;
        EasingCurve clickEasing = EasingCurve::ElasticOut;
    };
    
    void setAnimationConfig(const AnimationConfig& config);
    const AnimationConfig& animationConfig() const;
    
private:
    Button* m_owner;
    AnimationConfig m_config;
    
    // 动画回调
    void onAnimationFinished(Animation* animation);
    void onColorAnimationUpdate(const QColor& color);
    void onScaleAnimationUpdate(float scale);
    void onOpacityAnimationUpdate(float opacity);
};
```

## 5. Window类集成实现

### 5.1 Window基础类

```cpp
// presentation/ui/window/Window.hpp
#pragma once
#include "../controls/primitives/ContentControl.hpp"
#include "WindowChrome.hpp"
#include "WindowControls.hpp"
#include <QString>
#include <QIcon>

enum class WindowState {
    Normal,
    Minimized,
    Maximized,
    Fullscreen
};

class Window : public ContentControl {
private:
    // 窗口状态
    WindowState m_state = WindowState::Normal;
    QString m_title;
    QIcon m_icon;
    bool m_resizable = true;
    bool m_showInTaskbar = true;
    
    // 窗口组件
    std::unique_ptr<WindowChrome> m_chrome;
    std::unique_ptr<WindowControls> m_controls;
    
    // 原生窗口句柄（平台相关）
    void* m_nativeHandle = nullptr;
    
protected:
    // FrameworkElement重写
    void onRender(IRenderContext& context) override;
    QSize onMeasure(const SizeConstraints& constraints) override;
    void onArrange(const QRect& finalRect) override;
    bool onInputEvent(const InputEvent& event) override;
    
    // 窗口特有事件
    virtual void onClosing();
    virtual void onStateChanged(WindowState oldState, WindowState newState);
    virtual void onTitleChanged(const QString& oldTitle, const QString& newTitle);
    
public:
    Window();
    virtual ~Window();
    
    // === 窗口管理 ===
    void show();
    void hide();
    void minimize();
    void maximize();
    void restore();
    void close();
    
    // === 窗口属性 ===
    QString title() const { return m_title; }
    void setTitle(const QString& title);
    
    QIcon icon() const { return m_icon; }
    void setIcon(const QIcon& icon);
    
    WindowState state() const { return m_state; }
    void setState(WindowState state);
    
    bool isResizable() const { return m_resizable; }
    void setResizable(bool resizable);
    
    bool showInTaskbar() const { return m_showInTaskbar; }
    void setShowInTaskbar(bool show);
    
    // === 窗口组件访问 ===
    WindowChrome* chrome() const { return m_chrome.get(); }
    WindowControls* controls() const { return m_controls.get(); }
    
    // === 事件信号 ===
    Signal<> closing;
    Signal<WindowState> stateChanged;
    Signal<QSize> sizeChanged;
    Signal<> activated;
    Signal<> deactivated;
    
private:
    void initializeWindow();
    void setupChrome();
    void setupControls();
    void updateNativeWindow();
};
```

### 5.2 主窗口实现

```cpp
// presentation/ui/window/MainWindow.hpp
#pragma once
#include "Window.hpp"
#include "../widgets/UiTopBar.hpp"
#include "../containers/UiRoot.hpp"

class ThemeManager;
class AppConfig;

class MainWindow : public Window {
private:
    // 应用组件
    std::unique_ptr<UiTopBar> m_topBar;
    std::unique_ptr<UiRoot> m_contentRoot;
    
    // 服务依赖
    std::shared_ptr<ThemeManager> m_themeManager;
    std::shared_ptr<AppConfig> m_config;
    
    // 布局配置
    static constexpr int TopBarHeight = 32;
    QRect m_topBarBounds;
    QRect m_contentBounds;
    
protected:
    // Window重写
    void onRender(IRenderContext& context) override;
    QSize onMeasure(const SizeConstraints& constraints) override;
    void onArrange(const QRect& finalRect) override;
    bool onInputEvent(const InputEvent& event) override;
    void onThemeChange(const Theme& theme) override;
    
public:
    MainWindow(std::shared_ptr<ThemeManager> themeManager,
               std::shared_ptr<AppConfig> config);
    virtual ~MainWindow() = default;
    
    // === 初始化 ===
    void initialize();
    void setupUI();
    void setupBindings();
    
    // === 组件访问 ===
    UiTopBar* topBar() const { return m_topBar.get(); }
    UiRoot* contentRoot() const { return m_contentRoot.get(); }
    
    // === 应用特有功能 ===
    void setupThemeToggle();
    void setupNavigation();
    void setupPageRouting();
    
private:
    void createTopBar();
    void createContentRoot();
    void layoutComponents();
    void connectSignals();
};
```

## 6. 实施步骤和最佳实践

### 6.1 第一阶段实施清单

1. **创建核心基础类**
   - [ ] 实现 `UIElement.hpp/.cpp`
   - [ ] 实现 `FrameworkElement.hpp/.cpp`
   - [ ] 实现 `RenderContext.hpp/.cpp`
   - [ ] 创建基础测试

2. **重构现有Button控件**
   - [ ] 拆分 `UiPushButton` 为多个文件
   - [ ] 实现 `Button.hpp/.cpp`
   - [ ] 实现 `ButtonRenderer.hpp/.cpp`
   - [ ] 实现 `ButtonAnimations.hpp/.cpp`
   - [ ] 迁移现有功能

3. **集成测试和验证**
   - [ ] 创建Button测试页面
   - [ ] 验证渲染正确性
   - [ ] 验证交互功能
   - [ ] 性能基准测试

### 6.2 最佳实践

1. **文件组织**
   - 每个类一个头文件和实现文件
   - 按功能分组到相应目录
   - 使用前向声明减少依赖

2. **接口设计**
   - 优先使用接口而非继承
   - 保持接口最小化
   - 使用RAII管理资源

3. **性能考虑**
   - 避免频繁的动态分配
   - 使用对象池缓存常用对象
   - 实现增量更新机制

4. **测试策略**
   - 每个组件都有单元测试
   - 使用模拟对象测试交互
   - 自动化UI测试关键流程

该实施指南为具体的重构工作提供了详细的代码结构和实现模板，确保重构工作的顺利进行。