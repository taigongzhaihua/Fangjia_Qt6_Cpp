[English](../../doc/architecture/overview.md) | **简体中文**

# 架构概览

## 系统设计

Fangjia Qt6 C++ 遵循分层架构模式，在多个子系统间明确分离关注点：

- **渲染子系统**: Renderer/RenderData/IconCache 用于高性能图形处理
- **UI 框架**: UiRoot/UiPage/UiPanel/UiGrid 和组件系统用于布局和交互
- **业务视图**: 特定领域的页面，如 Home/Data/Settings
- **模型与配置**: ThemeManager/AppConfig 用于主题管理和持久化

## 渲染管线

渲染系统基于 OpenGL 构建，采用优化的基于命令的渲染：

- **顶点生成**: 矩形到 NDC 顶点缓冲区转换
- **片段着色**: 圆角矩形 SDF 渲染，带有色调的纹理采样
- **裁剪**: 逻辑像素裁剪 → 设备像素裁剪矩形 → glScissor

## 事件与动画系统

- **UiRoot**: 统一的鼠标/滚轮事件分发和捕获
- **组件动画**: 内部组件动画状态（NavRail/UiTabView 等）
- **主循环**: QTimer 驱动的 ~60fps tick() 调用从主窗口

## 主题传播

- **ThemeManager**: 派生有效配色方案（跟随系统/强制明亮/暗黑）
- **UiRoot**: 递归的 onThemeChanged() 传播到组件树

## 线程模型

- **OpenGL 上下文**: 所有 OpenGL 调用必须在具有有效上下文的 UI 线程上
- **数据模型**: 通常在 UI 线程上修改；异步操作需要线程安全
- **资源管理**: 图标/纹理加载与 UI 线程协调

## 组件生命周期

所有 UI 组件实现 IUiComponent 接口，具有标准化的生命周期：

1. **构造**: 组件初始化和默认状态设置
2. **主题应用**: 主题更改时调用 onThemeChanged()
3. **资源上下文**: updateResourceContext() 用于图标缓存和 GL 函数
4. **布局更新**: updateLayout() 用于尺寸和定位
5. **事件处理**: onMousePress/Move/Release 用于用户交互
6. **渲染**: append() 将渲染命令添加到 FrameData
7. **动画**: tick() 用于基于时间的更新和动画

## 数据绑定系统

框架通过以下方式提供响应式数据绑定：

- **BindingHost**: 中央绑定管理和变更传播
- **RebuildHost**: 声明式 UI 重建与环境同步
- **观察者模式**: 组件订阅模型更改以实现自动更新

有关特定子系统的详细信息，请参阅：
- [UI 框架概览](../presentation/ui-framework/overview.md)
- [图形与渲染](../infrastructure/gfx.md)
- [数据绑定](../presentation/binding.md)