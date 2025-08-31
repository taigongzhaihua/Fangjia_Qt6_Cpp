[English](../../../doc/presentation/components/tab-view.md) | **简体中文**

# TabView 组件

## 概述

TabView 组件提供标签界面，支持多个内容面板的标签管理、可关闭标签、溢出处理以及平滑过渡效果。它与应用程序的导航系统无缝集成，支持静态和动态标签配置。

## 核心特性

- **多标签界面**: 支持多个内容面板与标签头
- **可关闭标签**: 可选的关闭按钮与确认对话框
- **标签溢出**: 标签超出可用宽度时自动滚动
- **动态管理**: 程序化添加、删除和重新排序标签
- **内容过渡**: 在标签间切换时的平滑动画
- **键盘导航**: 完整的键盘导航支持（Ctrl+Tab、Ctrl+W 等）

## 基本用法

### 简单标签配置

```cpp
auto tabView = std::make_unique<UiTabView>();

// 配置标签栏外观
tabView->setTabHeight(40);
tabView->setTabMinWidth(120);
tabView->setTabMaxWidth(250);
tabView->setTabSpacing(2);

// 添加带内容的标签
tabView->addTab({
    .id = "editor",
    .title = "文本编辑器",
    .iconPath = ":/icons/edit.svg",
    .content = std::make_unique<TextEditorComponent>(),
    .isClosable = true
});

tabView->addTab({
    .id = "preview",
    .title = "预览",
    .iconPath = ":/icons/eye.svg", 
    .content = std::make_unique<PreviewComponent>(),
    .isClosable = false
});

// 设置初始激活标签
tabView->setActiveTab("editor");
```

### 事件处理

```cpp
// 处理标签选择
tabView->onTabClick = [](const QString& tabId) {
    qDebug() << "标签被点击:" << tabId;
    // 如需要，添加额外的点击处理
};

// 处理标签关闭
tabView->onTabClose = [](const QString& tabId) {
    qDebug() << "标签关闭请求:" << tabId;
    // 执行清理、保存更改等
    return true; // 返回 false 阻止关闭
};

// 处理活动标签变化
tabView->onActiveChanged = [](const QString& tabId) {
    qDebug() << "活动标签更改为:" << tabId;
    // 更新应用程序状态、面包屑等
};
```

## 标签结构

### 标签定义

```cpp
struct UiTabView::Tab {
    QString id;                                    // 唯一标识符
    QString title;                                 // 显示标题
    QString iconPath;                              // 可选图标
    std::unique_ptr<IUiComponent> content;         // 标签内容组件
    bool isClosable = true;                        // 显示关闭按钮
    bool isModified = false;                       // 显示修改指示器
    bool isPinned = false;                         // 防止重新排序/关闭
    QVariantMap metadata;                          // 自定义数据
    
    // 视觉状态
    QColor customColor;                            // 自定义标签颜色
    QString tooltip;                               // 标签工具提示
    bool isEnabled = true;                         // 交互状态
};
```

### 标签状态

```cpp
enum class TabState {
    Normal,      // 默认状态
    Active,      // 当前选中
    Hovered,     // 鼠标悬停
    Pressed,     // 鼠标按下
    Dragging,    // 正在拖拽重新排序
    Closing,     // 播放关闭动画
    Modified,    // 内容有未保存更改
    Disabled     // 非交互状态
};
```

## 高级配置

### 标签栏样式

```cpp
class UiTabView {
public:
    // 标签尺寸
    void setTabHeight(int height);
    void setTabMinWidth(int minWidth);
    void setTabMaxWidth(int maxWidth);
    void setTabSpacing(int spacing);
    void setTabPadding(const QMargins& padding);
    
    // 标签栏布局
    void setTabBarPosition(TabBarPosition position); // 顶部、底部、左侧、右侧
    void setTabBarBackground(const QColor& color);
    void setTabBarBorder(const QColor& color, int width);
    void setTabBarScrollable(bool scrollable);
    
    // 标签外观
    void setTabCornerRadius(float radius);
    void setTabBorderWidth(int width);
    void setShowTabIcons(bool show);
    void setShowCloseButtons(bool show);
    void setTabCloseButtonPosition(CloseButtonPosition position);
    
    // 排版
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

### 颜色调色板

```cpp
struct UiTabView::Palette {
    // 标签颜色
    QColor tabBackground;
    QColor tabActiveBackground;
    QColor tabHoverBackground;
    QColor tabPressedBackground;
    QColor tabDisabledBackground;
    
    // 文本颜色
    QColor tabText;
    QColor tabActiveText;
    QColor tabHoverText;
    QColor tabDisabledText;
    
    // 边框颜色
    QColor tabBorder;
    QColor tabActiveBorder;
    QColor tabHoverBorder;
    
    // 内容区域
    QColor contentBackground;
    QColor contentBorder;
    
    // 关闭按钮
    QColor closeButtonColor;
    QColor closeButtonHoverColor;
    QColor closeButtonPressedColor;
    
    // 修改指示器
    QColor modifiedIndicatorColor;
    
    // 滚动按钮
    QColor scrollButtonColor;
    QColor scrollButtonHoverColor;
    QColor scrollButtonDisabledColor;
};
```

## 标签管理

### 动态标签操作

```cpp
class UiTabView {
public:
    // 标签 CRUD 操作
    void addTab(const Tab& tab);
    void insertTab(int index, const Tab& tab);
    void removeTab(const QString& id);
    void removeTabAt(int index);
    void moveTab(const QString& id, int newIndex);
    void moveTab(int fromIndex, int toIndex);
    
    // 标签状态管理
    void setActiveTab(const QString& id);
    void setActiveTabAt(int index);
    QString activeTab() const;
    int activeTabIndex() const;
    
