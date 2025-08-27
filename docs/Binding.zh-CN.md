# 声明式绑定（BindingHost）与"变化即重建"

本页介绍如何在当前声明式 UI 框架中，使用 BindingHost + RebuildHost 组合，实现"信号触发 → 子树重建"的数据驱动渲染。

## 核心概念
- RebuildHost：已有的可重建宿主组件（IUiComponent），提供 setBuilder(...) 与 requestRebuild()。
- BindingHost：新的声明式 Widget 封装。
  - 接收一个 Builder（返回 WidgetPtr），在内部创建 RebuildHost 并设置 builder：每次需要重建时重新 build 子树。
  - 通过 connect(Connector) 注册信号连接：例如订阅某个 VM 的 changed 信号，在回调中调用 host->requestRebuild()。

## 快速上手
```cpp
using UI::bindingHost;
using UI::observe;

auto vm = /* 某个 QObject 派生的 ViewModel 指针 */;

auto view = bindingHost([vm]{
    // 根据 VM 的当前状态生成一段 UI
    if (/* vm->isEmpty() */) {
        return container(text("暂无数据")->align(Qt::AlignCenter));
    }
    return panel({
        text("标题")->fontSize(16),
        // ... 更多基于 vm 的部件 ...
    })->vertical();
})
->connect([vm](UI::RebuildHost* host){
    // 订阅 VM 的变化信号，触发重建
    observe(vm, &MyViewModel::dataChanged, [host]{ host->requestRebuild(); });
    observe(vm, &MyViewModel::titleChanged, [host]{ host->requestRebuild(); });
});
```

## 何时使用 BindingHost
- 模板化内容、结构变化频繁或难以增量更新的区域（例如详情页、占位/加载/出错三态切换）。
- 简化开发：避免为少量变化引入复杂的"增量 diff"，直接重建更简单可控。

## 与增量更新的关系
- BindingHost 不排斥增量更新。对简单属性（如颜色/显隐），仍可走组件自身 API 或后续将提供的属性绑定 Helper。
- 若将来引入更细粒度的属性绑定（bindText/bindVisible 等），可与 BindingHost 并行使用。

## 注意事项
- RebuildHost 会在首次构建时自动构建一次（首帧不为空）。
- 连接器里的 observe(...) 需要确保 VM 生命周期长于 BindingHost；否则请在合适的生命周期中断开连接。
- 渲染与 GL 相关上下文仍由 UiRoot/容器负责，BindingHost 内部已复用 RebuildHost 的同步逻辑。