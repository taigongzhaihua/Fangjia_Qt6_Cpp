[English](../../doc/presentation/ui-framework/layouts.md) | **简体中文**

# 布局系统

## 概览

Fangjia 布局系统通过容器类型的层次结构提供灵活高效的 UI 组件定位。每个容器实现针对不同用例优化的特定布局算法，同时保持一致的行为和性能。

## 布局容器类型

### UiPanel - 堆叠布局

`UiPanel` 提供具有自动子组件定位的线性布局功能：

```cpp
class UiPanel : public IUiComponent {
public:
    enum class Direction {
        Vertical,   // 垂直堆叠子组件
        Horizontal  // 水平堆叠子组件
    };
    
    // 布局配置
    void setDirection(Direction direction);
    void setSpacing(int spacing);
    void setPadding(const QMargins& padding);
    void setAlignment(Qt::Alignment alignment);
    void setWrap(bool wrap);  // 允许换行到下一行/列
    
    // 子组件大小策略
    void setChildSizePolicy(IUiComponent* child, SizePolicy policy);
    void setChildStretch(IUiComponent* child, int stretch);
    void setChildMinimumSize(IUiComponent* child, const QSize& size);
    void setChildMaximumSize(IUiComponent* child, const QSize& size);
    
private:
    Direction m_direction = Direction::Vertical;
    int m_spacing = 8;
    QMargins m_padding{8, 8, 8, 8};
    Qt::Alignment m_alignment = Qt::AlignTop | Qt::AlignLeft;
    bool m_wrap = false;
};
```

### UiGrid - 网格布局

`UiGrid` 提供精确的基于网格的定位：

```cpp
class UiGrid : public IUiComponent {
public:
    struct GridItem {
        std::unique_ptr<IUiComponent> component;
        int row;
        int column;
        int rowSpan = 1;
        int columnSpan = 1;
        Qt::Alignment alignment = Qt::AlignCenter;
    };
    
    // 网格结构
    void setRowCount(int rows);
    void setColumnCount(int columns);
    void setUniformRowHeight(bool uniform);
    void setUniformColumnWidth(bool uniform);
    
    // 行/列大小
    void setRowHeight(int row, int height);
    void setColumnWidth(int column, int width);
    void setRowMinimumHeight(int row, int height);
    void setColumnMinimumWidth(int column, int width);
    void setRowStretch(int row, int stretch);
    void setColumnStretch(int column, int stretch);
    
    // 间距
    void setRowSpacing(int spacing);
    void setColumnSpacing(int spacing);
    void setSpacing(int spacing) { setRowSpacing(spacing); setColumnSpacing(spacing); }
    
    // 项目管理
    void addItem(GridItem item);
    void setItem(int row, int column, std::unique_ptr<IUiComponent> component);
    void removeItem(int row, int column);
    
private:
    std::vector<std::vector<GridItem*>> m_grid;
    std::vector<int> m_rowHeights;
    std::vector<int> m_columnWidths;
    std::vector<int> m_rowStretches;
    std::vector<int> m_columnStretches;
    int m_rowSpacing = 4;
    int m_columnSpacing = 4;
};
```

### UiContainer - 自由形式布局

`UiContainer` 允许具有可选约束的绝对定位：

```cpp
class UiContainer : public IUiComponent {
public:
    struct ContainerItem {
        std::unique_ptr<IUiComponent> component;
        QPoint position;
        QSize size;
        Qt::Alignment anchor = Qt::AlignTopLeft;
        QMargins margins;
        bool autoSize = false;
        
        // 基于约束的定位
        struct Constraints {
            std::optional<int> leftMargin;
            std::optional<int> rightMargin; 
            std::optional<int> topMargin;
            std::optional<int> bottomMargin;
            std::optional<int> centerX;
            std::optional<int> centerY;
            IUiComponent* relativeToLeft = nullptr;
            IUiComponent* relativeToRight = nullptr;
            IUiComponent* relativeToTop = nullptr;
            IUiComponent* relativeToBottom = nullptr;
        } constraints;
    };
    
    // 项目管理
    void addItem(ContainerItem item);
    void setItemPosition(IUiComponent* component, const QPoint& position);
    void setItemSize(IUiComponent* component, const QSize& size);
    void setItemConstraints(IUiComponent* component, const ContainerItem::Constraints& constraints);
    
    // 布局求解
    void resolveConstraints();
    void setAutoResolve(bool autoResolve);
    
private:
    std::vector<ContainerItem> m_items;
    bool m_autoResolve = true;
    void solveConstraints();
};
```

