# UI框架重构设计文档

## 1. 概述

本文档基于项目需求，提出了Fangjia Qt6 C++项目的UI框架重新设计方案。设计遵循单一职责原则、体系化架构以及MVVM模式，目标是构建一个高度封装、易于维护且性能优异的UI框架。

### 1.1 设计目标

1. **单一职责原则**：每个文件只负责一个UI元素或功能模块
2. **WPF风格继承体系**：构建清晰的UI元素继承关系
3. **增强封装性**：集渲染、交互、动画于一体的控件封装
4. **体系化设计**：整体性与关联性，无需向前兼容
5. **结构分层**：框架与业务分离，UI与数据分离，采用MVVM模式

### 1.2 现状分析

当前系统已有一个相对完善的基础：
- ✅ `IUiComponent` 接口定义了标准生命周期
- ✅ 基础容器类（`UiRoot`, `UiPage`, `UiPanel`等）
- ✅ 声明式UI系统（`RebuildHost`）
- ✅ 主题系统和渲染管线
- ❌ 文件职责划分不够清晰
- ❌ Window类未完全集成到UI体系
- ❌ 控件封装性有待增强

## 2. 新UI元素继承体系设计

### 2.1 核心继承层次结构（参考WPF）

```cpp
// 基础元素层次
UIElement                           // 最基础的UI元素（位置、大小、可见性）
├── FrameworkElement                // 框架级元素（布局、主题、数据绑定）
│   ├── Control                     // 控件基类（模板、样式、状态）
│   │   ├── ContentControl          // 内容控件（单一内容）
│   │   │   ├── Button             // 按钮控件
│   │   │   ├── Label              // 标签控件
│   │   │   └── Window             // 窗口控件 ⭐新增
│   │   ├── ItemsControl           // 项目控件（多项内容）
│   │   │   ├── ListBox            // 列表框
│   │   │   ├── TreeView           // 树形控件
│   │   │   └── TabView            // 标签页控件
│   │   └── RangeBase              // 范围控件
│   │       ├── ScrollBar          // 滚动条
│   │       └── Slider             // 滑块
│   └── Panel                      // 面板基类（布局容器）
│       ├── Canvas                 // 画布（绝对定位）
│       ├── StackPanel             // 栈面板（线性布局）
│       ├── Grid                   // 网格面板
│       ├── DockPanel              // 停靠面板
│       └── ScrollViewer           // 滚动查看器
└── Popup                          // 弹出层元素
```

### 2.2 核心接口体系

```cpp
// === 基础接口层 ===
class IUIElement {
    // 基础属性
    virtual QSize measureSize(const SizeConstraints& constraints) = 0;
    virtual void arrange(const QRect& finalRect) = 0;
    virtual void render(IRenderContext& context) = 0;
    virtual bool hitTest(const QPoint& point) = 0;
    
    // 基础事件
    virtual bool onInput(const InputEvent& event) = 0;
    virtual void onPropertyChanged(const PropertyChangeArgs& args) = 0;
};

class IFrameworkElement : public IUIElement {
    // 框架级功能
    virtual void applyTemplate() = 0;
    virtual void applyStyle() = 0;
    virtual void updateDataBinding() = 0;
    virtual void onThemeChanged(const Theme& theme) = 0;
};

class IControl : public IFrameworkElement {
    // 控件功能
    virtual void onStateChanged(ControlState oldState, ControlState newState) = 0;
    virtual void updateVisualState() = 0;
    virtual void invokeDefaultAction() = 0;
};

class IContentControl : public IControl {
    // 内容控件功能
    virtual void setContent(std::unique_ptr<IUIElement> content) = 0;
    virtual IUIElement* getContent() const = 0;
    virtual void onContentChanged() = 0;
};

class IPanel : public IFrameworkElement {
    // 面板容器功能
    virtual void addChild(std::unique_ptr<IUIElement> child) = 0;
    virtual void removeChild(IUIElement* child) = 0;
    virtual void clearChildren() = 0;
    virtual const std::vector<std::unique_ptr<IUIElement>>& getChildren() const = 0;
};
```

## 3. 文件结构重组设计

### 3.1 新的目录结构

