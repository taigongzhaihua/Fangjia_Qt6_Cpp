# Fangjia Qt6 C++ 架构设计（中文）

目标
- 清晰：严格分层、单向依赖
- 可扩展：新增功能模块无需修改核心
- 可维护：最少的全局状态，构造注入，小而稳定的接口
- 现代 CMake + 目标划分；服务与视图模型可测试

高层分层（自上而下）
- App（组装根、窗口、导航）
- Features（Home、Data/Formula、Explore、Favorites、Settings）
- UI Framework（容器/布局、基础/高级控件、声明式外观）
- Rendering（渲染器、图标缓存/加载器、渲染数据）
- Core（配置/设置、主题管理、工具）
- Platform（可选的平台特定辅助）

依赖规则
App → Features → UI Framework → Rendering → Core
App → Core
App → Platform（可选）
禁止向上依赖；Feature 之间互不依赖。

建议的项目结构（已对齐仓库）
- src/
  - app/（main.cpp、MainOpenGlWindow、PageRouter）
  - core/
    - config/AppConfig
    - rendering/(Renderer、IconCache/Loader、RenderData)
    - vm/（如需：ViewModelBase、Command）
    - utils/（Logging/Result 等）
  - framework/
    - base/（IThemeAware、IUiComponent、ILayoutable 等）
    - containers/（UiRoot、UiPage、UiPanel、UiGrid、UiContainer）
    - widgets/（UiNav、UiTopBar、UiTabView、UiTreeList）
    - declarative/（Widget + Basic/Advanced/Layouts/TabView/ComponentWrapper/RebuildHost/Binding）
  - features/
    - home/（HomePage + 演示 VM）
    - data/（DataPage + UiFormulaView + FormulaViewModel）
    - …
- resources/（qrc、icons、fonts）
- docs/（本文档与其他指南）

关键架构决策
1) Composition Root 替代 ServiceLocator
- 在 main.cpp 中构建 AppConfig 与 ThemeManager，并通过构造注入到 MainOpenGlWindow。
- ThemeManager 仅发出模式/生效配色信号，持久化由 AppConfig 统一承担。

2) 渲染分层与上下文
- Renderer 仅负责 GL 资源与 FrameData 绘制。
- IconLoader 为无状态栅格化 helper；IconCache 统一管理纹理与缓存键。
- IUiComponent 通过 updateResourceContext(IconCache*, QOpenGLFunctions*, DPR) 获取必要上下文。

3) UI 框架：保留式（retained）组件 + 声明式 DSL
- IUiComponent 定义统一生命周期：updateLayout → updateResourceContext → append → tick。
- ILayoutable 为可选测量/安排接口（UiPanel/UiGrid/UiContainer 等实现）。
- 声明式层（UI::Text/Icon/Panel/Grid/TabView/Card/...）编译为运行期组件。

4) Feature 垂直切片 + Router 懒加载
- PageRouter 以 id->factory 注册页面，切换时懒创建并缓存当前页。
- UiPage 提供 onAppear/onDisappear 生命周期与标题/卡片样式。

5) 主题：单一来源，事件驱动
- system → ThemeManager.effectiveColorScheme → MainOpenGlWindow::Theme
- UiRoot.propagateThemeChange → 各组件 onThemeChanged
- ThemeManager.modeChanged → AppConfig 写入并保存

数据/主题/渲染流向（简化 ASCII）
```
system ─→ ThemeManager ── modeChanged/effectiveColorSchemeChanged ─→ MainOpenGlWindow
                                                        └─→ AppConfig 持久化
MainOpenGlWindow ─→ UiRoot.propagateThemeChange ─→ 所有 IUiComponent
UiRoot.updateResourceContext(IconCache, GL, DPR) ─→ 所有 IUiComponent
```

命名与约定
- 按层命名空间/前缀：core::/rendering::/ui::/features::/app::（当前以目录划分为主）。
- 偏好 enum class / 构造注入 / 禁止隐式单例。
- 仅在必要处使用宏（平台条件）。

附：与仓库代码映射
- 主题：ThemeManager + AppConfig 映射（见 src/app/main.cpp）。
- UI 容器：UiRoot/UiPage/UiPanel/UiGrid/UiContainer；
- 控件：UiNav、UiTopBar、UiTabView、UiTreeList；
- 声明式：UI::Text/Icon/Panel/Grid/TabView/Card，bindingHost/observe（见 BINDING.md）。