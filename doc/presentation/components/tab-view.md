**English** | [简体中文](../../../doc.zh-cn/presentation/components/tab-view.md)

# TabView Component

## Overview

The TabView component provides a tabbed interface supporting multiple content panels with tab management, closeable tabs, overflow handling, and smooth transitions. It integrates seamlessly with the application's navigation system and supports both static and dynamic tab configurations.

## Core Features

- **Multi-Tab Interface**: Support for multiple content panels with tab headers
- **Closeable Tabs**: Optional close buttons with confirmation dialogs
- **Tab Overflow**: Automatic scrolling for tabs that exceed available width
- **Dynamic Management**: Add, remove, and reorder tabs programmatically
- **Content Transitions**: Smooth animations when switching between tabs
- **Keyboard Navigation**: Full keyboard navigation support (Ctrl+Tab, Ctrl+W, etc.)

## Basic Usage

### Simple Tab Configuration

```cpp
auto tabView = std::make_unique<UiTabView>();

// Configure tab bar appearance
tabView->setTabHeight(40);
tabView->setTabMinWidth(120);
tabView->setTabMaxWidth(250);
tabView->setTabSpacing(2);

// Add tabs with content
tabView->addTab({
    .id = "editor",
    .title = "Text Editor",
    .iconPath = ":/icons/edit.svg",
    .content = std::make_unique<TextEditorComponent>(),
    .isClosable = true
});

tabView->addTab({
    .id = "preview",
    .title = "Preview",
    .iconPath = ":/icons/eye.svg", 
    .content = std::make_unique<PreviewComponent>(),
    .isClosable = false
});

// Set initial active tab
tabView->setActiveTab("editor");
```

### Event Handling

```cpp
// Handle tab selection
tabView->onTabClick = [](const QString& tabId) {
    qDebug() << "Tab clicked:" << tabId;
    // Additional click handling if needed
};

// Handle tab closure
tabView->onTabClose = [](const QString& tabId) {
    qDebug() << "Tab close requested:" << tabId;
    // Perform cleanup, save changes, etc.
    return true; // Return false to prevent closing
};

// Handle active tab changes
tabView->onActiveChanged = [](const QString& tabId) {
    qDebug() << "Active tab changed to:" << tabId;
    // Update application state, breadcrumbs, etc.
};
```

## Tab Structure

### Tab Definition

```cpp
struct UiTabView::Tab {
    QString id;                                    // Unique identifier
    QString title;                                 // Display title
    QString iconPath;                              // Optional icon
    std::unique_ptr<IUiComponent> content;         // Tab content component
    bool isClosable = true;                        // Show close button
    bool isModified = false;                       // Show modified indicator
    bool isPinned = false;                         // Prevent reordering/closing
    QVariantMap metadata;                          // Custom data
    
    // Visual state
    QColor customColor;                            // Custom tab color
    QString tooltip;                               // Tab tooltip
    bool isEnabled = true;                         // Interactive state
};
```

### Tab States

```cpp
enum class TabState {
    Normal,      // Default state
    Active,      // Currently selected
    Hovered,     // Mouse hover
    Pressed,     // Mouse pressed
    Dragging,    // Being dragged for reordering
    Closing,     // Playing close animation
    Modified,    // Content has unsaved changes
    Disabled     // Non-interactive
};
```

## Advanced Configuration

### Tab Bar Styling

```cpp
class UiTabView {
public:
    // Tab dimensions
    void setTabHeight(int height);
    void setTabMinWidth(int minWidth);
    void setTabMaxWidth(int maxWidth);
    void setTabSpacing(int spacing);
    void setTabPadding(const QMargins& padding);
    
    // Tab bar layout
    void setTabBarPosition(TabBarPosition position); // Top, Bottom, Left, Right
    void setTabBarBackground(const QColor& color);
    void setTabBarBorder(const QColor& color, int width);
    void setTabBarScrollable(bool scrollable);
    
    // Tab appearance
    void setTabCornerRadius(float radius);
    void setTabBorderWidth(int width);
    void setShowTabIcons(bool show);
    void setShowCloseButtons(bool show);
    void setTabCloseButtonPosition(CloseButtonPosition position);
    
    // Typography
    void setTabFont(const QFont& font);
    void setTabActiveFont(const QFont& font);
    void setTabMaxTitleLength(int maxLength);
    void setTabTitleEliding(Qt::TextElideMode mode);
    
private:
    int m_tabHeight = 40;
    int m_tabMinWidth = 100;
    int m_tabMaxWidth = 200;
    int m_tabSpacing = 2;
    QMargins m_tabPadding{12, 8, 12, 8};
};
```

