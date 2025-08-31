[English](../../doc/presentation/ui/components.md) | **简体中文**

# UI 基础部件与容器

本文档介绍 Fangjia Qt6 C++ 框架中的核心 UI 组件，包括基础容器（UiPanel、UiBoxLayout、UiGrid）和滚动容器（UiScrollView），以及它们的配置选项与使用模式。

## 容器组件概览

### UiPanel - 基础面板容器

`UiPanel` 是最基础的容器组件，提供简单的子组件堆叠功能。

```cpp
auto panel = UI::panel()
    ->children({
        UI::button()->text("按钮1"),
        UI::button()->text("按钮2"),
        UI::label()->text("标签文本")
    })
    ->padding(16)                    // 内边距
    ->background(QColor(250, 250, 250)); // 背景色
```

**特点**：
- **简单布局**: 子组件按添加顺序堆叠
- **装饰器支持**: 支持 padding、margin、background、border 等装饰器
- **轻量级**: 最小的布局开销，适合简单场景

### UiBoxLayout - 线性布局容器

`UiBoxLayout` 提供水平或垂直的线性布局功能。

```cpp
// 水平布局
auto hbox = UI::boxLayout(UI::BoxLayout::Horizontal)
    ->children({
        UI::button()->text("左"),
        UI::spacer(),                    // 弹性空间
        UI::button()->text("右")
    })
    ->spacing(8);                        // 子组件间距

// 垂直布局
auto vbox = UI::boxLayout(UI::BoxLayout::Vertical)
    ->children({
        UI::label()->text("标题"),
        UI::panel()->fixedHeight(200),   // 固定高度面板
        UI::button()->text("确定")
    })
    ->spacing(12);
```

**配置选项**：
- **方向**: `Horizontal` 或 `Vertical`
- **间距**: `spacing()` 设置子组件间距
- **对齐**: 支持多种对齐方式（start、center、end、stretch）
- **权重分配**: 子组件可设置拉伸权重

### UiGrid - 网格布局容器

`UiGrid` 提供强大的网格布局功能，支持复杂的行列配置。

```cpp
auto grid = UI::grid()
    ->rows({
        UI::GridTrack::fixed(40),        // 固定 40px 行
        UI::GridTrack::flex(1),          // 弹性行，权重 1
        UI::GridTrack::auto_()           // 自动调整行
    })
    ->columns({
        UI::GridTrack::fixed(120),       // 固定 120px 列
        UI::GridTrack::flex(2),          // 弹性列，权重 2
        UI::GridTrack::flex(1)           // 弹性列，权重 1
    })
    ->children({
        UI::label()->text("标题")->gridArea(0, 0, 1, 3),  // 跨 3 列
        UI::button()->text("按钮1")->gridArea(1, 0),
        UI::panel()->background(QColor(240, 240, 240))->gridArea(1, 1, 2, 1)  // 跨 2 行
    })
    ->gap(8, 12);                        // 行间距 8px，列间距 12px
```

**轨道类型**：
- **`GridTrack::fixed(px)`**: 固定像素大小
- **`GridTrack::flex(weight)`**: 按权重分配剩余空间
- **`GridTrack::auto_()`**: 根据内容自动调整大小
- **`GridTrack::minMax(min, max)`**: 指定最小/最大尺寸范围

**网格区域配置**：
```cpp
// gridArea(row, col, rowSpan, colSpan)
widget->gridArea(1, 2, 2, 1);  // 从第 1 行第 2 列开始，跨 2 行 1 列
```

## 滚动容器

### UiScrollView - 滚动视图容器

`UiScrollView` 为内容提供垂直滚动功能，支持平滑滚动与滚动条渲染。

```cpp
auto scrollView = UI::scrollView(
    UI::panel()
        ->children({
            UI::label()->text("项目 1"),
            UI::label()->text("项目 2"),
            UI::label()->text("项目 3"),
            // ... 更多内容
        })
)->scrollBarVisible(true)               // 显示滚动条
  ->fadeInOut(true);                    // 滚动条淡入淡出动画
```

