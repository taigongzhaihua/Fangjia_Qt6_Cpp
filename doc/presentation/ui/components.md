**English** | [简体中文](../../../doc.zh-cn/presentation/ui/components.md)

# UI Components & Containers

This document introduces the core UI components in the Fangjia Qt6 C++ framework, including basic containers (UiPanel, UiBoxLayout, UiGrid) and scroll containers (UiScrollView), along with their configuration options and usage patterns.

## Container Components Overview

### UiPanel - Basic Panel Container

`UiPanel` is the most basic container component, providing simple child component stacking functionality.

```cpp
auto panel = UI::panel()
    ->children({
        UI::label()->text("Header"),
        UI::button()->text("Action"),
        UI::label()->text("Footer")
    })
    ->padding(16)
    ->spacing(8);
```

#### Key Features
- **Simple Layout**: Children are stacked vertically by default
- **Padding Control**: Configurable internal spacing
- **Background Support**: Optional background color and border radius
- **Flexible Sizing**: Adapts to content or explicit size constraints

#### Configuration Options

```cpp
class UiPanel : public Widget {
public:
    // Content management
    UiPanel* children(const std::vector<WidgetPtr>& children);
    UiPanel* addChild(WidgetPtr child);
    UiPanel* removeChild(WidgetPtr child);
    
    // Layout configuration
    UiPanel* padding(int padding);                    // All sides
    UiPanel* padding(int horizontal, int vertical);   // Horizontal/vertical
    UiPanel* padding(int top, int right, int bottom, int left);  // Individual sides
    UiPanel* spacing(int spacing);                    // Between children
    
    // Visual styling
    UiPanel* backgroundColor(const QColor& color);
    UiPanel* borderRadius(float radius);
    UiPanel* borderWidth(float width);
    UiPanel* borderColor(const QColor& color);
    
    // Size constraints
    UiPanel* minSize(const QSize& size);
    UiPanel* maxSize(const QSize& size);
    UiPanel* fixedSize(const QSize& size);
};
```

### UiGrid - Grid Layout Container

`UiGrid` provides flexible grid-based layout with configurable columns and rows.

```cpp
auto grid = UI::grid()
    ->columns({
        UI::GridTrack::fixed(100),    // Fixed 100px column
        UI::GridTrack::flex(1),       // Flexible column (takes remaining space)
        UI::GridTrack::auto()         // Auto-sized column (content-based)
    })
    ->children({
        UI::label()->text("Label 1"),     // (0,0)
        UI::textField()->placeholder("Input 1"),  // (0,1)
        UI::button()->text("Action 1"),   // (0,2)
        
        UI::label()->text("Label 2"),     // (1,0)
        UI::textField()->placeholder("Input 2"),  // (1,1)
        UI::button()->text("Action 2")    // (1,2)
    })
    ->gap(8);
```

#### Grid Track Types

```cpp
namespace UI {
    class GridTrack {
    public:
        static GridTrack fixed(int pixels);              // Fixed pixel size
        static GridTrack flex(float factor);             // Proportional size
        static GridTrack auto();                         // Content-based size
        static GridTrack minMax(int minPx, int maxPx);   // Size with constraints
        static GridTrack fractional(float fraction);     // Fraction of available space
    };
}
```

#### Advanced Grid Configuration

```cpp
auto advancedGrid = UI::grid()
    ->columns({
        UI::GridTrack::fixed(80),      // Icon column
        UI::GridTrack::flex(2),        // Main content (2/3 of flexible space)
        UI::GridTrack::flex(1),        // Secondary content (1/3 of flexible space)
        UI::GridTrack::fixed(120)      // Action buttons
    })
    ->rows({
        UI::GridTrack::auto(),         // Header row (content-based)
        UI::GridTrack::flex(1),        // Content row (takes remaining space)
        UI::GridTrack::fixed(48)       // Footer row (fixed height)
    })
    ->gap(12)                          // 12px gap between all cells
    ->columnGap(16)                    // Override column gap
    ->rowGap(8)                        // Override row gap
    ->alignment(GridAlignment::Stretch) // How children fill their cells
    ->justifyContent(GridJustify::Start); // How grid content is justified
```

### UiScrollView - Scrollable Container

`UiScrollView` provides efficient scrolling for content that exceeds the visible area.

```cpp
auto scrollView = UI::scrollView(
    UI::panel()
        ->children(createLargeContentList())
        ->spacing(8)
)
->scrollBarPolicy(ScrollBarPolicy::AsNeeded)
->horizontalScrollEnabled(false)
->verticalScrollEnabled(true)
->contentMargins(16);
```

#### Scroll Configuration Options