### Color Palette

```cpp
struct UiTabView::Palette {
    // Tab colors
    QColor tabBackground;
    QColor tabActiveBackground;
    QColor tabHoverBackground;
    QColor tabPressedBackground;
    QColor tabDisabledBackground;
    
    // Text colors
    QColor tabText;
    QColor tabActiveText;
    QColor tabHoverText;
    QColor tabDisabledText;
    
    // Border colors
    QColor tabBorder;
    QColor tabActiveBorder;
    QColor tabHoverBorder;
    
    // Content area
    QColor contentBackground;
    QColor contentBorder;
    
    // Close button
    QColor closeButtonColor;
    QColor closeButtonHoverColor;
    QColor closeButtonPressedColor;
    
    // Modified indicator
    QColor modifiedIndicatorColor;
    
    // Scroll buttons
    QColor scrollButtonColor;
    QColor scrollButtonHoverColor;
    QColor scrollButtonDisabledColor;
};
```

## Tab Management

### Dynamic Tab Operations

```cpp
class UiTabView {
public:
    // Tab CRUD operations
    void addTab(const Tab& tab);
    void insertTab(int index, const Tab& tab);
    void removeTab(const QString& id);
    void removeTabAt(int index);
    void moveTab(const QString& id, int newIndex);
    void moveTab(int fromIndex, int toIndex);
    
    // Tab state management
    void setActiveTab(const QString& id);
    void setActiveTabAt(int index);
    QString activeTab() const;
    int activeTabIndex() const;
    
    // Tab queries
    bool hasTab(const QString& id) const;
    int tabCount() const;
    QStringList tabIds() const;
    Tab* findTab(const QString& id);
    Tab* tabAt(int index);
    
    // Tab modification state
    void setTabModified(const QString& id, bool modified);
    bool isTabModified(const QString& id) const;
    QStringList modifiedTabs() const;
    
    // Tab visibility
    void setTabVisible(const QString& id, bool visible);
    void setTabEnabled(const QString& id, bool enabled);
    
private:
    std::vector<Tab> m_tabs;
    QString m_activeTabId;
    QHash<QString, int> m_tabIndexCache;
};
```

### Tab Persistence

```cpp
class TabViewState {
public:
    // State serialization
    QJsonObject saveState() const;
    void restoreState(const QJsonObject& state);
    
    // Session management
    void saveSession(const QString& sessionName);
    void restoreSession(const QString& sessionName);
    QStringList availableSessions() const;
    
private:
    struct TabState {
        QString id;
        QString title;
        QString iconPath;
        bool isActive;
        bool isModified;
        bool isPinned;
        QVariantMap metadata;
    };
    
    QVector<TabState> m_tabStates;
    QString m_activeTabId;
};
```

## Content Management

### Content Area

The content area displays the active tab's content component:

```cpp
class UiTabView {
private:
    std::unique_ptr<IUiComponent> m_currentContent;
    QHash<QString, std::unique_ptr<IUiComponent>> m_contentCache;
    
    void switchToTab(const QString& tabId) {
        // Find target tab
        auto tab = findTab(tabId);
        if (!tab) return;
        
        // Cache current content if needed
        if (m_currentContent && m_contentCaching) {
            m_contentCache[m_activeTabId] = std::move(m_currentContent);
        }
        
        // Load new content
        if (m_contentCache.contains(tabId)) {
            m_currentContent = std::move(m_contentCache[tabId]);
            m_contentCache.remove(tabId);
        } else {
            m_currentContent = std::move(tab->content);
        }
        
        // Update layout and visibility
        updateContentLayout();
        
        // Trigger transition animation if enabled
        if (m_transitionsEnabled) {
            startContentTransition(m_activeTabId, tabId);
        }
        
        m_activeTabId = tabId;
        emit activeTabChanged(tabId);
    }
};
```