## 大小策略

### SizePolicy 枚举

```cpp
enum class SizePolicy {
    Fixed,      // 组件具有固定大小
    Minimum,    // 组件需要最小大小但可以增长
    Maximum,    // 组件可以收缩但有最大首选大小
    Preferred,  // 组件有首选大小但可以增长/收缩
    Expanding,  // 组件希望使用可用空间
    MinimumExpanding, // 类似 Minimum 但也会扩展
    Ignored     // 布局忽略组件大小
};
```

### 大小提示

```cpp
class ILayoutable {
public:
    virtual QSize sizeHint() const = 0;
    virtual QSize minimumSizeHint() const = 0;
    virtual QSize maximumSizeHint() const { return QSize(QWIDGETSIZE_MAX, QWIDGETSIZE_MAX); }
    virtual SizePolicy horizontalSizePolicy() const { return SizePolicy::Preferred; }
    virtual SizePolicy verticalSizePolicy() const { return SizePolicy::Preferred; }
    virtual int heightForWidth(int width) const { return -1; }
    virtual int widthForHeight(int height) const { return -1; }
};
```

## 布局算法

### 堆叠布局算法 (UiPanel)

```cpp
void UiPanel::performLayout(const QSize& availableSize) {
    if (m_children.empty()) return;
    
    QSize contentSize = availableSize - QSize(m_padding.left() + m_padding.right(),
                                             m_padding.top() + m_padding.bottom());
    
    if (m_direction == Direction::Vertical) {
        layoutVertical(contentSize);
    } else {
        layoutHorizontal(contentSize);
    }
}

void UiPanel::layoutVertical(const QSize& contentSize) {
    // 计算总固定大小和拉伸因子
    int totalFixedHeight = 0;
    int totalStretch = 0;
    
    for (auto& child : m_children) {
        if (child->verticalSizePolicy() == SizePolicy::Fixed) {
            totalFixedHeight += child->sizeHint().height();
        } else {
            totalStretch += getChildStretch(child.get());
        }
    }
    
    // 计算间距
    int totalSpacing = (m_children.size() - 1) * m_spacing;
    int availableHeight = contentSize.height() - totalFixedHeight - totalSpacing;
    
    // 定位子组件
    int currentY = m_padding.top();
    
    for (auto& child : m_children) {
        QSize childSize = child->sizeHint();
        
        if (child->verticalSizePolicy() != SizePolicy::Fixed && totalStretch > 0) {
            int stretch = getChildStretch(child.get());
            childSize.setHeight(availableHeight * stretch / totalStretch);
        }
        
        // 应用水平对齐
        int childX = calculateHorizontalPosition(child.get(), childSize.width(), contentSize.width());
        
        child->updateLayout(childSize);
        child->setPosition(QPoint(childX, currentY));
        
        currentY += childSize.height() + m_spacing;
    }
}
```

### 网格布局算法 (UiGrid)

