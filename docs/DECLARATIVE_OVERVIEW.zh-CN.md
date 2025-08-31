# 声明式 Widget 体系概览（中文文档）

本文档全面介绍声明式 UI 体系，包括 Widget 基类、装饰器系统、ComponentWrapper、BindingHost 自动重建机制，以及丰富的 UI 工厂函数。

## 声明式 Widget 体系概述

声明式 Widget 体系位于 `presentation/ui/declarative/` 目录，提供现代化的链式 API 来构建 UI，同时与底层的 IUiComponent 运行时组件保持完全兼容。

### 核心组成

- **Widget**: 基础装饰器类，提供 padding、margin、background 等通用装饰
- **DecoratedBox**: 专用装饰容器，支持主题化 hover/press 背景
- **ComponentWrapper**: 将运行时 IUiComponent 包装为声明式组件
- **BindingHost**: 响应式重建宿主，支持 observe 和自动重建
- **UI 工厂函数**: 便利的 UI 创建函数（text、icon、container、card 等）

## Widget 基类与装饰器

### Widget 基础装饰器

`Widget`（位于 `presentation/ui/declarative/Widget.h`）是所有声明式组件的基类：

```cpp
class Widget : public IUiComponent {
public:
    // 内边距装饰器
    std::shared_ptr<Widget> padding(float all);
    std::shared_ptr<Widget> padding(float vertical, float horizontal);
    std::shared_ptr<Widget> padding(float top, float right, float bottom, float left);
    
    // 外边距装饰器
    std::shared_ptr<Widget> margin(float all);
    std::shared_ptr<Widget> margin(float vertical, float horizontal);
    std::shared_ptr<Widget> margin(float top, float right, float bottom, float left);
    
    // 背景装饰器
    std::shared_ptr<Widget> background(const QColor& color);
    std::shared_ptr<Widget> backgroundGradient(const QLinearGradient& gradient);
    
    // 边框装饰器
    std::shared_ptr<Widget> border(const QColor& color, float width);
    std::shared_ptr<Widget> borderRadius(float radius);
    
    // 透明度装饰器
    std::shared_ptr<Widget> opacity(float alpha);
    
    // 交互装饰器
    std::shared_ptr<Widget> onTap(std::function<void()> callback);
    std::shared_ptr<Widget> onHover(std::function<void(bool)> callback);
};
```

### 装饰器应用示例

```cpp
// 创建一个带装饰的按钮
auto decoratedButton = UI::text("Click Me")
    ->fontSize(16)
    ->fontWeight(QFont::Bold)
    ->themeColor(QColor(255, 255, 255), QColor(0, 0, 0))
    ->padding(12, 24)                    // 内边距
    ->margin(8)                          // 外边距
    ->background(QColor(0, 120, 255))    // 蓝色背景
    ->borderRadius(8.0f)                 // 圆角
    ->onTap([]() {                       // 点击回调
        qDebug() << "Button clicked!";
    })
    ->onHover([](bool hovered) {         // 悬停回调
        qDebug() << "Hovered:" << hovered;
    });
```

### 装饰器渲染顺序

Widget 装饰器按以下顺序渲染：

1. **外边距**: 计算容器布局位置
2. **背景**: 绘制背景色或渐变
3. **边框**: 绘制边框（在背景之上）
4. **内边距**: 为子内容预留空间
5. **子内容**: 渲染实际的子组件
6. **透明度**: 最后应用整体透明度

```cpp
void Widget::append(Render::FrameData& fd) const {
    // 1. 应用外边距后的矩形
    QRectF contentRect = applyMargin(m_bounds);
    
    // 2. 绘制背景
    if (m_backgroundColor.alpha() > 0) {
        fd.addRoundedRect(contentRect, m_borderRadius, m_backgroundColor);
    }
    
    // 3. 绘制边框
    if (m_borderWidth > 0 && m_borderColor.alpha() > 0) {
        fd.addRoundedRectBorder(contentRect, m_borderRadius, m_borderColor, m_borderWidth);
    }
    
    // 4. 渲染子内容（在内边距区域内）
    QRectF childRect = applyPadding(contentRect);
    if (m_child) {
        m_child->append(fd);
    }
    
    // 5. 应用透明度
    if (m_opacity < 1.0f) {
        fd.applyOpacity(m_opacity);
    }
}
```

## DecoratedBox：通用装饰容器

`DecoratedBox` 提供更高级的装饰功能，特别是主题化的交互背景：

