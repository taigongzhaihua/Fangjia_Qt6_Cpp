# 布局系统详解（中文文档）

本文档详细介绍 UI 框架的布局系统，包括 ILayoutable 接口、SizeConstraints 概念，以及 UiPanel、UiBoxLayout、UiGrid 等核心布局容器的使用方法。

## ILayoutable 接口与 SizeConstraints

### ILayoutable 基本概念

`ILayoutable` 接口（位于 `presentation/ui/base/ILayoutable.hpp`）定义了参与布局计算的组件协议：

```cpp
class ILayoutable {
public:
    // 测量阶段：计算在给定约束下的期望尺寸
    virtual QSizeF measure(const SizeConstraints& constraints) = 0;
    
    // 安排阶段：在给定矩形内安排子组件
    virtual void arrange(const QRectF& rect) = 0;
};
```

### SizeConstraints 约束系统

`SizeConstraints` 描述了布局约束条件：

```cpp
struct SizeConstraints {
    QSizeF minSize;    // 最小尺寸
    QSizeF maxSize;    // 最大尺寸（可以是无限大）
    
    // 辅助方法
    bool hasFiniteWidth() const;
    bool hasFiniteHeight() const;
    SizeConstraints withWidth(float width) const;
    SizeConstraints withHeight(float height) const;
};
```

约束类型：
- **固定约束**: 明确指定宽度或高度（如 `width: 200px`）
- **范围约束**: 指定最小/最大范围（如 `min-width: 100px, max-width: 300px`）
- **无限约束**: 允许组件使用其自然尺寸（如高度适应内容）

## UiPanel：顺序布局容器

`UiPanel`（位于 `presentation/ui/containers/UiPanel.h`）实现简单的顺序布局（水平或垂直）。

### 基本用法

```cpp
#include "containers/UiPanel.h"

auto panel = std::make_unique<UiPanel>();
panel->setDirection(UiPanel::Direction::Vertical);    // 垂直布局
panel->setCrossAlignment(UiPanel::CrossAlign::Center); // 交叉轴居中

// 添加子组件
panel->addChild(std::make_unique<UiButton>());
panel->addChild(std::make_unique<UiLabel>());
```

### 方向控制

```cpp
enum class Direction {
    Horizontal,  // 水平排列：子项从左到右
    Vertical     // 垂直排列：子项从上到下
};
```

### 交叉轴对齐

交叉轴是垂直于主轴的方向（水平布局时的垂直方向，垂直布局时的水平方向）：

```cpp
enum class CrossAlign {
    Start,    // 交叉轴起始对齐（左对齐或顶部对齐）
    Center,   // 交叉轴居中对齐
    End,      // 交叉轴结束对齐（右对齐或底部对齐）
    Stretch   // 拉伸填充交叉轴
};
```

### 测量与安排算法

**测量阶段**：
1. 沿主轴方向：累加所有子项的主轴尺寸
2. 沿交叉轴方向：取所有子项的最大交叉轴尺寸
3. 应用容器的边距和间距

**安排阶段**：
1. 计算每个子项在主轴上的位置（考虑间距）
2. 根据交叉轴对齐方式计算交叉轴位置
3. 调用子项的 `arrange()` 方法

### 实际示例

```cpp
// 创建垂直布局的工具栏
auto toolbar = std::make_unique<UiPanel>();
toolbar->setDirection(UiPanel::Direction::Horizontal);
toolbar->setCrossAlignment(UiPanel::CrossAlign::Center);
toolbar->setSpacing(8); // 子项间距 8px

toolbar->addChild(createButton("Save"));
toolbar->addChild(createButton("Load")); 
toolbar->addChild(createSeparator());
toolbar->addChild(createButton("Exit"));
```

## UiBoxLayout：权重与空间分配

`UiBoxLayout`（位于 `presentation/ui/containers/UiBoxLayout.h`）提供更高级的布局功能，支持权重分配和复杂的对齐方式。

### 核心特性

- **权重系统**: 子项可按权重分配空间
- **主轴对齐**: 多种空间分配策略
- **SizeMode**: 权重模式 vs 自然尺寸模式
- **边距和间距**: 精确的空间控制

### SizeMode 模式

```cpp
enum class SizeMode {
    Natural,   // 使用子项的自然尺寸（measure 结果）
    Weighted   // 根据权重分配可用空间
};
```

### 主轴对齐方式