```
presentation/ui/
├── core/                           # 核心基础设施
│   ├── UIElement.hpp              # 基础UI元素
│   ├── FrameworkElement.hpp       # 框架元素
│   ├── RenderContext.hpp          # 渲染上下文
│   ├── InputSystem.hpp            # 输入系统
│   ├── LayoutEngine.hpp           # 布局引擎
│   ├── PropertySystem.hpp         # 属性系统
│   └── ThemeSystem.hpp            # 主题系统
├── controls/                       # 控件实现
│   ├── primitives/                # 原语控件
│   │   ├── Control.hpp/.cpp
│   │   ├── ContentControl.hpp/.cpp
│   │   ├── ItemsControl.hpp/.cpp
│   │   └── RangeBase.hpp/.cpp
│   ├── button/                    # 按钮控件
│   │   ├── Button.hpp/.cpp
│   │   ├── ButtonRenderer.hpp/.cpp
│   │   ├── ButtonAnimations.hpp/.cpp
│   │   └── ButtonThemes.hpp/.cpp
│   ├── input/                     # 输入控件
│   │   ├── TextBox.hpp/.cpp
│   │   ├── ComboBox.hpp/.cpp
│   │   └── CheckBox.hpp/.cpp
│   ├── navigation/               # 导航控件
│   │   ├── TabView.hpp/.cpp
│   │   ├── NavigationView.hpp/.cpp
│   │   └── MenuBar.hpp/.cpp
│   └── data/                     # 数据控件
│       ├── ListBox.hpp/.cpp
│       ├── TreeView.hpp/.cpp
│       └── DataGrid.hpp/.cpp
├── panels/                        # 面板容器
│   ├── Panel.hpp/.cpp            # 面板基类
│   ├── Canvas.hpp/.cpp           # 画布面板
│   ├── StackPanel.hpp/.cpp       # 栈面板
│   ├── Grid.hpp/.cpp             # 网格面板
│   ├── DockPanel.hpp/.cpp        # 停靠面板
│   └── ScrollViewer.hpp/.cpp     # 滚动查看器
├── window/                        # 窗口系统
│   ├── Window.hpp/.cpp           # 窗口基类
│   ├── WindowChrome.hpp/.cpp     # 窗口装饰
│   ├── WindowControls.hpp/.cpp   # 窗口控制按钮
│   └── WindowManager.hpp/.cpp    # 窗口管理器
├── styling/                       # 样式系统
│   ├── Style.hpp/.cpp            # 样式定义
│   ├── Template.hpp/.cpp         # 控件模板
│   ├── Trigger.hpp/.cpp          # 触发器
│   └── Resources.hpp/.cpp        # 资源管理
├── animation/                     # 动画系统
│   ├── Animation.hpp/.cpp        # 动画基类
│   ├── Storyboard.hpp/.cpp       # 故事板
│   ├── Timeline.hpp/.cpp         # 时间线
│   └── Easing.hpp/.cpp           # 缓动函数
└── binding/                       # 数据绑定
    ├── Binding.hpp/.cpp          # 绑定基类
    ├── ObservableObject.hpp/.cpp # 可观察对象
    ├── Command.hpp/.cpp          # 命令系统
    └── Converter.hpp/.cpp        # 值转换器
```

### 3.2 单一职责原则实施

每个组件按功能拆分为独立文件：

#### 示例：Button控件拆分
```cpp
// controls/button/Button.hpp - 主要接口和基础实现
class Button : public ContentControl {
    // 核心按钮逻辑
};

// controls/button/ButtonRenderer.hpp - 渲染逻辑
class ButtonRenderer {
    static void render(const Button& button, IRenderContext& context);
};

// controls/button/ButtonAnimations.hpp - 动画逻辑
class ButtonAnimations {
    static void playPressAnimation(Button& button);
    static void playHoverAnimation(Button& button);
};

// controls/button/ButtonThemes.hpp - 主题样式
class ButtonThemes {
    static Style getDefaultLightTheme();
    static Style getDefaultDarkTheme();
};
```

## 4. Window类UI体系集成设计

### 4.1 Window继承关系

