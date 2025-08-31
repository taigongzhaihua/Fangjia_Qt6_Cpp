**English** | [简体中文](../../../doc.zh-cn/presentation/ui-framework/layouts.md)

# Layout System

## Overview

The Fangjia layout system provides flexible and efficient positioning of UI components through a hierarchy of container types. Each container implements specific layout algorithms optimized for different use cases while maintaining consistent behavior and performance.

## Layout Container Types

### UiPanel - Stack Layouts

`UiPanel` provides linear layout capabilities with automatic child positioning:

```cpp
class UiPanel : public IUiComponent {
public:
    enum class Direction {
        Vertical,   // Stack children vertically
        Horizontal  // Stack children horizontally
    };
    
    // Layout configuration
    void setDirection(Direction direction);
    void setSpacing(int spacing);
    void setPadding(const QMargins& padding);
    void setAlignment(Qt::Alignment alignment);
    void setWrap(bool wrap);  // Allow wrapping to next line/column
    
    // Child size policies
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

### UiGrid - Grid Layouts

`UiGrid` provides precise grid-based positioning:

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
    
    // Grid structure
    void setRowCount(int rows);
    void setColumnCount(int columns);
    void setUniformRowHeight(bool uniform);
    void setUniformColumnWidth(bool uniform);
    
    // Row/column sizing
    void setRowHeight(int row, int height);
    void setColumnWidth(int column, int width);
    void setRowMinimumHeight(int row, int height);
    void setColumnMinimumWidth(int column, int width);
    void setRowStretch(int row, int stretch);
    void setColumnStretch(int column, int stretch);
    
    // Spacing
    void setRowSpacing(int spacing);
    void setColumnSpacing(int spacing);
    void setSpacing(int spacing) { setRowSpacing(spacing); setColumnSpacing(spacing); }
    
    // Item management
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

### UiContainer - Free-Form Layouts

`UiContainer` allows absolute positioning with optional constraints:

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
        
        // Constraint-based positioning
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
    
    // Item management
    void addItem(ContainerItem item);
    void setItemPosition(IUiComponent* component, const QPoint& position);
    void setItemSize(IUiComponent* component, const QSize& size);
    void setItemConstraints(IUiComponent* component, const ContainerItem::Constraints& constraints);
    
    // Layout solving
    void resolveConstraints();
    void setAutoResolve(bool autoResolve);
    
private:
    std::vector<ContainerItem> m_items;
    bool m_autoResolve = true;
    void solveConstraints();
};
```

## Size Policies

### SizePolicy Enumeration

```cpp
enum class SizePolicy {
    Fixed,      // Component has fixed size
    Minimum,    // Component wants minimum size but can grow
    Maximum,    // Component can shrink but has maximum preferred size
    Preferred,  // Component has preferred size but can grow/shrink
    Expanding,  // Component wants to use available space
    MinimumExpanding, // Like Minimum but also expands
    Ignored     // Component size is ignored by layout
};
```

### Size Hints

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

## Layout Algorithms

### Stack Layout Algorithm (UiPanel)

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
    // Calculate total fixed sizes and stretch factors
    int totalFixedHeight = 0;
    int totalStretch = 0;
    
    for (auto& child : m_children) {
        if (child->verticalSizePolicy() == SizePolicy::Fixed) {
            totalFixedHeight += child->sizeHint().height();
        } else {
            totalStretch += getChildStretch(child.get());
        }
    }
    
    // Calculate spacing
    int totalSpacing = (m_children.size() - 1) * m_spacing;
    int availableHeight = contentSize.height() - totalFixedHeight - totalSpacing;
    
    // Position children
    int currentY = m_padding.top();
    
    for (auto& child : m_children) {
        QSize childSize = child->sizeHint();
        
        if (child->verticalSizePolicy() != SizePolicy::Fixed && totalStretch > 0) {
            int stretch = getChildStretch(child.get());
            childSize.setHeight(availableHeight * stretch / totalStretch);
        }
        
        // Apply horizontal alignment
        int childX = calculateHorizontalPosition(child.get(), childSize.width(), contentSize.width());
        
        child->updateLayout(childSize);
        child->setPosition(QPoint(childX, currentY));
        
        currentY += childSize.height() + m_spacing;
    }
}
```

### Grid Layout Algorithm (UiGrid)

```cpp
void UiGrid::performLayout(const QSize& availableSize) {
    calculateRowHeights(availableSize.height());
    calculateColumnWidths(availableSize.width());
    positionItems();
}

