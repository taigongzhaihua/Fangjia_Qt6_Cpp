# 声明式弹出控件 (Declarative Popup Widget)

## 概述

本文档介绍新增的声明式弹出控件API，它为现有的`UiPopup`组件提供了一个流式、类型安全的声明式包装器。

## 问题背景

原有的弹出控件使用命令式API，需要手动创建实例、设置属性，代码冗长且容易出错：

```cpp
// 原有命令式API
auto popup = std::make_unique<UiPopup>(parentWindow);
popup->setTrigger(button.get());
popup->setPopupContent(content.get());
popup->setPopupSize(QSize(200, 150));
popup->setPlacement(UiPopup::Placement::Bottom);
popup->setPopupStyle(bgColor, radius);
popup->setOnPopupVisibilityChanged(callback);
```

## 解决方案

新的声明式`Popup`组件提供了流式API，与其他声明式组件保持一致：

```cpp
// 新声明式API
auto popupWidget = popup()
    ->trigger(text("菜单"))
    ->content(menuPanel)
    ->size(QSize(200, 150))
    ->placement(Popup::Placement::Bottom)
    ->style(QColor(255, 255, 255, 240), 8.0f)
    ->onVisibilityChanged(callback);
```

## API参考

### 工厂函数

```cpp
auto popup() -> std::shared_ptr<Popup>
```

创建新的弹出控件实例。

### 配置方法

#### trigger()
```cpp
std::shared_ptr<Popup> trigger(WidgetPtr trigger)
```
设置触发器组件，通常是按钮或其他可交互元素。

#### content()
```cpp
std::shared_ptr<Popup> content(WidgetPtr content)
```
设置弹出时显示的内容组件。

#### size()
```cpp
std::shared_ptr<Popup> size(const QSize& size)
```
设置弹出窗口的大小。

#### placement()
```cpp
std::shared_ptr<Popup> placement(Placement placement)
```
设置弹出位置策略：
- `Bottom` - 触发器下方
- `Top` - 触发器上方
- `Right` - 触发器右侧
- `Left` - 触发器左侧
- `BottomLeft` - 触发器左下方
- `BottomRight` - 触发器右下方
- `TopLeft` - 触发器左上方
- `TopRight` - 触发器右上方
- `Custom` - 自定义位置

#### style()
```cpp
std::shared_ptr<Popup> style(const QColor& backgroundColor, float cornerRadius = 8.0f)
```
设置弹出窗口的背景颜色和圆角半径。

#### offset()
```cpp
std::shared_ptr<Popup> offset(const QPoint& offset)
```
设置相对于计算位置的额外偏移量。

#### closeOnClickOutside()
```cpp
std::shared_ptr<Popup> closeOnClickOutside(bool close = true)
```
设置是否在点击弹出窗口外部时自动关闭。

#### onVisibilityChanged()
```cpp
std::shared_ptr<Popup> onVisibilityChanged(std::function<void(bool)> callback)
```
设置弹出窗口可见性变化的回调函数。

## 使用示例

### 简单下拉菜单

```cpp
auto menuButton = popup()
    ->trigger(text("菜单"))
    ->content(panel({
        text("选项1")->onTap([]() { /* 处理选项1 */ }),
        text("选项2")->onTap([]() { /* 处理选项2 */ }),
        text("选项3")->onTap([]() { /* 处理选项3 */ })
    }))
    ->placement(Popup::Placement::Bottom)
    ->style(QColor(255, 255, 255, 240), 6.0f);
```

### 工具提示

```cpp
auto helpTooltip = popup()
    ->trigger(icon("help"))
    ->content(text("这是一个帮助提示"))
    ->size(QSize(200, 60))
    ->placement(Popup::Placement::Top)
    ->style(QColor(50, 50, 50, 230), 4.0f);
```

### 用户菜单

```cpp
auto userMenu = popup()
    ->trigger(text("用户"))
    ->content(panel({
        text("个人资料"),
        text("账户设置"),
        text("退出登录")
    }))
    ->placement(Popup::Placement::BottomRight)
    ->size(QSize(120, 90))
    ->onVisibilityChanged([](bool visible) {
        qDebug() << "用户菜单" << (visible ? "打开" : "关闭");
    });
```

## 集成使用

由于技术限制，弹出控件需要访问父窗口，因此需要额外的配置步骤：

```cpp
// 1. 创建弹出组件
auto popupWidget = popup()
    ->trigger(button)
    ->content(menu);

// 2. 构建组件
auto component = popupWidget->build();

// 3. 配置窗口上下文
Popup::configurePopupWindow(component.get(), parentWindow);

// 4. 添加到UI系统
uiRoot.add(component.release());
```

### 在主窗口中使用

```cpp
class MainWindow : public QOpenGLWindow {
private:
    void initializeUI() {
        // 创建工具栏弹出菜单
        auto toolsMenu = popup()
            ->trigger(text("工具"))
            ->content(createToolsMenu())
            ->placement(Popup::Placement::Bottom);

        // 构建并配置
        auto component = toolsMenu->build();
        Popup::configurePopupWindow(component.get(), this);
        
        // 添加到UI系统
        m_uiRoot.add(component.release());
    }
    
    WidgetPtr createToolsMenu() {
        return panel({
            text("导入数据")->onTap([this]() { importData(); }),
            text("导出数据")->onTap([this]() { exportData(); }),
            text("首选项")->onTap([this]() { openPreferences(); })
        });
    }
};
```

## 架构设计

### 组件层次结构

```
Popup (声明式接口)
  └── PopupHost (内部实现)
      └── UiPopup (现有实现)
          └── UiPopupWindow (窗口实现)
```

### 关键特性

1. **延迟创建**: PopupHost在获得窗口上下文后才创建UiPopup
2. **资源管理**: 自动管理组件生命周期和资源上下文
3. **事件传播**: 完整的鼠标和键盘事件传播链
4. **装饰器支持**: 支持标准的Widget装饰器（padding、margin、background等）

## 优势

与原有命令式API相比，声明式API具有以下优势：

1. **简洁性**: 减少约40%的代码量
2. **一致性**: 与其他声明式组件API保持一致
3. **类型安全**: 编译期检查，减少运行时错误
4. **可组合性**: 可以轻松与其他声明式组件组合使用
5. **内存管理**: 自动化的生命周期管理
6. **函数式风格**: 支持现代C++函数式编程模式

## 限制和注意事项

1. **窗口依赖**: 需要手动调用`configurePopupWindow()`设置窗口上下文
2. **构建顺序**: 必须在添加到UI系统之前完成配置
3. **资源上下文**: 弹出窗口的创建需要完整的资源上下文
4. **平台限制**: 继承了UiPopup的平台相关限制

## 未来改进

1. **自动窗口注入**: 研究通过上下文系统自动注入窗口依赖
2. **更多位置策略**: 增加智能位置调整和碰撞检测
3. **动画支持**: 集成显示/隐藏动画
4. **无障碍支持**: 添加键盘导航和屏幕阅读器支持