```cpp
// window/Window.hpp
class Window : public ContentControl {
private:
    // 窗口特有功能
    std::unique_ptr<WindowChrome> m_chrome;      // 窗口装饰
    std::unique_ptr<WindowControls> m_controls;  // 窗口控制按钮
    WindowState m_state = WindowState::Normal;   // 窗口状态
    
public:
    // 窗口管理
    void show();
    void hide();
    void minimize();
    void maximize();
    void restore();
    void close();
    
    // 窗口属性
    void setTitle(const QString& title);
    void setIcon(const QIcon& icon);
    void setResizable(bool resizable);
    
    // 窗口事件
    Signal<> closing;
    Signal<WindowState> stateChanged;
    Signal<QSize> sizeChanged;
    
    // ContentControl重写
    void setContent(std::unique_ptr<IUIElement> content) override;
    void render(IRenderContext& context) override;
    bool onInput(const InputEvent& event) override;
};

// window/MainWindow.hpp
class MainWindow : public Window {
private:
    std::unique_ptr<UiTopBar> m_topBar;
    std::unique_ptr<UiRoot> m_contentRoot;
    
public:
    MainWindow();
    void setupUI();
    void initializeLayout();
    
    // 应用特有功能
    void setupThemeToggle();
    void setupNavigation();
};
```

### 4.2 Window在UI树中的位置

```
Application
└── MainWindow (Window)                 // 主窗口
    ├── WindowChrome                    // 窗口装饰
    │   ├── TitleBar                   // 标题栏
    │   │   ├── WindowIcon             // 窗口图标
    │   │   ├── WindowTitle            // 窗口标题
    │   │   └── WindowControls         // 最小化/最大化/关闭
    │   └── ResizeBorder               // 调整边框
    └── Content (UiRoot)               // 内容区域
        ├── UiTopBar                   // 应用顶栏
        └── UiPage                     // 页面内容
            ├── NavigationPane
            └── ContentPane
```

## 5. 增强的UI控件封装设计

### 5.1 完整的控件生命周期

```cpp
class EnhancedControl : public Control {
private:
    // 渲染组件
    std::unique_ptr<IControlRenderer> m_renderer;
    
    // 动画组件
    std::unique_ptr<IAnimationManager> m_animations;
    
    // 交互组件
    std::unique_ptr<IInputHandler> m_inputHandler;
    
    // 样式组件
    std::unique_ptr<IStyleManager> m_styleManager;
    
public:
    // 完整的控件周期管理
    virtual void initialize();          // 初始化
    virtual void load();               // 加载资源
    virtual void updateState();       // 更新状态
    virtual void render();             // 渲染
    virtual void dispose();            // 释放资源
    
    // 集成的功能接口
    void playAnimation(const QString& animationName);
    void applyStyle(const Style& style);
    void handleInput(const InputEvent& event);
    void updateVisualState(ControlState state);
};
```

### 5.2 统一的渲染架构

```cpp
// core/RenderContext.hpp
class IRenderContext {
public:
    // 基础渲染
    virtual void drawRect(const QRect& rect, const Brush& brush) = 0;
    virtual void drawText(const QRect& rect, const QString& text, const Font& font) = 0;
    virtual void drawImage(const QRect& rect, const Image& image) = 0;
    
    // 高级渲染
    virtual void pushClip(const QRect& clipRect) = 0;
    virtual void popClip() = 0;
    virtual void pushTransform(const Transform& transform) = 0;
    virtual void popTransform() = 0;
    virtual void pushOpacity(float opacity) = 0;
    virtual void popOpacity() = 0;
};

// 统一的控件渲染器基类
class IControlRenderer {
public:
    virtual void render(const Control& control, IRenderContext& context) = 0;
    virtual void renderBackground(const Control& control, IRenderContext& context) = 0;
    virtual void renderForeground(const Control& control, IRenderContext& context) = 0;
    virtual void renderBorder(const Control& control, IRenderContext& context) = 0;
};
```

## 6. MVVM架构分层设计

### 6.1 架构分层

```
┌─────────────────────────────────────────────┐
│                   View Layer                 │  UI层：UI控件、布局、样式
├─────────────────────────────────────────────┤
│                ViewModel Layer               │  视图模型层：UI逻辑、命令、数据绑定
├─────────────────────────────────────────────┤
│                  Model Layer                 │  模型层：业务逻辑、数据访问
└─────────────────────────────────────────────┘
```

### 6.2 数据绑定系统