```cpp
enum class ScrollBarPolicy {
    AlwaysOff,      // Never show scroll bars
    AlwaysOn,       // Always show scroll bars
    AsNeeded        // Show scroll bars when content overflows
};

enum class ScrollMode {
    Pixel,          // Pixel-based scrolling
    Item,           // Item-based scrolling (for lists)
    Page            // Page-based scrolling
};

class UiScrollView : public Widget {
public:
    // Content management
    UiScrollView* content(WidgetPtr content);
    
    // Scroll behavior
    UiScrollView* horizontalScrollEnabled(bool enabled);
    UiScrollView* verticalScrollEnabled(bool enabled);
    UiScrollView* scrollBarPolicy(ScrollBarPolicy policy);
    UiScrollView* scrollMode(ScrollMode mode);
    
    // Visual configuration
    UiScrollView* contentMargins(int margin);
    UiScrollView* contentMargins(int horizontal, int vertical);
    UiScrollView* scrollBarWidth(int width);
    UiScrollView* scrollBarColor(const QColor& color);
    
    // Performance optimization
    UiScrollView* virtualScrolling(bool enabled);    // For large lists
    UiScrollView* itemHeight(int height);           // For virtual scrolling
    UiScrollView* visibleItems(int count);          // Visible item count hint
    
    // Scroll control
    void scrollTo(const QPoint& position);
    void scrollToItem(int index);                   // For item-based scrolling
    void scrollBy(const QPoint& delta);
    
    // Event handling
    UiScrollView* onScrolled(std::function<void(const QPoint&)> callback);
    UiScrollView* onScrollStart(std::function<void()> callback);
    UiScrollView* onScrollEnd(std::function<void()> callback);
};
```

## Layout Patterns & Best Practices

### Responsive Grid Layouts

```cpp
auto createResponsiveGrid() {
    return UI::bindingHost([this]() -> WidgetPtr {
        auto windowSize = getCurrentWindowSize();
        
        // Adapt grid based on available width
        std::vector<UI::GridTrack> columns;
        if (windowSize.width() > 1200) {
            // Wide layout: 4 columns
            columns = {
                UI::GridTrack::flex(1),
                UI::GridTrack::flex(1),
                UI::GridTrack::flex(1),
                UI::GridTrack::flex(1)
            };
        } else if (windowSize.width() > 768) {
            // Medium layout: 2 columns
            columns = {
                UI::GridTrack::flex(1),
                UI::GridTrack::flex(1)
            };
        } else {
            // Narrow layout: 1 column
            columns = { UI::GridTrack::flex(1) };
        }
        
        return UI::grid()
            ->columns(columns)
            ->children(createGridItems())
            ->gap(16);
    });
}
```

### Efficient List Rendering

```cpp
class VirtualListView : public UiScrollView {
public:
    VirtualListView(IListDataProvider* dataProvider) 
        : m_dataProvider(dataProvider) {
        
        setupVirtualScrolling();
        connectDataSignals();
    }
    
protected:
    WidgetPtr buildContent() override {
        auto visibleRange = calculateVisibleRange();
        std::vector<WidgetPtr> visibleItems;
        
        for (int i = visibleRange.start; i < visibleRange.end; ++i) {
            auto item = createListItem(i);
            visibleItems.push_back(item);
        }
        
        return UI::panel()
            ->children(visibleItems)
            ->spacing(0);
    }
    
private:
    struct VisibleRange {
        int start;
        int end;
    };
    
    VisibleRange calculateVisibleRange() {
        int itemHeight = m_dataProvider->itemHeight();
        int visibleHeight = getVisibleHeight();
        int scrollTop = getScrollPosition().y();
        
        int startIndex = std::max(0, scrollTop / itemHeight);
        int endIndex = std::min(m_dataProvider->itemCount(), 
                               startIndex + (visibleHeight / itemHeight) + 2);
        
        return {startIndex, endIndex};
    }
    
    WidgetPtr createListItem(int index) {
        auto itemData = m_dataProvider->itemData(index);
        
        return UI::panel()
            ->children({
                UI::label()->text(itemData.title),
                UI::label()->text(itemData.subtitle)
            })
            ->padding(16)
            ->fixedHeight(m_dataProvider->itemHeight());
    }
    
    void setupVirtualScrolling() {
        virtualScrolling(true);
        itemHeight(m_dataProvider->itemHeight());
        
        onScrolled([this](const QPoint&) {
            requestRebuild();  // Rebuild visible items
        });
    }
    
    void connectDataSignals() {
        connect(m_dataProvider, &IListDataProvider::dataChanged,
                this, [this]() { requestRebuild(); });
    }
    
    IListDataProvider* m_dataProvider;
};
```

### Complex Form Layouts

