# WinWindowChrome 非客户区渲染修复

## 问题描述
原始实现中，`windowchrome依然没有生效，窗口并没有突入非客户区绘制` - 自定义窗口框架没有正确扩展到非客户区进行绘制。

## 根本原因
1. `DwmExtendFrameIntoClientArea` 调用使用了 `{0, 0, 0, 0}` 边距，这意味着"扩展边框0像素"，实际上没有启用框架扩展
2. `WM_NCACTIVATE` 消息处理返回 `false`，阻止了自定义窗口激活状态的绘制
3. 缺少窗口样式修改以移除默认标题栏

## 解决方案

### 1. 修复框架扩展边距
```cpp
// 修改前
constexpr MARGINS m{ 0, 0, 0, 0 };
DwmExtendFrameIntoClientArea(h, &m); // 保持 DWM 阴影

// 修改后  
constexpr MARGINS m{ 1, 1, 1, 1 };
DwmExtendFrameIntoClientArea(h, &m); // 启用非客户区扩展并保持 DWM 阴影
```

**说明**: `{1, 1, 1, 1}` 表示在所有边上扩展1像素的框架到客户区，这会启用 DWM 的玻璃效果并允许自定义绘制。

### 2. 移除默认标题栏
```cpp
LONG_PTR style = GetWindowLongPtr(h, GWL_STYLE);
style &= ~WS_CAPTION;  // 移除默认标题栏
SetWindowLongPtr(h, GWL_STYLE, style);
```

**说明**: 移除 `WS_CAPTION` 样式以隐藏系统默认标题栏，但保留其他重要功能如调整大小、任务栏集成等。

### 3. 修复激活状态处理
```cpp
// 修改前
case WM_NCACTIVATE:
    return false;

// 修改后
case WM_NCACTIVATE:
    // 对于自定义窗口 chrome，我们需要允许激活状态绘制，但阻止默认的非客户区绘制
    if (result) *result = TRUE;
    return true;
```

**说明**: 正确处理 `WM_NCACTIVATE` 消息以允许自定义窗口激活状态的绘制。

## 预期效果
- 窗口现在应该显示自定义标题栏而不是系统默认标题栏
- 应用可以在非客户区进行自定义绘制
- 保持窗口阴影、调整大小、任务栏集成等功能
- 自定义拖拽区域和按钮应该正常工作

## 测试方法
1. 构建并运行应用
2. 验证窗口没有默认的 Windows 标题栏
3. 测试自定义标题栏的拖拽和按钮功能
4. 确认窗口边框调整大小正常工作
5. 检查任务栏集成和 Alt+Tab 行为

## 兼容性注意事项
- 需要 Windows Vista 或更高版本（支持 DWM）
- 需要启用 DWM 合成（通常默认启用）
- 某些 Windows 主题或设置可能影响效果