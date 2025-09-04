# 弹出控件 (Popup Control)

## 概述

弹出控件是一个基于QOpenGLWindow的UI组件，能够在父窗口边界之外绘制内容，同时保持与父窗口的关联关系。这个组件特别适用于实现下拉菜单、工具提示、上下文菜单等需要超出主窗口边界的UI元素。

## 核心特性

- **超出窗口绘制**：可以在父窗口边界外显示内容
- **父子关系**：作为子窗口存在，保持与父窗口的关联
- **集成UI框架**：完全集成到现有的OpenGL UI框架中
- **事件处理**：支持完整的鼠标、键盘和滚轮事件
- **主题支持**：支持明/暗主题切换
- **动画支持**：内置动画系统支持

## 架构设计

### 组件层次结构

```
UiPopup (UI组件接口适配器)
    ├── UiPopupWindow (独立的QOpenGLWindow)
    └── Trigger Component (触发器组件)
```

### 核心类

1. **UiPopupWindow** - 基于QOpenGLWindow的弹出窗口
2. **UiPopup** - 集成到UI框架的组件适配器

## 使用方法

### 基本用法

```cpp
// 创建弹出控件
auto popup = std::make_unique<UiPopup>(parentWindow);

// 设置触发器（可选）
popup->setTrigger(triggerButton);

// 设置弹出内容
popup->setPopupContent(menuContent);

// 配置弹出样式
popup->setPopupSize(QSize(200, 150));
popup->setPlacement(UiPopup::Placement::Bottom);
popup->setPopupStyle(QColor(255, 255, 255, 240), 8.0f);

// 添加到UI根容器
uiRoot.add(popup.get());
```

### 程序控制显示/隐藏

```cpp
// 显示弹出窗口
popup->showPopup();

// 隐藏弹出窗口
popup->hidePopup();

// 检查可见性
bool visible = popup->isPopupVisible();
```

### 位置策略

支持多种预定义的位置策略：

- `Bottom` - 在触发器下方
- `Top` - 在触发器上方
- `Right` - 在触发器右侧
- `Left` - 在触发器左侧
- `BottomLeft` - 在触发器左下方
- `BottomRight` - 在触发器右下方
- `TopLeft` - 在触发器左上方
- `TopRight` - 在触发器右上方
- `Custom` - 自定义位置

### 事件回调

```cpp
// 设置可见性变化回调
popup->setOnPopupVisibilityChanged([](bool visible) {
    qDebug() << "弹出窗口可见性:" << visible;
});
```

## 技术实现

### 超出窗口边界的实现

使用QOpenGLWindow的子窗口机制：

1. **子窗口关系**：弹出窗口作为主窗口的子窗口
2. **窗口标志**：使用`Qt::Tool | Qt::FramelessWindowHint | Qt::WindowStaysOnTopHint`
3. **位置计算**：将主窗口坐标转换为全局屏幕坐标
4. **边界检测**：自动调整位置确保在屏幕可见区域内

### OpenGL渲染集成

- **共享上下文**：与主窗口共享OpenGL上下文和资源
- **渲染管线**：使用相同的Renderer和FrameData系统
- **资源管理**：共享IconCache和字体资源

### 事件处理

- **焦点管理**：ESC键自动关闭弹出窗口
- **点击外部**：可配置点击外部区域关闭弹出窗口
- **事件转发**：在触发器和弹出内容之间正确转发事件

## 示例代码

### 简单的弹出菜单

```cpp
class MenuPopupDemo : public QOpenGLWindow
{
public:
    MenuPopupDemo() {
        // 创建触发按钮
        auto button = std::make_unique<DemoButton>("菜单");
        
        // 创建菜单内容
        auto menu = std::make_unique<MenuContent>();
        
        // 创建弹出控件
        auto popup = std::make_unique<UiPopup>(this);
        popup->setTrigger(button.get());
        popup->setPopupContent(menu.get());
        popup->setPlacement(UiPopup::Placement::Bottom);
        
        // 添加到UI
        m_uiRoot.add(popup.get());
    }
};
```

### 自定义位置弹出

```cpp
// 在指定全局坐标显示弹出窗口
QPoint globalPos = QPoint(100, 100);
popup->setPlacement(UiPopup::Placement::Custom);
popup->setOffset(globalPos - triggerPos);
popup->showPopup();
```

## 注意事项

### 性能考虑

- 弹出窗口是独立的OpenGL上下文，有一定的GPU内存开销
- 建议按需创建和销毁弹出窗口，避免同时显示过多弹出窗口

### 平台兼容性

- 在Windows平台上表现最佳
- 在macOS和Linux上可能需要额外的窗口管理器配置

### 限制

- 弹出窗口不能超出屏幕边界
- 某些窗口管理器可能对子窗口有特殊处理

## 测试验证

提供了两个测试程序：

1. **simple_popup_demo.cpp** - 最小化实现，验证基本功能
2. **popup_demo.cpp** - 完整实现，展示所有特性

运行测试：
```bash
# 编译并运行简单演示
cd build
make simple_popup_demo
./simple_popup_demo

# 测试步骤：
# 1. 点击主窗口显示弹出
# 2. 按空格键在窗口外显示弹出
# 3. 按ESC键关闭弹出
```

## 扩展开发

### 自定义弹出内容

继承IUiComponent实现自定义的弹出内容：

```cpp
class CustomPopupContent : public IUiComponent, public IUiContent {
    // 实现必要的接口方法
    void append(Render::FrameData& fd) const override;
    bool onMousePress(const QPoint& pos) override;
    // ...
};
```

### 动画效果

可以在弹出内容中实现淡入淡出等动画效果：

```cpp
bool CustomPopupContent::tick() override {
    // 更新动画状态
    updateAnimation();
    return hasActiveAnimation();
}
```

## 版本历史

- v1.0 - 初始实现，支持基本的弹出窗口功能
- 未来版本将支持更多动画效果和位置策略