```cpp
auto createFormLayout() {
    return UI::grid()
        ->columns({
            UI::GridTrack::fixed(120),     // Label column
            UI::GridTrack::flex(2),        // Input column (main)
            UI::GridTrack::flex(1)         // Helper text column
        })
        ->children({
            // Row 1: Text input
            UI::label()->text("Name:"),
            UI::textField()->placeholder("Enter your name"),
            UI::label()->text("Required").color(Qt::red),
            
            // Row 2: Email input
            UI::label()->text("Email:"),
            UI::textField()->placeholder("Enter your email"),
            UI::label()->text("We'll never share your email"),
            
            // Row 3: Password input
            UI::label()->text("Password:"),
            UI::textField()->placeholder("Enter password")->password(true),
            UI::label()->text("At least 8 characters"),
            
            // Row 4: Checkbox (spans all columns)
            UI::spacer(),  // Empty label cell
            UI::checkBox()->text("I agree to the terms and conditions"),
            UI::spacer(),  // Empty helper cell
            
            // Row 5: Submit button (spans all columns)
            UI::spacer(),  // Empty label cell
            UI::button()->text("Create Account")->primary(true),
            UI::spacer()   // Empty helper cell
        })
        ->gap(12)
        ->alignment(GridAlignment::Start);
}
```

## Performance Optimization

### Container Reuse

```cpp
class ContainerPool {
public:
    WidgetPtr acquirePanel() {
        if (m_panelPool.empty()) {
            return UI::panel();
        }
        
        auto panel = m_panelPool.back();
        m_panelPool.pop_back();
        
        // Reset to default state
        panel->children({});
        panel->padding(0);
        panel->backgroundColor(QColor());
        
        return panel;
    }
    
    void releasePanel(WidgetPtr panel) {
        if (m_panelPool.size() < MAX_POOL_SIZE) {
            m_panelPool.push_back(panel);
        }
    }
    
private:
    static constexpr int MAX_POOL_SIZE = 100;
    std::vector<WidgetPtr> m_panelPool;
};
```

### Lazy Content Loading

```cpp
class LazyContentPanel : public UiPanel {
public:
    LazyContentPanel(std::function<WidgetPtr()> contentFactory)
        : m_contentFactory(std::move(contentFactory)) {}
    
protected:
    void onFirstRender() override {
        if (!m_contentLoaded) {
            auto content = m_contentFactory();
            addChild(content);
            m_contentLoaded = true;
        }
    }
    
private:
    std::function<WidgetPtr()> m_contentFactory;
    bool m_contentLoaded = false;
};

// Usage
auto lazyPanel = std::make_shared<LazyContentPanel>([]() {
    // This expensive content creation is deferred until first render
    return createExpensiveContent();
});
```

### Memory-Efficient Scrolling

```cpp
class RecyclingScrollView : public UiScrollView {
public:
    RecyclingScrollView(IListDataProvider* dataProvider, int itemHeight)
        : m_dataProvider(dataProvider), m_itemHeight(itemHeight) {
        
        setupRecycling();
    }
    
protected:
    WidgetPtr buildContent() override {
        auto visibleRange = calculateVisibleRange();
        auto visibleItems = acquireItemsForRange(visibleRange);
        
        // Return items that are no longer visible to the pool
        returnUnusedItems(visibleRange);
        
        return UI::panel()
            ->children(visibleItems)
            ->spacing(0);
    }
    
private:
    std::vector<WidgetPtr> acquireItemsForRange(const VisibleRange& range) {
        std::vector<WidgetPtr> items;
        
        for (int i = range.start; i < range.end; ++i) {
            WidgetPtr item;
            
            if (!m_itemPool.empty()) {
                // Reuse existing item
                item = m_itemPool.back();
                m_itemPool.pop_back();
            } else {
                // Create new item
                item = createNewItem();
            }
            
            // Update item content
            updateItemContent(item, i);
            items.push_back(item);
        }
        
        return items;
    }
    
    void returnUnusedItems(const VisibleRange& currentRange) {
        // Return items that are no longer in the visible range
        for (auto& item : m_currentItems) {
            int itemIndex = getItemIndex(item);
            if (itemIndex < currentRange.start || itemIndex >= currentRange.end) {
                m_itemPool.push_back(item);
            }
        }
    }
    
    IListDataProvider* m_dataProvider;
    int m_itemHeight;
    std::vector<WidgetPtr> m_itemPool;
    std::vector<WidgetPtr> m_currentItems;
};
```

## Related Documentation

- [Presentation Architecture Overview](../architecture.md) - How containers integrate with the overall UI system
- [Layout System](../ui-framework/layouts.md) - Detailed layout algorithms and positioning
- [Binding & Reactive Rebuild](../binding.md) - Reactive data binding with containers