```cpp
class DecoratedBox : public Widget {
public:
    // 主题化背景
    std::shared_ptr<DecoratedBox> themeBackground(
        const QColor& lightColor, 
        const QColor& darkColor
    );
    
    // 交互状态背景
    std::shared_ptr<DecoratedBox> hoverBackground(const QColor& color);
    std::shared_ptr<DecoratedBox> pressedBackground(const QColor& color);
    
    // 圆角和阴影
    std::shared_ptr<DecoratedBox> cornerRadius(float radius);
    std::shared_ptr<DecoratedBox> shadow(const QColor& color, float blur, QPointF offset);
};
```

### 交互状态管理

```cpp
// 卡片式容器，支持悬停和按下效果
auto card = UI::decoratedBox()
    ->themeBackground(QColor(250, 250, 250), QColor(40, 40, 40))
    ->hoverBackground(QColor(240, 240, 240))
    ->pressedBackground(QColor(230, 230, 230))
    ->cornerRadius(12.0f)
    ->shadow(QColor(0, 0, 0, 30), 4.0f, QPointF(0, 2))
    ->padding(16)
    ->child(UI::text("Card Content"));
```

### 状态动画

DecoratedBox 支持平滑的状态过渡动画：

```cpp
bool DecoratedBox::tick() {
    bool needsRedraw = false;
    
    // 背景颜色动画
    if (m_currentBgColor != m_targetBgColor) {
        constexpr float ANIMATION_SPEED = 0.1f;
        m_currentBgColor = lerpColor(m_currentBgColor, m_targetBgColor, ANIMATION_SPEED);
        needsRedraw = true;
    }
    
    // 传播到子组件
    if (m_child && m_child->tick()) {
        needsRedraw = true;
    }
    
    return needsRedraw;
}
```

## ComponentWrapper：运行时组件桥接

`ComponentWrapper`（位于 `presentation/ui/declarative/ComponentWrapper.h`）将现有的 IUiComponent 包装为声明式组件：

### 基本包装

```cpp
// 包装现有的运行时组件
auto navRail = std::make_unique<Ui::NavRail>();
auto wrappedNav = UI::wrap(std::move(navRail));

// 可以应用装饰器
auto decoratedNav = UI::wrap(&existingNavRail)
    ->padding(8)
    ->background(QColor(0, 0, 0, 20));
```

### 生命周期转发

ComponentWrapper 确保所有生命周期事件正确转发：

```cpp
class ComponentWrapper : public Widget {
public:
    void updateLayout(const QSize& winSize) override {
        // 先处理自身装饰器布局
        Widget::updateLayout(winSize);
        
        // 转发到包装的组件
        if (m_wrappedComponent) {
            QRectF childRect = calculateChildRect();
            m_wrappedComponent->updateLayout(childRect.size().toSize());
        }
    }
    
    void onThemeChanged(bool isDark) override {
        // 转发主题事件
        if (m_wrappedComponent) {
            m_wrappedComponent->onThemeChanged(isDark);
        }
        
        // 处理装饰器主题
        Widget::onThemeChanged(isDark);
    }
    
    bool onMousePress(const QPointF& localPos) override {
        // 坐标转换后转发鼠标事件
        QPointF childPos = transformToChild(localPos);
        return m_wrappedComponent && m_wrappedComponent->onMousePress(childPos);
    }
};
```

### 所有权管理

ComponentWrapper 支持两种所有权模式：

```cpp
// 模式1：转移所有权
auto wrapper1 = UI::wrap(std::move(component)); // ComponentWrapper 拥有组件

// 模式2：引用模式
auto wrapper2 = UI::wrap(&component);           // 原始指针，不转移所有权
```

## BindingHost：响应式重建机制

`BindingHost`（位于 `presentation/ui/declarative/Binding.h`）提供响应式 UI 重建功能：

### 基本用法

```cpp
// 创建响应式 UI
auto host = UI::bindingHost([this]() -> WidgetPtr {
    return UI::panel()
        ->direction(UI::Panel::Direction::Vertical)
        ->children({
            UI::text(m_viewModel->title()),
            UI::text(m_viewModel->description()),
            UI::button("Action")
                ->onTap([this]() { m_viewModel->performAction(); })
        });
});

// 绑定数据变化自动重建
host->observe(m_viewModel, &ViewModel::titleChanged, [this]() {
    m_host->requestRebuild();
});
```

### observe 自动重建

```cpp
class BindingHost : public RebuildHost {
public:
    // 绑定信号，变化时自动重建
    template<typename Object, typename Signal>
    void observe(Object* object, Signal signal, std::function<void()> callback = nullptr) {
        QObject::connect(object, signal, [this, callback]() {
            if (callback) callback();
            requestRebuild();
        });
    }
    
    // 绑定属性变化
    template<typename Object>
    void observeProperty(Object* object, const char* propertyName) {
        // 使用 QPropertyNotifier 或信号槽机制监听属性变化
        // ...
    }
};
```