void UiGrid::calculateRowHeights(int availableHeight) {
    // Reset calculated heights
    m_calculatedRowHeights.clear();
    m_calculatedRowHeights.resize(m_rowCount, 0);
    
    // Calculate minimum heights based on content
    for (const auto& item : m_items) {
        int minHeight = item.component->minimumSizeHint().height() / item.rowSpan;
        for (int row = item.row; row < item.row + item.rowSpan; ++row) {
            m_calculatedRowHeights[row] = std::max(m_calculatedRowHeights[row], minHeight);
        }
    }
    
    // Distribute extra space based on stretch factors
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
        // Calculate item position
        int x = 0;
        for (int col = 0; col < item.column; ++col) {
            x += m_calculatedColumnWidths[col] + m_columnSpacing;
        }
        
        int y = 0;
        for (int row = 0; row < item.row; ++row) {
            y += m_calculatedRowHeights[row] + m_rowSpacing;
        }
        
        // Calculate item size
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
        
        // Apply alignment within cell
        QPoint finalPosition = applyAlignment(QPoint(x, y), QSize(width, height), 
                                            item.component->sizeHint(), item.alignment);
        
        item.component->setPosition(finalPosition);
        item.component->updateLayout(QSize(width, height));
    }
}
```

## Responsive Layout

### Breakpoint System

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
    
    // Responsive configuration
    void setLayoutForBreakpoint(Breakpoint bp, std::unique_ptr<IUiComponent> layout);
    void setBreakpointWidths(const QHash<Breakpoint, int>& widths);
    
    // Current state
    Breakpoint currentBreakpoint() const;
    
private:
    QHash<Breakpoint, std::unique_ptr<IUiComponent>> m_layouts;
    QHash<Breakpoint, int> m_breakpointWidths;
    Breakpoint m_currentBreakpoint = Breakpoint::Medium;
    
    void updateBreakpoint(int width);
};
```

### Adaptive Layout

```cpp
class AdaptiveGrid : public UiGrid {
public:
    // Column adaptation
    void setAdaptiveColumns(int minColumns, int maxColumns);
    void setColumnBreakpoints(const QVector<int>& breakpoints);
    void setItemMinimumWidth(int width);
    
    // Responsive behavior
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

## Layout Debugging

### Debug Visualization

```cpp
class LayoutDebugger {
public:
    // Debug rendering
    void setDebugMode(bool enabled);
    void setShowBounds(bool show);
    void setShowMargins(bool show);
    void setShowSpacing(bool show);
    void setShowConstraints(bool show);
    
    // Debug information
    QString getLayoutInfo(IUiComponent* component) const;
    void dumpLayoutTree(IUiComponent* root) const;
    
    // Visual debugging
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

### Performance Metrics

```cpp
class LayoutProfiler {
public:
    struct LayoutMetrics {
        int layoutTime;     // Time spent in layout calculation (microseconds)
        int componentCount; // Number of components laid out
        int depth;          // Layout tree depth
        int iterations;     // Number of layout passes
        QStringList bottlenecks; // Components taking most time
    };
    
    // Profiling control
    void startProfiling();
    void stopProfiling();
    LayoutMetrics getMetrics() const;
    
    // Performance analysis
    void identifyBottlenecks();
    void suggestOptimizations();
    
private:
    QElapsedTimer m_timer;
    LayoutMetrics m_metrics;
    QHash<IUiComponent*, qint64> m_componentTimes;
};
```

## Layout Best Practices

### Performance Optimization

1. **Minimize Layout Passes**: Design layouts to minimize the number of constraint-solving iterations
2. **Cache Size Hints**: Cache expensive size calculations when possible
3. **Use Fixed Sizes**: Prefer fixed sizes for components that don't need to resize
4. **Lazy Layout**: Only perform layout when visible or when size changes

### Memory Management

```cpp
class LayoutCache {
public:
    // Size hint caching
    void cacheSizeHint(IUiComponent* component, const QSize& hint);
    QSize getCachedSizeHint(IUiComponent* component) const;
    void invalidateCache(IUiComponent* component);
    
    // Layout result caching
    void cacheLayout(IUiComponent* container, const QHash<IUiComponent*, QRect>& layout);
    bool hasValidCache(IUiComponent* container) const;
    
private:
    QHash<IUiComponent*, QSize> m_sizeHintCache;
    QHash<IUiComponent*, QHash<IUiComponent*, QRect>> m_layoutCache;
    QHash<IUiComponent*, qint64> m_cacheTimestamps;
};
```

### Common Patterns

#### Two-Panel Layout
```cpp
auto createTwoPanelLayout(std::unique_ptr<IUiComponent> left, 
                         std::unique_ptr<IUiComponent> right,
                         int leftWidth = 300) {
    auto container = std::make_unique<UiContainer>();
    
    // Left panel - fixed width
    UiContainer::ContainerItem leftItem;
    leftItem.component = std::move(left);
    leftItem.constraints.leftMargin = 0;
    leftItem.constraints.topMargin = 0;
    leftItem.constraints.bottomMargin = 0;
    leftItem.size = QSize(leftWidth, -1); // Auto height
    container->addItem(std::move(leftItem));
    
    // Right panel - remaining width
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

#### Card Layout
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

## Related Documentation

- [UI Framework Overview](overview.md)
- [Theme & Rendering](theme-and-rendering.md)
- [Component Documentation](../components/)