    // 标签查询
    bool hasTab(const QString& id) const;
    int tabCount() const;
    QStringList tabIds() const;
    Tab* findTab(const QString& id);
    Tab* tabAt(int index);
    
    // 标签修改状态
    void setTabModified(const QString& id, bool modified);
    bool isTabModified(const QString& id) const;
    QStringList modifiedTabs() const;
    
    // 标签可见性
    void setTabVisible(const QString& id, bool visible);
    void setTabEnabled(const QString& id, bool enabled);
    
private:
    std::vector<Tab> m_tabs;
    QString m_activeTabId;
    QHash<QString, int> m_tabIndexCache;
};
```

## 内容管理

### 内容区域

内容区域显示活动标签的内容组件：

```cpp
class UiTabView {
private:
    std::unique_ptr<IUiComponent> m_currentContent;
    QHash<QString, std::unique_ptr<IUiComponent>> m_contentCache;
    
    void switchToTab(const QString& tabId) {
        // 查找目标标签
        auto tab = findTab(tabId);
        if (!tab) return;
        
        // 如需要，缓存当前内容
        if (m_currentContent && m_contentCaching) {
            m_contentCache[m_activeTabId] = std::move(m_currentContent);
        }
        
        // 加载新内容
        if (m_contentCache.contains(tabId)) {
            m_currentContent = std::move(m_contentCache[tabId]);
            m_contentCache.remove(tabId);
        } else {
            m_currentContent = std::move(tab->content);
        }
        
        // 更新布局和可见性
        updateContentLayout();
        
        // 如启用，触发过渡动画
        if (m_transitionsEnabled) {
            startContentTransition(m_activeTabId, tabId);
        }
        
        m_activeTabId = tabId;
        emit activeTabChanged(tabId);
    }
};
```

### 内容过渡

```cpp
class UiTabView {
public:
    enum class TransitionType {
        None,        // 即时切换
        Fade,        // 交叉淡入淡出过渡
        Slide,       // 滑动过渡
        Zoom,        // 缩放过渡
        Custom       // 自定义过渡回调
    };
    
    // 过渡配置
    void setTransitionType(TransitionType type);
    void setTransitionDuration(int ms);
    void setTransitionEasing(QEasingCurve::Type easing);
    void setCustomTransition(std::function<void(IUiComponent*, IUiComponent*)> transition);
    
private:
    TransitionType m_transitionType = TransitionType::Fade;
    int m_transitionDuration = 200;
    QEasingCurve::Type m_transitionEasing = QEasingCurve::OutCubic;
};
```

## 标签溢出与滚动

### 溢出处理

当标签超出可用宽度时，TabView 自动启用滚动：

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
};
```

## 键盘导航

### 键盘快捷键

```cpp
class UiTabView {
public:
    bool onKeyPress(QKeyEvent* event) override {
        // 处理键盘导航
        if (event->modifiers() & Qt::ControlModifier) {
            switch (event->key()) {
            case Qt::Key_Tab:
                // Ctrl+Tab: 下一个标签
                cycleTab(1);
                return true;
                
            case Qt::Key_Backtab:
                // Ctrl+Shift+Tab: 上一个标签
                cycleTab(-1);
                return true;
                
            case Qt::Key_W:
                // Ctrl+W: 关闭当前标签
                closeCurrentTab();
                return true;
                
            case Qt::Key_T:
                // Ctrl+T: 新建标签
                emit newTabRequested();
                return true;
                
            case Qt::Key_1: case Qt::Key_2: case Qt::Key_3:
            case Qt::Key_4: case Qt::Key_5: case Qt::Key_6:
            case Qt::Key_7: case Qt::Key_8: case Qt::Key_9:
                // Ctrl+1-9: 切换到特定标签
                int tabIndex = event->key() - Qt::Key_1;
                if (tabIndex < m_tabs.size()) {
                    setActiveTabAt(tabIndex);
                }
                return true;
            }
        }
        
        return IUiComponent::onKeyPress(event);
    }
};
```

## 主题集成

### 主题感知样式

```cpp
void UiTabView::onThemeChanged(bool isDark) {
    // 为当前主题更新调色板
    if (isDark) {
        applyDarkTheme();
    } else {
        applyLightTheme();
    }
    
    // 更新所有标签视觉效果
    updateTabAppearance();
    
    // 更新内容区域样式
    updateContentStyling();
    
    // 使用新主题刷新图标缓存
    invalidateIconCache();
    
    // 请求视觉更新
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

## 性能考虑

### 高效渲染

- **视口剔除**: 只详细渲染可见标签
- **内容懒加载**: 标签内容在首次访问时创建
- **图标缓存**: 标签图标被缓存和重用
- **动画优化**: 可用时使用硬件加速

### 内存管理

```cpp
class UiTabView {
private:
    // 内容缓存策略
    enum class ContentCaching {
        None,           // 无缓存，每次重新创建内容
        LRU,            // 最近最少使用缓存
        All,            // 缓存所有标签内容
        Manual          // 手动缓存管理
    };
    
    ContentCaching m_contentCaching = ContentCaching::LRU;
    int m_maxCachedContent = 5;
    
    QHash<QString, std::unique_ptr<IUiComponent>> m_contentCache;
    QStringList m_tabAccessOrder; // 用于 LRU 跟踪
};
```

## 相关文档

- [UI 框架概览](../ui-framework/overview.md)
- [布局系统](../ui-framework/layouts.md)
- [主题系统](../ui-framework/theme-and-rendering.md)
- [NavRail 组件](nav-rail.md)
- [ScrollView 组件](scroll-view.md)