### RebuildHost 的重建顺序

继承自 `RebuildHost` 的重建机制确保正确的更新顺序：

```cpp
void RebuildHost::requestRebuild() {
    if (!m_builder) return;
    
    // 重新构建子树
    m_child = m_builder();
    
    if (m_child) {
        // 1. 设置视口
        if (m_hasViewport && dynamic_cast<IUiContent*>(m_child.get())) {
            dynamic_cast<IUiContent*>(m_child.get())->setViewportRect(m_viewport);
        }
        
        // 2. 应用主题（在资源上下文之前）
        if (m_hasTheme) {
            m_child->onThemeChanged(m_isDark);
        }
        
        // 3. 更新资源上下文
        if (m_hasCtx) {
            m_child->updateResourceContext(*m_cache, m_gl, m_dpr);
        }
        
        // 4. 更新布局
        if (m_hasWinSize) {
            m_child->updateLayout(m_winSize);
        }
    }
}
```

## Text/Icon 的主题适配与缓存键

### Text 组件主题适配

Text 组件（位于 `presentation/ui/declarative/BasicWidgets.h`）支持主题色和缓存优化：

```cpp
class Text : public Widget {
public:
    // 主题颜色
    std::shared_ptr<Text> themeColor(const QColor& lightColor, const QColor& darkColor);
    
    // 字体设置
    std::shared_ptr<Text> fontSize(int px);
    std::shared_ptr<Text> fontWeight(QFont::Weight weight);
    std::shared_ptr<Text> fontFamily(const QString& family);
    
    // 对齐方式
    std::shared_ptr<Text> alignment(Qt::Alignment align);
};
```

### 文本缓存键生成

```cpp
QString Text::generateCacheKey() const {
    QColor actualColor = m_isDark ? m_darkColor : m_lightColor;
    
    // 使用 RenderUtils 生成包含所有渲染参数的缓存键
    return RenderUtils::makeTextCacheKey(
        m_text,           // 文本内容
        m_fontSize,       // 字体大小
        actualColor       // 当前主题的颜色
    );
}
```

`RenderUtils::makeTextCacheKey` 实现（位于 `presentation/ui/base/RenderUtils.hpp`）：

```cpp
namespace RenderUtils {
    QString makeTextCacheKey(const QString& text, int fontSize, const QColor& color) {
        // 包含颜色 RGBA 值确保主题变化时缓存失效
        return QString("%1__%2__%3").arg(
            text,
            QString::number(fontSize),
            color.name(QColor::HexArgb)  // 包含 alpha 通道
        );
    }
}
```

### Icon 组件主题适配

Icon 组件支持主题路径和主题色：

```cpp
class Icon : public Widget {
public:
    // 主题路径：不同主题使用不同的 SVG 文件
    std::shared_ptr<Icon> themePaths(const QString& lightPath, const QString& darkPath);
    
    // 主题色：对单色图标应用主题颜色
    std::shared_ptr<Icon> themeColor(const QColor& lightColor, const QColor& darkColor);
    
    // 尺寸设置
    std::shared_ptr<Icon> size(float width, float height);
    std::shared_ptr<Icon> size(float square);
};
```

### 图标缓存键生成

```cpp
QString Icon::generateCacheKey() const {
    QString path = m_isDark ? m_darkPath : m_lightPath;
    QColor color = m_isDark ? m_darkColor : m_lightColor;
    
    // 使用 RenderUtils 生成图标缓存键
    return RenderUtils::makeIconCacheKey(
        path,                                           // 图标路径
        static_cast<int>(m_size.width()),               // 像素尺寸
        m_isDark ? QStringLiteral("dark") : QStringLiteral("light")  // 主题标识
    );
}
```

`RenderUtils::makeIconCacheKey` 实现：

```cpp
namespace RenderUtils {
    QString makeIconCacheKey(const QString& basePath, int pixelSize, const QString& theme) {
        // 格式：路径__尺寸__主题
        return QString("%1__%2__%3").arg(basePath, QString::number(pixelSize), theme);
    }
}
```

## UI 工厂函数

UI 命名空间（位于 `presentation/ui/declarative/UI.h`）提供丰富的工厂函数：

### 基础组件工厂