### Content Transitions

```cpp
class UiTabView {
public:
    enum class TransitionType {
        None,        // Instant switch
        Fade,        // Cross-fade transition
        Slide,       // Slide transition
        Zoom,        // Zoom transition
        Custom       // Custom transition callback
    };
    
    // Transition configuration
    void setTransitionType(TransitionType type);
    void setTransitionDuration(int ms);
    void setTransitionEasing(QEasingCurve::Type easing);
    void setCustomTransition(std::function<void(IUiComponent*, IUiComponent*)> transition);
    
private:
    void startContentTransition(const QString& fromTabId, const QString& toTabId) {
        if (m_transitionType == TransitionType::None) return;
        
        switch (m_transitionType) {
        case TransitionType::Fade:
            startFadeTransition();
            break;
        case TransitionType::Slide:
            startSlideTransition();
            break;
        case TransitionType::Zoom:
            startZoomTransition();
            break;
        case TransitionType::Custom:
            if (m_customTransition) {
                m_customTransition(m_previousContent.get(), m_currentContent.get());
            }
            break;
        }
    }
    
    TransitionType m_transitionType = TransitionType::Fade;
    int m_transitionDuration = 200;
    QEasingCurve::Type m_transitionEasing = QEasingCurve::OutCubic;
};
```

## Tab Overflow & Scrolling

### Overflow Handling

When tabs exceed available width, the TabView automatically enables scrolling:

```cpp
class UiTabView {
private:
    struct ScrollState {
        bool isOverflowing = false;
        int scrollOffset = 0;
        int maxScrollOffset = 0;
        int visibleTabCount = 0;
        int firstVisibleTab = 0;
        int lastVisibleTab = 0;
    } m_scrollState;
    
    void updateScrollState() {
        int totalTabWidth = calculateTotalTabWidth();
        int availableWidth = m_tabBarBounds.width() - m_scrollButtonWidth * 2;
        
        m_scrollState.isOverflowing = totalTabWidth > availableWidth;
        
        if (m_scrollState.isOverflowing) {
            m_scrollState.maxScrollOffset = totalTabWidth - availableWidth;
            updateVisibleTabs();
        } else {
            m_scrollState.scrollOffset = 0;
            m_scrollState.firstVisibleTab = 0;
            m_scrollState.lastVisibleTab = m_tabs.size() - 1;
        }
    }
    
    void scrollToTab(const QString& tabId) {
        int tabIndex = findTabIndex(tabId);
        if (tabIndex < 0) return;
        
        // Calculate required scroll to make tab visible
        int tabPosition = calculateTabPosition(tabIndex);
        int tabWidth = calculateTabWidth(tabIndex);
        
        if (tabPosition < m_scrollState.scrollOffset) {
            // Scroll left to show tab
            scrollTo(tabPosition);
        } else if (tabPosition + tabWidth > m_scrollState.scrollOffset + getVisibleWidth()) {
            // Scroll right to show tab
            scrollTo(tabPosition + tabWidth - getVisibleWidth());
        }
    }
};
```

### Scroll Controls

```cpp
class UiTabView {
public:
    // Scroll configuration
    void setScrollButtonSize(int size);
    void setScrollButtonStyle(ScrollButtonStyle style);
    void setScrollAnimationEnabled(bool enabled);
    void setScrollWheelEnabled(bool enabled);
    
    // Scroll control
    void scrollLeft();
    void scrollRight();
    void scrollToBeginning();
    void scrollToEnd();
    void scrollToTab(const QString& tabId);
    
private:
    Ui::Button m_scrollLeftButton;
    Ui::Button m_scrollRightButton;
    int m_scrollButtonWidth = 24;
    QPropertyAnimation* m_scrollAnimation = nullptr;
    
    void createScrollButtons() {
        m_scrollLeftButton.setText("<");
        m_scrollLeftButton.onClick = [this]() { scrollLeft(); };
        
        m_scrollRightButton.setText(">");
        m_scrollRightButton.onClick = [this]() { scrollRight(); };
    }
};
```

