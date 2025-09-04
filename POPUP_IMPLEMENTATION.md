# 弹出控件实现 (Popup Control Implementation)

## 概述

本实现为 Fangjia Qt6 C++ 项目添加了一个可以超出窗口边界绘制的弹出控件，基于 QOpenGLWindow 实现。

## 核心功能

✅ **超出窗口边界绘制** - 弹出窗口可以显示在主窗口外部  
✅ **保持父子关系** - 弹出窗口作为主窗口的子窗口  
✅ **集成UI框架** - 完全融入现有的OpenGL UI组件系统  
✅ **事件处理** - 支持鼠标、键盘、滚轮事件  
✅ **主题支持** - 支持明/暗主题自动切换  
✅ **动画系统** - 集成现有的动画框架  

## 文件结构

```
presentation/ui/widgets/
├── UiPopupWindow.h/cpp     # 核心弹出窗口实现
└── UiPopup.h/cpp          # UI框架集成组件

tests/
├── simple_popup_demo.cpp  # 最小化演示程序
└── popup_demo.cpp         # 完整功能演示

examples/
└── popup_integration_example.cpp  # 集成示例代码

doc.zh-cn/ui/
└── popup-control.md       # 详细文档
```

## 快速开始

### 1. 基本使用

```cpp
// 创建弹出控件
auto popup = std::make_unique<UiPopup>(parentWindow);

// 设置内容和配置
popup->setTrigger(triggerButton);
popup->setPopupContent(menuContent);
popup->setPopupSize(QSize(200, 150));
popup->setPlacement(UiPopup::Placement::Bottom);

// 添加到UI
uiRoot.add(popup.get());
```

### 2. 运行演示

```bash
cd build
make simple_popup_demo
./simple_popup_demo
```

**演示操作：**
- 点击主窗口 → 显示弹出窗口
- 按空格键 → 在窗口外部显示弹出
- 按ESC键 → 关闭弹出窗口

## 技术实现

### 架构设计

- **UiPopupWindow**: 继承 QOpenGLWindow，实现可独立显示的弹出窗口
- **UiPopup**: 适配器组件，集成到现有UI框架，管理触发器和弹出内容

### 关键特性

1. **超出边界显示**: 使用子窗口机制，可在父窗口外显示
2. **资源共享**: 与主窗口共享OpenGL上下文和渲染资源
3. **位置策略**: 支持8种预定义位置 + 自定义位置
4. **自动调整**: 自动检测屏幕边界并调整位置

### 事件处理

- 自动焦点管理（ESC键关闭）
- 事件转发机制（触发器 ↔ 弹出内容）
- 可配置的点击外部关闭行为

## 集成指南

### 在主应用中使用

参考 `examples/popup_integration_example.cpp` 查看完整的集成示例，包括：

- 创建自定义弹出内容组件
- 在主窗口中集成弹出菜单
- 处理事件和主题切换
- 生命周期管理

### 自定义弹出内容

继承 `IUiComponent`, `IUiContent`, `ILayoutable` 接口：

```cpp
class CustomPopupContent : public IUiComponent, public IUiContent, public ILayoutable {
    // 实现接口方法
    void append(Render::FrameData& fd) const override;
    bool onMousePress(const QPoint& pos) override;
    // ...
};
```

## 位置策略

支持的预定义位置：
- `Bottom`, `Top`, `Left`, `Right` - 基本方向
- `BottomLeft`, `BottomRight`, `TopLeft`, `TopRight` - 角落位置  
- `Custom` - 自定义位置

## 注意事项

- 弹出窗口有独立的OpenGL上下文，需要合理管理资源
- 建议按需创建和销毁，避免同时显示过多弹出窗口
- 在某些窗口管理器上可能需要特殊配置

## 测试验证

提供了两个层次的测试：

1. **simple_popup_demo.cpp** - 验证基本功能的最小化实现
2. **popup_demo.cpp** - 展示完整特性的综合演示

## 文档

详细文档请参考：`doc.zh-cn/ui/popup-control.md`

## 贡献

本实现完全集成到现有的项目架构中，遵循项目的编码规范和设计模式。所有组件都支持主题切换、动画系统和事件处理。