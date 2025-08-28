# 声明式 UI 绑定指南（Binding）

本项目的绑定层遵循"简单优先"的原则：
- 属性类绑定（直接设置组件属性）
- 变化即重建（BindingHost/RebuildHost）
- 事件回调 → 驱动 ViewModel

核心 API
- UI::observe(obj, signal, lambda)：订阅信号触发回调。
- bindingHost(builder).connect(connector)：
  - builder: () -> WidgetPtr，负责根据当前 VM 状态构建一棵子树；
  - connector: (RebuildHost*) -> void，在此注册 observe；当相应信号触发时 host->requestRebuild()。

最小示例（取自 HomePage 计数器）：
```cpp
// 构建可重建区域
bindingHost([this]() -> WidgetPtr {
  return panel({
    text(QString("当前计数: %1").arg(counterVM->count()))->fontSize(16),
    text(counterVM->count() % 2 == 0 ? "偶数" : "奇数")
  })->vertical();
}).connect([this](UI::RebuildHost* host){
  UI::observe(counterVM.get(), &CounterViewModel::countChanged, [host](){ host->requestRebuild(); });
});
```

TabView 与 ViewModel（取自 DataPage）：
```cpp
tabView()
  ->viewModel(dataViewModel->tabs())
  ->contents(WidgetList{ wrap(formulaView.get()), /* ... */ })
  ->onChanged([this](int idx){ /* 可选：埋点/日志 */ });
```

最佳实践
- 将"重建区域"尽量收敛在粒度合适的子树，避免整页重建。
- 对于高频/细粒度变化，优先使用控件自身的增量属性（如 TextComponent 内部已有裁剪/测量）。
- 绑定逻辑尽量靠近使用处，便于读者从 UI 直达 VM。
- VM 只暴露状态（Q_PROPERTY/NOTIFY）与命令（函数/可选 Command），不直接持有 GL/缓存等渲染上下文。

FAQ
- 与 updateResourceContext 的关系？
  - 二者互补：前者是数据驱动 UI，后者是资源上下文（IconCache/GL/DPR）注入，二者均由容器（UiRoot/UiPage/UiTabView 等）调度。