```cpp
enum class MainAlign {
    Start,         // 起始对齐：子项向主轴起始聚集
    Center,        // 居中对齐：子项在主轴中心聚集  
    End,           // 结束对齐：子项向主轴结束聚集
    SpaceBetween,  // 两端对齐：子项间等间距，首末贴边
    SpaceAround,   // 周围分布：子项周围等间距
    SpaceEvenly    // 均匀分布：子项和间隙等宽度
};
```

### 使用示例

```cpp
auto layout = std::make_unique<UiBoxLayout>();
layout->setDirection(UiBoxLayout::Direction::Horizontal);
layout->setMainAlignment(UiBoxLayout::MainAlign::SpaceBetween);
layout->setCrossAlignment(UiBoxLayout::CrossAlign::Stretch);

// 添加权重子项
layout->addChild(std::make_unique<UiButton>(), 1.0f, UiBoxLayout::SizeMode::Weighted);
layout->addChild(std::make_unique<UiButton>(), 2.0f, UiBoxLayout::SizeMode::Weighted);
layout->addChild(std::make_unique<UiButton>(), 1.0f, UiBoxLayout::SizeMode::Weighted);
// 结果：第二个按钮占据两倍空间
```

### 边距和间距

```cpp
// 设置容器边距（内容到边界的距离）
layout->setMargin(16, 8, 16, 8); // 上、右、下、左

// 设置子项间距
layout->setSpacing(12); // 子项之间 12px 间距
```

### contentRect 的裁剪传播

UiBoxLayout 支持内容区域裁剪：

```cpp
class UiBoxLayout : public IUiComponent, public IUiContent {
public:
    // 设置内容视口，用于裁剪子内容
    void setViewportRect(const QRectF& viewport) override {
        m_contentRect = viewport;
        
        // 传播到支持裁剪的子项
        for (auto& child : m_children) {
            if (auto* content = dynamic_cast<IUiContent*>(child.get())) {
                content->setViewportRect(calculateChildViewport(child));
            }
        }
    }
};
```

## UiGrid：网格布局系统

`UiGrid`（位于 `presentation/ui/containers/UiGrid.h`）实现 WPF 风格的网格布局，支持 Auto、Pixel、Star 轨道定义。

### 轨道定义类型

```cpp
enum class TrackSizeType {
    Auto,   // 自适应：根据内容自动调整大小
    Pixel,  // 固定像素：指定确切的像素值
    Star    // 星号权重：按比例分配剩余空间
};

struct TrackDefinition {
    TrackSizeType type;
    float value;  // Pixel 的像素值或 Star 的权重值
};
```

### 网格配置示例

```cpp
auto grid = std::make_unique<UiGrid>();

// 定义列：固定宽度 + 自适应 + 权重分配
grid->addColumnDefinition({UiGrid::TrackSizeType::Pixel, 100});  // 100px 固定列
grid->addColumnDefinition({UiGrid::TrackSizeType::Auto, 0});     // 自适应列
grid->addColumnDefinition({UiGrid::TrackSizeType::Star, 1});     // 1* 权重列
grid->addColumnDefinition({UiGrid::TrackSizeType::Star, 2});     // 2* 权重列

// 定义行：自适应标题 + 权重内容
grid->addRowDefinition({UiGrid::TrackSizeType::Auto, 0});        // 标题行
grid->addRowDefinition({UiGrid::TrackSizeType::Star, 1});        // 内容行
```

### 子项放置

```cpp
// 基本放置：指定行列位置
grid->addChild(std::make_unique<UiLabel>("Title"), 0, 0);  // 第0行第0列

// 跨列放置：标题跨越所有列
grid->addChild(std::make_unique<UiLabel>("Header"), 0, 0, 1, 4);  // 跨4列

// 跨行放置：侧边栏跨越多行
grid->addChild(std::make_unique<UiPanel>(), 1, 0, 2, 1);         // 跨2行
```

### 内容测量算法

Grid 使用复杂的两阶段测量算法：

**阶段一：Intrinsic 测量**
1. 测量所有 Auto 轨道的内容需求
2. 计算 Pixel 轨道的固定尺寸
3. 确定剩余空间用于 Star 轨道

**阶段二：Bounded 测量** 
1. 根据第一阶段结果分配 Star 轨道空间
2. 对每个子项进行最终的约束测量
3. 处理跨行/跨列子项的空间分配

### 空间分配算法