**滚动特性**：
- **Width-bounded 测量**: 内容宽度受容器约束，高度可超出
- **Viewport 坐标**: 支持顶部偏移滚动
- **平滑动画**: 鼠标滚轮触发平滑滚动动画
- **滚动条渲染**: 可选的滚动条显示与动画

**高级配置**：
```cpp
auto advancedScroll = UI::scrollView(content)
    ->scrollSpeed(120)                  // 滚动速度（像素/滚轮单位）
    ->scrollBarWidth(8)                 // 滚动条宽度
    ->scrollBarMargin(4)                // 滚动条边距
    ->animationDuration(200);           // 滚动动画时长（毫秒）
```

## 布局协议与接口

### ILayoutable 接口

所有容器组件实现 `ILayoutable` 接口：

```cpp
class ILayoutable {
public:
    virtual QSize measure(const SizeConstraints& constraints) = 0;
    virtual void arrange(const QRect& bounds) = 0;
};
```

**测量阶段**（`measure`）:
- 基于约束条件计算组件的期望尺寸
- 递归测量所有子组件
- 返回最终的布局尺寸

**安排阶段**（`arrange`）:
- 基于分配的矩形区域放置组件
- 递归安排所有子组件的位置和大小

### SizeConstraints 约束系统

布局约束定义了组件可用的尺寸范围：

```cpp
struct SizeConstraints {
    int minWidth, maxWidth;      // 宽度约束
    int minHeight, maxHeight;    // 高度约束
    
    // 便利构造函数
    static SizeConstraints fixed(int w, int h);           // 固定尺寸
    static SizeConstraints unbounded();                   // 无约束
    static SizeConstraints widthFixed(int w);             // 固定宽度
    static SizeConstraints heightFixed(int h);            // 固定高度
};
```

## 容器选择指南

### 使用场景建议

**选择 UiPanel 当**：
- 需要简单的组件堆叠
- 不需要复杂的布局计算
- 主要依赖装饰器（padding、margin）控制间距

**选择 UiBoxLayout 当**：
- 需要水平或垂直线性排列
- 需要弹性空间分配
- 需要统一的组件间距

**选择 UiGrid 当**：
- 需要复杂的二维布局
- 需要组件跨行或跨列
- 需要精确的尺寸控制

**选择 UiScrollView 当**：
- 内容可能超出容器尺寸
- 需要垂直滚动功能
- 需要滚动条视觉反馈

### 性能考虑

- **UiPanel**: 最低布局开销，适合静态内容
- **UiBoxLayout**: 一维布局计算，性能良好
- **UiGrid**: 二维布局计算，适中的性能开销
- **UiScrollView**: 额外的滚动逻辑，但支持大量内容

## 代码示例

### 典型应用布局

```cpp
auto createMainLayout() {
    return UI::grid()
        ->rows({
            UI::GridTrack::fixed(60),    // 顶部栏
            UI::GridTrack::flex(1),      // 主内容区
            UI::GridTrack::fixed(40)     // 状态栏
        })
        ->columns({
            UI::GridTrack::fixed(200),   // 侧边栏
            UI::GridTrack::flex(1)       // 主内容
        })
        ->children({
            // 顶部栏（跨两列）
            UI::topBar()->gridArea(0, 0, 1, 2),
            
            // 侧边导航
            UI::navRail()->gridArea(1, 0),
            
            // 主内容区（带滚动）
            UI::scrollView(
                createMainContent()
            )->gridArea(1, 1),
            
            // 状态栏（跨两列）
            UI::statusBar()->gridArea(2, 0, 1, 2)
        });
}
```

## 相关文档

- [表现层架构概览](../architecture.md) - 组件生命周期与事件处理
- [声明式 TopBar 组件](topbar/declarative-topbar.md) - TopBar 具体使用方法
- [Binding 与响应式重建](../binding.md) - 动态内容更新机制