## Keyboard Navigation

### Keyboard Shortcuts

```cpp
class UiTabView {
public:
    bool onKeyPress(QKeyEvent* event) override {
        // Handle keyboard navigation
        if (event->modifiers() & Qt::ControlModifier) {
            switch (event->key()) {
            case Qt::Key_Tab:
                // Ctrl+Tab: Next tab
                cycleTab(1);
                return true;
                
            case Qt::Key_Backtab:
                // Ctrl+Shift+Tab: Previous tab
                cycleTab(-1);
                return true;
                
            case Qt::Key_W:
                // Ctrl+W: Close current tab
                closeCurrentTab();
                return true;
                
            case Qt::Key_T:
                // Ctrl+T: New tab
                emit newTabRequested();
                return true;
                
            case Qt::Key_1: case Qt::Key_2: case Qt::Key_3:
            case Qt::Key_4: case Qt::Key_5: case Qt::Key_6:
            case Qt::Key_7: case Qt::Key_8: case Qt::Key_9:
                // Ctrl+1-9: Switch to specific tab
                int tabIndex = event->key() - Qt::Key_1;
                if (tabIndex < m_tabs.size()) {
                    setActiveTabAt(tabIndex);
                }
                return true;
            }
        }
        
        return IUiComponent::onKeyPress(event);
    }
    
private:
    void cycleTab(int direction) {
        int currentIndex = activeTabIndex();
        int newIndex = (currentIndex + direction + m_tabs.size()) % m_tabs.size();
        setActiveTabAt(newIndex);
    }
};
```

## Theme Integration

### Theme-Aware Styling

```cpp
void UiTabView::onThemeChanged(bool isDark) {
    // Update palette for current theme
    if (isDark) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
    
    // Update all tab visuals
    updateTabAppearance();
    
    // Update content area styling
    updateContentStyling();
    
    // Refresh icon cache with new theme
    invalidateIconCache();
    
    // Request visual update
    requestRepaint();
}

void UiTabView::applyDarkTheme() {
    m_palette.tabBackground = QColor(52, 58, 64);
    m_palette.tabActiveBackground = QColor(73, 80, 87);
    m_palette.tabText = QColor(248, 249, 250);
    m_palette.tabActiveText = QColor(255, 255, 255);
    m_palette.contentBackground = QColor(33, 37, 41);
    m_palette.contentBorder = QColor(73, 80, 87);
}
```

## Performance Considerations

### Efficient Rendering

- **Viewport Culling**: Only visible tabs are rendered in detail
- **Content Lazy Loading**: Tab content is created on first access
- **Icon Caching**: Tab icons are cached and reused
- **Animation Optimization**: Hardware acceleration when available

### Memory Management

```cpp
class UiTabView {
private:
    // Content caching strategy
    enum class ContentCaching {
        None,           // No caching, recreate content each time
        LRU,            // Least recently used caching
        All,            // Cache all tab content
        Manual          // Manual cache management
    };
    
    ContentCaching m_contentCaching = ContentCaching::LRU;
    int m_maxCachedContent = 5;
    
    void manageContentCache() {
        if (m_contentCache.size() <= m_maxCachedContent) return;
        
        // Remove least recently used content
        QStringList sortedTabs = m_tabAccessOrder;
        while (m_contentCache.size() > m_maxCachedContent && !sortedTabs.isEmpty()) {
            QString oldestTab = sortedTabs.takeFirst();
            m_contentCache.remove(oldestTab);
        }
    }
    
    QHash<QString, std::unique_ptr<IUiComponent>> m_contentCache;
    QStringList m_tabAccessOrder; // For LRU tracking
};
```

## Related Documentation

- [UI Framework Overview](../ui-framework/overview.md)
- [Layout System](../ui-framework/layouts.md)
- [Theme System](../ui-framework/theme-and-rendering.md)
- [NavRail Component](nav-rail.md)
- [ScrollView Component](scroll-view.md)