```cpp
namespace UI {
    // 文本组件
    std::shared_ptr<Text> text(const QString& content);
    
    // 图标组件
    std::shared_ptr<Icon> icon(const QString& path);
    
    // 容器组件
    std::shared_ptr<DecoratedBox> container();
    std::shared_ptr<DecoratedBox> card();
    
    // 按钮组件
    std::shared_ptr<Button> button(const QString& text);
    
    // 输入组件
    std::shared_ptr<TextInput> textInput(const QString& placeholder = "");
}
```

### 布局组件工厂

```cpp
namespace UI {
    // Panel 布局
    std::shared_ptr<Panel> panel();
    
    // Grid 布局
    std::shared_ptr<Grid> grid();
    
    // ScrollView
    std::shared_ptr<ScrollView> scrollView(WidgetPtr child);
    
    // TabView
    std::shared_ptr<TabView> tabView();
}
```

### 复合组件工厂

```cpp
namespace UI {
    // AppShell：应用程序外壳
    std::shared_ptr<AppShell> appShell();
    
    // NavRail：导航栏
    std::shared_ptr<NavRail> navRail();
    
    // TopBar：顶部栏
    std::shared_ptr<TopBar> topBar();
    
    // 条件渲染
    WidgetPtr when(bool condition, std::function<WidgetPtr()> builder);
}
```

### 工厂函数使用示例

```cpp
// 使用工厂函数构建复杂 UI
auto app = UI::appShell()
    ->topBar(UI::topBar()
        ->followSystem(true)
        ->onThemeToggle([this]() { toggleTheme(); })
    )
    ->nav(UI::navRail()
        ->widths(48, 200)
        ->iconSize(24)
    )
    ->content(UI::scrollView(
        UI::panel()
            ->direction(UI::Panel::Direction::Vertical)
            ->spacing(16)
            ->padding(24)
            ->children({
                UI::text("Welcome")
                    ->fontSize(32)
                    ->fontWeight(QFont::Bold),
                
                UI::card()
                    ->padding(16)
                    ->child(UI::text("Card content")),
                
                UI::when(showButton, []() {
                    return UI::button("Conditional Button");
                })
            })
    ));
```

## 与运行时组件对齐

### 声明式 TopBar 与 UiTopBar 一致性

声明式 TopBar 确保与运行时 `UiTopBar` 行为完全一致：

- **动画时长**: 使用相同的 `scaleDuration` (2/3 缩放)
- **交互阈值**: 相同的 `themeInteractive()` 逻辑
- **状态管理**: 相同的 `AnimPhase` 枚举和状态机
- **回调接口**: 一致的事件处理方式

### 声明式 NavRail 与 Ui::NavRail 一致性

声明式 NavRail 保持与运行时组件的一致性：

- **布局计算**: 相同的展开/收缩宽度（默认 48/200px）
- **动画行为**: 相同的展开动画时长和缓动曲线
- **图标缓存**: 使用相同的缓存键生成逻辑
- **数据绑定**: 相同的 `INavDataProvider` 接口

```cpp
// 声明式组件内部包装运行时组件
class TopBarComponent : public IUiComponent {
private:
    std::unique_ptr<UiTopBar> m_runtimeTopBar;  // 包装的运行时组件
    
public:
    void updateLayout(const QSize& winSize) override {
        m_runtimeTopBar->updateLayout(winSize);
    }
    
    void onThemeChanged(bool isDark) override {
        m_runtimeTopBar->onThemeChanged(isDark);
    }
    
    bool tick() override {
        return m_runtimeTopBar->tick();
    }
};
```

## 性能优化与最佳实践

### 缓存策略

- **文本缓存**: 基于内容、字体、颜色的完整缓存键
- **图标缓存**: 基于路径、尺寸、主题的缓存键
- **布局缓存**: 容器级别的布局结果缓存

### 重建优化

- **增量重建**: 只重建变化的子树部分
- **条件渲染**: 使用 `UI::when` 避免不必要的组件创建
- **延迟加载**: 大型列表使用虚拟化技术

### 内存管理

- **智能指针**: 使用 `std::shared_ptr` 管理组件生命周期
- **弱引用**: 避免循环引用导致的内存泄漏
- **资源复用**: 相同配置的组件复用缓存资源

## 相关文档

- [UI 架构](./UI_ARCHITECTURE.zh-CN.md) - IUiComponent 生命周期和主题传播机制
- [声明式 TopBar](./DECLARATIVE_NAV_TOPBAR.zh-CN.md) - TopBar 和 NavRail 的具体使用方法
- [布局系统](./LAYOUTS.zh-CN.md) - 声明式布局容器详解
- [滚动容器](./SCROLL_VIEW.zh-CN.md) - ScrollView 的声明式封装