```cpp
void UiGrid::performLayout(const QSize& availableSize) {
    calculateRowHeights(availableSize.height());
    calculateColumnWidths(availableSize.width());
    positionItems();
}

void UiGrid::calculateRowHeights(int availableHeight) {
    // 重置计算的高度
    m_calculatedRowHeights.clear();
    m_calculatedRowHeights.resize(m_rowCount, 0);
    
    // 基于内容计算最小高度
    for (const auto& item : m_items) {
        int minHeight = item.component->minimumSizeHint().height() / item.rowSpan;
        for (int row = item.row; row < item.row + item.rowSpan; ++row) {
            m_calculatedRowHeights[row] = std::max(m_calculatedRowHeights[row], minHeight);
        }
    }
    
    // 基于拉伸因子分配额外空间
    int totalMinHeight = std::accumulate(m_calculatedRowHeights.begin(), m_calculatedRowHeights.end(), 0);
    int extraHeight = availableHeight - totalMinHeight - (m_rowCount - 1) * m_rowSpacing;
    
    if (extraHeight > 0) {
        int totalStretch = std::accumulate(m_rowStretches.begin(), m_rowStretches.end(), 0);
        if (totalStretch > 0) {
            for (int row = 0; row < m_rowCount; ++row) {
                m_calculatedRowHeights[row] += extraHeight * m_rowStretches[row] / totalStretch;
            }
        }
    }
}

void UiGrid::positionItems() {
    for (auto& item : m_items) {
        // 计算项目位置
        int x = 0;
        for (int col = 0; col < item.column; ++col) {
            x += m_calculatedColumnWidths[col] + m_columnSpacing;
        }
        
        int y = 0;
        for (int row = 0; row < item.row; ++row) {
            y += m_calculatedRowHeights[row] + m_rowSpacing;
        }
        
        // 计算项目大小
        int width = 0;
        for (int col = item.column; col < item.column + item.columnSpan; ++col) {
            width += m_calculatedColumnWidths[col];
        }
        width += (item.columnSpan - 1) * m_columnSpacing;
        
        int height = 0;
        for (int row = item.row; row < item.row + item.rowSpan; ++row) {
            height += m_calculatedRowHeights[row];
        }
        height += (item.rowSpan - 1) * m_rowSpacing;
        
        // 在单元格内应用对齐
        QPoint finalPosition = applyAlignment(QPoint(x, y), QSize(width, height), 
                                            item.component->sizeHint(), item.alignment);
        
        item.component->setPosition(finalPosition);
        item.component->updateLayout(QSize(width, height));
    }
}
```

## 响应式布局

### 断点系统

```cpp
class ResponsiveContainer : public IUiComponent {
public:
    enum class Breakpoint {
        XSmall,  // < 576px
        Small,   // 576px - 768px
        Medium,  // 768px - 992px
        Large,   // 992px - 1200px
        XLarge   // >= 1200px
    };
    
    // 响应式配置
    void setLayoutForBreakpoint(Breakpoint bp, std::unique_ptr<IUiComponent> layout);
    void setBreakpointWidths(const QHash<Breakpoint, int>& widths);
    
    // 当前状态
    Breakpoint currentBreakpoint() const;
    
private:
    QHash<Breakpoint, std::unique_ptr<IUiComponent>> m_layouts;
    QHash<Breakpoint, int> m_breakpointWidths;
    Breakpoint m_currentBreakpoint = Breakpoint::Medium;
    
    void updateBreakpoint(int width);
};
```

### 自适应布局

```cpp
class AdaptiveGrid : public UiGrid {
public:
    // 列自适应
    void setAdaptiveColumns(int minColumns, int maxColumns);
    void setColumnBreakpoints(const QVector<int>& breakpoints);
    void setItemMinimumWidth(int width);
    
    // 响应式行为
    void setStackOnNarrow(bool stack);
    void setHideOnNarrow(const QStringList& itemIds);
    
private:
    int m_minColumns = 1;
    int m_maxColumns = 4;
    int m_itemMinWidth = 200;
    QVector<int> m_columnBreakpoints;
    
    void recalculateColumns(int availableWidth);
};
```

## 布局调试

### 调试可视化

```cpp
class LayoutDebugger {
public:
    // 调试渲染
    void setDebugMode(bool enabled);
    void setShowBounds(bool show);
    void setShowMargins(bool show);
    void setShowSpacing(bool show);
    void setShowConstraints(bool show);
    
    // 调试信息
    QString getLayoutInfo(IUiComponent* component) const;
    void dumpLayoutTree(IUiComponent* root) const;
    
    // 可视化调试
    void highlightComponent(IUiComponent* component, const QColor& color);
    void showLayoutPath(IUiComponent* from, IUiComponent* to);
    
private:
    bool m_debugMode = false;
    bool m_showBounds = true;
    bool m_showMargins = true;
    bool m_showSpacing = true;
    QHash<IUiComponent*, QColor> m_highlights;
};
```