```cpp
// binding/ObservableObject.hpp
class ObservableObject {
protected:
    template<typename T>
    void setProperty(T& field, const T& value, const QString& propertyName);
    
public:
    Signal<QString, QVariant> propertyChanged;
};

// binding/Binding.hpp
template<typename TSource, typename TTarget>
class Binding {
private:
    ObservableObject* m_source;
    QString m_sourcePath;
    std::function<TTarget(const TSource&)> m_converter;
    
public:
    Binding(ObservableObject* source, const QString& sourcePath);
    void setConverter(std::function<TTarget(const TSource&)> converter);
    void bind(std::function<void(const TTarget&)> setter);
};

// 使用示例
class MainViewModel : public ObservableObject {
private:
    bool m_isDarkTheme = false;
    
public:
    bool isDarkTheme() const { return m_isDarkTheme; }
    void setIsDarkTheme(bool value) { 
        setProperty(m_isDarkTheme, value, "isDarkTheme"); 
    }
    
    Command toggleThemeCommand{[this]() { 
        setIsDarkTheme(!m_isDarkTheme); 
    }};
};
```

### 6.3 命令系统

```cpp
// binding/Command.hpp
class Command {
private:
    std::function<void()> m_execute;
    std::function<bool()> m_canExecute;
    
public:
    Command(std::function<void()> execute, 
            std::function<bool()> canExecute = nullptr);
    
    void execute();
    bool canExecute() const;
    
    Signal<> canExecuteChanged;
};

// 控件中的命令绑定
class Button : public ContentControl {
private:
    Command* m_command = nullptr;
    
public:
    void setCommand(Command* command);
    
protected:
    void onClick() override {
        if (m_command && m_command->canExecute()) {
            m_command->execute();
        }
    }
};
```

## 7. 实现迁移策略

### 7.1 阶段性迁移计划

#### 阶段1：核心基础设施（Week 1-2）
- [ ] 实现新的UIElement基础类
- [ ] 创建RenderContext统一渲染接口
- [ ] 实现基础的属性系统和事件系统
- [ ] 迁移现有的IUiComponent到新架构

#### 阶段2：控件体系重构（Week 3-4）
- [ ] 重构Button控件，拆分为多个职责文件
- [ ] 重构Panel系列容器
- [ ] 实现新的Window类集成
- [ ] 迁移现有控件到新的继承体系

#### 阶段3：高级功能集成（Week 5-6）
- [ ] 实现完整的数据绑定系统
- [ ] 集成动画系统
- [ ] 完善样式和主题系统
- [ ] 实现命令系统

#### 阶段4：完整性测试和优化（Week 7-8）
- [ ] 性能优化和内存管理
- [ ] 完整性测试和bug修复
- [ ] 文档更新和示例代码
- [ ] 向后兼容性处理

### 7.2 兼容性策略

虽然要求"无需考虑向前兼容"，但为了平滑过渡，提供适配器模式：

```cpp
// 适配器：将旧的IUiComponent适配到新架构
class LegacyComponentAdapter : public UIElement {
private:
    std::unique_ptr<IUiComponent> m_legacyComponent;
    
public:
    LegacyComponentAdapter(std::unique_ptr<IUiComponent> component);
    
    // 将新接口调用转换为旧接口调用
    void render(IRenderContext& context) override;
    bool onInput(const InputEvent& event) override;
    QSize measureSize(const SizeConstraints& constraints) override;
};
```

## 8. 性能和质量保证

### 8.1 性能优化策略

- **渲染优化**：脏区域跟踪、批量渲染、GPU加速
- **内存管理**：对象池、延迟加载、智能指针
- **布局优化**：布局缓存、增量布局更新
- **事件优化**：事件路由优化、命中测试加速

### 8.2 代码质量保证

- **单元测试**：每个控件都有对应的测试用例
- **集成测试**：完整的UI交互流程测试
- **性能测试**：渲染性能和内存使用监控
- **代码审查**：确保架构一致性和代码质量

## 9. 总结

本设计文档提出了一个全面的UI框架重构方案，主要特点：

1. **清晰的继承体系**：参考WPF构建了完整的UI元素继承关系
2. **单一职责原则**：每个文件只负责一个具体功能
3. **增强的封装性**：控件集成了渲染、交互、动画等完整功能
4. **完整的MVVM支持**：数据绑定、命令系统、视图模型分离
5. **Window系统集成**：Window作为UI体系的一部分完整集成
6. **平滑迁移策略**：阶段性实施，最小化对现有代码的影响

该设计为后续的重构实施提供了清晰的蓝图和实现指南。