```cpp
// Star 轨道空间分配示例
float totalStarWeight = 0;
for (auto& track : starTracks) {
    totalStarWeight += track.value;
}

for (auto& track : starTracks) {
    float ratio = track.value / totalStarWeight;
    track.allocatedSize = availableSpace * ratio;
}
```

### 舍入补偿

为避免浮点舍入误差导致的间隙：

```cpp
// 最后一个 Star 轨道补偿舍入误差
float usedSpace = 0;
for (int i = 0; i < starTracks.size() - 1; ++i) {
    starTracks[i].pixelSize = std::round(starTracks[i].allocatedSize);
    usedSpace += starTracks[i].pixelSize;
}
// 最后一个轨道获得剩余空间
starTracks.back().pixelSize = totalAvailable - usedSpace;
```

### 实际应用示例

```cpp
// 经典的应用程序布局
auto appGrid = std::make_unique<UiGrid>();

// 列定义：侧边栏 + 主内容
appGrid->addColumnDefinition({UiGrid::TrackSizeType::Pixel, 200}); // 侧边栏200px
appGrid->addColumnDefinition({UiGrid::TrackSizeType::Star, 1});     // 主内容区域

// 行定义：标题栏 + 内容 + 状态栏  
appGrid->addRowDefinition({UiGrid::TrackSizeType::Auto, 0});        // 标题栏自适应
appGrid->addRowDefinition({UiGrid::TrackSizeType::Star, 1});        // 内容区域
appGrid->addRowDefinition({UiGrid::TrackSizeType::Pixel, 24});      // 状态栏24px

// 放置组件
appGrid->addChild(createTitleBar(), 0, 0, 1, 2);      // 标题栏跨两列
appGrid->addChild(createSidebar(), 1, 0);             // 侧边栏
appGrid->addChild(createMainContent(), 1, 1);         // 主内容
appGrid->addChild(createStatusBar(), 2, 0, 1, 2);     // 状态栏跨两列
```

## 布局调试与性能

### 测量缓存

容器可以缓存子项的测量结果：

```cpp
struct MeasureCache {
    SizeConstraints lastConstraints;
    QSizeF lastResult;
    bool valid = false;
    
    bool matches(const SizeConstraints& constraints) const {
        return valid && lastConstraints == constraints;
    }
};
```

### 布局失效

只有在影响布局的属性变化时才触发重新布局：

```cpp
void UiPanel::setSpacing(int spacing) {
    if (m_spacing != spacing) {
        m_spacing = spacing;
        invalidateLayout(); // 标记布局失效
    }
}
```

### 调试工具

```cpp
// 开发模式下的布局可视化
#ifdef DEBUG_LAYOUT
void UiGrid::debugDrawGrid(Render::FrameData& fd) {
    // 绘制网格线辅助调试
    for (int i = 0; i <= m_columnCount; ++i) {
        float x = calculateColumnPosition(i);
        fd.addLine(QPointF(x, 0), QPointF(x, height()), Qt::red);
    }
}
#endif
```

## 声明式布局API

所有布局容器都有对应的声明式封装：

```cpp
// 声明式 Panel
auto panel = UI::panel()
    ->direction(UI::Panel::Direction::Vertical)
    ->crossAlign(UI::Panel::CrossAlign::Center)
    ->spacing(8)
    ->children({
        UI::text("Label 1"),
        UI::text("Label 2"),
        UI::text("Label 3")
    });

// 声明式 Grid
auto grid = UI::grid()
    ->columns({
        {UI::Grid::TrackType::Pixel, 100},
        {UI::Grid::TrackType::Star, 1}
    })
    ->rows({
        {UI::Grid::TrackType::Auto, 0},
        {UI::Grid::TrackType::Star, 1}
    })
    ->children({
        {UI::text("Header"), 0, 0, 1, 2},  // 位置和跨度
        {UI::text("Content"), 1, 0},
        {UI::button("Action"), 1, 1}
    });
```

## 相关文档

- [UI 架构](./UI_ARCHITECTURE.zh-CN.md) - IUiComponent 生命周期和主题传播
- [滚动容器](./SCROLL_VIEW.zh-CN.md) - UiScrollView 的视口管理和布局交互
- [声明式概览](./DECLARATIVE_OVERVIEW.zh-CN.md) - 声明式布局 API 详解
- [声明式 TopBar](./DECLARATIVE_NAV_TOPBAR.zh-CN.md) - TopBar 在 AppShell 中的布局集成