### 性能指标

```cpp
class LayoutProfiler {
public:
    struct LayoutMetrics {
        int layoutTime;     // 布局计算时间（微秒）
        int componentCount; // 布局的组件数量
        int depth;          // 布局树深度
        int iterations;     // 布局遍数
        QStringList bottlenecks; // 花费最多时间的组件
    };
    
    // 性能分析控制
    void startProfiling();
    void stopProfiling();
    LayoutMetrics getMetrics() const;
    
    // 性能分析
    void identifyBottlenecks();
    void suggestOptimizations();
    
private:
    QElapsedTimer m_timer;
    LayoutMetrics m_metrics;
    QHash<IUiComponent*, qint64> m_componentTimes;
};
```

## 布局最佳实践

### 性能优化

1. **最小化布局遍数**: 设计布局以最小化约束求解迭代次数
2. **缓存大小提示**: 尽可能缓存昂贵的大小计算
3. **使用固定大小**: 对于不需要调整大小的组件优先使用固定大小
4. **延迟布局**: 仅在可见或大小更改时执行布局

### 内存管理

```cpp
class LayoutCache {
public:
    // 大小提示缓存
    void cacheSizeHint(IUiComponent* component, const QSize& hint);
    QSize getCachedSizeHint(IUiComponent* component) const;
    void invalidateCache(IUiComponent* component);
    
    // 布局结果缓存
    void cacheLayout(IUiComponent* container, const QHash<IUiComponent*, QRect>& layout);
    bool hasValidCache(IUiComponent* container) const;
    
private:
    QHash<IUiComponent*, QSize> m_sizeHintCache;
    QHash<IUiComponent*, QHash<IUiComponent*, QRect>> m_layoutCache;
    QHash<IUiComponent*, qint64> m_cacheTimestamps;
};
```

### 常见模式

#### 双面板布局
```cpp
auto createTwoPanelLayout(std::unique_ptr<IUiComponent> left, 
                         std::unique_ptr<IUiComponent> right,
                         int leftWidth = 300) {
    auto container = std::make_unique<UiContainer>();
    
    // 左面板 - 固定宽度
    UiContainer::ContainerItem leftItem;
    leftItem.component = std::move(left);
    leftItem.constraints.leftMargin = 0;
    leftItem.constraints.topMargin = 0;
    leftItem.constraints.bottomMargin = 0;
    leftItem.size = QSize(leftWidth, -1); // 自动高度
    container->addItem(std::move(leftItem));
    
    // 右面板 - 剩余宽度
    UiContainer::ContainerItem rightItem;
    rightItem.component = std::move(right);
    rightItem.constraints.leftMargin = leftWidth;
    rightItem.constraints.rightMargin = 0;
    rightItem.constraints.topMargin = 0;
    rightItem.constraints.bottomMargin = 0;
    container->addItem(std::move(rightItem));
    
    return container;
}
```

#### 卡片布局
```cpp
auto createCardLayout(const std::vector<std::unique_ptr<IUiComponent>>& cards,
                     int cardsPerRow = 3,
                     int cardSpacing = 16) {
    auto grid = std::make_unique<UiGrid>();
    grid->setColumnCount(cardsPerRow);
    grid->setSpacing(cardSpacing);
    grid->setUniformColumnWidth(true);
    
    for (size_t i = 0; i < cards.size(); ++i) {
        UiGrid::GridItem item;
        item.component = std::move(cards[i]);
        item.row = i / cardsPerRow;
        item.column = i % cardsPerRow;
        item.alignment = Qt::AlignTop | Qt::AlignLeft;
        grid->addItem(std::move(item));
    }
    
    return grid;
}
```

## 相关文档

- [UI 框架概览](overview.md)
- [主题与渲染](theme-and-rendering.md)
- [组件文档](../components/)