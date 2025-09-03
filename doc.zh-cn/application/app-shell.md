[English](../../doc/application/app-shell.md) | **简体中文**

# App Shell 与应用组装

本文档介绍 Fangjia Qt6 C++ 框架中的 AppShell 应用外壳组件，以及导航、TopBar、内容区的拼装与交互协调机制。

## AppShell 概述

### 应用外壳架构

`AppShell` 是应用程序的顶级布局容器，负责组装和协调主要的 UI 区域：

- **TopBar 区域**: 窗口控制、主题切换、系统集成
- **Navigation 区域**: 侧边导航栏，支持收缩/展开
- **Content 区域**: 主要内容显示区域
- **Status 区域**: 可选的状态栏区域

```cpp
auto shell = UI::appShell()
    ->topBar(createTopBar())           // 顶部栏
    ->nav(createNavigation())          // 侧边导航
    ->content(createMainContent())     // 主内容
    ->statusBar(createStatusBar());    // 状态栏（可选）
```

### 布局结构

AppShell 采用 Grid 布局管理各个区域：

```
┌─────────────────────────────────────┐
│              TopBar                 │  ← 固定高度（通常 40-60px）
├──────────┬──────────────────────────┤
│          │                          │
│   Nav    │        Content           │  ← 弹性高度
│  Rail    │                          │
│          │                          │
├──────────┴──────────────────────────┤
│             StatusBar               │  ← 固定高度（可选）
└─────────────────────────────────────┘
```

## AppShell API 设计

### 基础配置

```cpp
class AppShell : public Widget {
public:
    // 设置各个区域的组件
    AppShell* topBar(WidgetPtr topBar);
    AppShell* nav(WidgetPtr navigation);
    AppShell* content(WidgetPtr content);
    AppShell* statusBar(WidgetPtr statusBar);
    
    // 布局配置
    AppShell* topBarHeight(int height);         // TopBar 高度
    AppShell* navWidth(int collapsed, int expanded);  // 导航栏宽度
    AppShell* statusBarHeight(int height);      // 状态栏高度
    
    // 交互配置
    AppShell* navCollapsible(bool collapsible); // 导航栏是否可收缩
    AppShell* navInitialState(bool expanded);   // 导航栏初始状态
    
protected:
    WidgetPtr build() override;
};
```

### 响应式布局

AppShell 支持响应式布局，根据窗口尺寸自动调整：

```cpp
auto createResponsiveShell() {
    return UI::bindingHost([this]() -> WidgetPtr {
        auto windowSize = getWindowSize();
        
        // 根据窗口宽度决定导航栏行为
        bool autoCollapse = windowSize.width() < 1200;
        bool navExpanded = !autoCollapse && m_navExpanded;
        
        return UI::appShell()
            ->topBar(createTopBar())
            ->nav(createNavigation()
                ->expanded(navExpanded)
                ->autoCollapse(autoCollapse)
            )
            ->content(createContent())
            ->navCollapsible(true)
            ->navInitialState(navExpanded);
    });
}
```

## 导航系统集成

### NavRail 集成

```cpp
auto createNavigation() {
    return UI::navRail()
        ->dataProvider(&m_navDataProvider)      // 绑定导航数据
        ->widths(60, 240)                       // 收缩 60px，展开 240px
        ->iconSize(24)                          // 图标尺寸
        ->itemHeight(56)                        // 导航项高度
        ->onItemSelected([this](int index) {   // 选择回调
            navigateToPage(index);
        })
        ->onToggleExpand([this](bool expanded) { // 展开状态变化
            m_navExpanded = expanded;
            requestShellRebuild();
        });
}
```

### 导航数据提供者

```cpp
class AppNavDataProvider : public INavDataProvider {
private:
    struct NavItem {
        QString icon;
        QString label;
        QString route;
        bool enabled = true;
    };
    
    std::vector<NavItem> m_items;
    int m_selectedIndex = 0;
    
public:
    int itemCount() const override { return m_items.size(); }
    
    QString itemIcon(int index) const override {
        return m_items[index].icon;
    }
    
    QString itemLabel(int index) const override {
        return m_items[index].label;
    }
    
    bool itemEnabled(int index) const override {
        return m_items[index].enabled;
    }
    
    int selectedIndex() const override { return m_selectedIndex; }
    
    // 添加导航项
    void addItem(const QString& icon, const QString& label, const QString& route) {
        m_items.push_back({icon, label, route, true});
        notifyDataChanged();
    }
    
    // 设置选中项
    void setSelectedIndex(int index) {
        if (m_selectedIndex != index && index >= 0 && index < m_items.size()) {
            m_selectedIndex = index;
            notifySelectionChanged(index);
        }
    }
};
```

## TopBar 系统集成

### TopBar 配置与交互

```cpp
auto createTopBar() {
    return UI::topBar()
        ->followSystem(m_themeManager->followSystem(), m_animateThemeChange)
        ->cornerRadius(8.0f)
        ->svgTheme(":/icons/theme/sun.svg", ":/icons/theme/moon.svg")
        ->svgFollow(":/icons/theme/auto_on.svg", ":/icons/theme/auto_off.svg")
        ->svgSystem(":/icons/window/minimize.svg", 
                   ":/icons/window/maximize.svg", 
                   ":/icons/window/close.svg")
        ->onThemeToggle([this]() {
            toggleTheme();
        })
        ->onFollowToggle([this]() {
            toggleFollowSystem();
        })
        ->onMinimize([this]() {
            minimizeWindow();
        })
        ->onMaximize([this]() {
            toggleMaximizeWindow();
        })
        ->onClose([this]() {
            closeApplication();
        });
}
```

### 主题系统协调

```cpp
void MainApplication::onThemeToggle() {
    bool currentDark = m_themeManager->isDarkMode();
    m_themeManager->setDarkMode(!currentDark);
    
    // 标记动画意图
    m_animateThemeChange = true;
    
    // 触发 Shell 重建（会传递动画标志）
    requestShellRebuild();
    
    // 重置动画标志
    m_animateThemeChange = false;
}

void MainApplication::onFollowSystemToggle() {
    bool currentFollow = m_themeManager->followSystem();
    
    // 标记动画意图（用于跟随系统动画）
    m_animateFollowChange = true;
    
    // 切换跟随系统状态
    m_themeManager->setFollowSystem(!currentFollow);
    
    // 触发 Shell 重建
    requestShellRebuild();
}
```

## 内容区域管理

### 页面路由系统

```cpp
class PageHost : public IUiContent {
private:
    std::unordered_map<QString, std::unique_ptr<IPage>> m_pages;
    QString m_currentPageRoute;
    IPage* m_currentPage = nullptr;
    
public:
    // 注册页面
    void registerPage(const QString& route, std::unique_ptr<IPage> page) {
        m_pages[route] = std::move(page);
    }
    
    // 导航到页面
    void navigateTo(const QString& route) {
        auto it = m_pages.find(route);
        if (it != m_pages.end() && m_currentPageRoute != route) {
            // 清理当前页面
            if (m_currentPage) {
                m_currentPage->onDeactivated();
            }
            
            // 激活新页面
            m_currentPageRoute = route;
            m_currentPage = it->second.get();
            m_currentPage->onActivated();
            
            // 通知路由变化
            notifyRouteChanged(route);
        }
    }
    
    // IUiContent 实现
    WidgetPtr createContent() override {
        if (m_currentPage) {
            return m_currentPage->createContent();
        }
        return UI::label()->text("未找到页面");
    }
    
    void updateLayout(const QSize& size) override {
        if (m_currentPage) {
            m_currentPage->updateLayout(size);
        }
    }
};
```

### 页面生命周期

```cpp
class IPage {
public:
    virtual ~IPage() = default;
    
    // 页面生命周期
    virtual void onActivated() {}     // 页面激活
    virtual void onDeactivated() {}   // 页面失活
    virtual void onSuspended() {}     // 页面挂起（后台）
    virtual void onResumed() {}       // 页面恢复（前台）
    
    // UI 构建
    virtual WidgetPtr createContent() = 0;
    virtual void updateLayout(const QSize& size) {}
    
    // 数据处理
    virtual void loadData() {}        // 加载数据
    virtual void saveData() {}        // 保存数据
    virtual void refreshData() {}     // 刷新数据
};
```

### 典型页面实现

```cpp
class HomePage : public IPage {
private:
    std::unique_ptr<UI::BindingHost> m_contentHost;
    HomeViewModel* m_viewModel;
    
public:
    HomePage(HomeViewModel* viewModel) : m_viewModel(viewModel) {
        m_contentHost = UI::bindingHost([this]() {
            return createHomeContent();
        });
    }
    
    void onActivated() override {
        m_viewModel->loadHomeData();
    }
    
    WidgetPtr createContent() override {
        return m_contentHost.get();
    }
    
private:
    WidgetPtr createHomeContent() {
        auto stats = m_viewModel->getStatistics();
        auto recentItems = m_viewModel->getRecentItems();
        
        return UI::scrollView(
            UI::panel()
                ->children({
                    createWelcomeSection(),
                    createStatsSection(stats),
                    createRecentSection(recentItems),
                    createQuickActionsSection()
                })
                ->padding(24)
        );
    }
    
    WidgetPtr createStatsSection(const Statistics& stats) {
        return UI::grid()
            ->columns({
                UI::GridTrack::flex(1),
                UI::GridTrack::flex(1),
                UI::GridTrack::flex(1)
            })
            ->children({
                createStatCard("总项目", stats.totalProjects),
                createStatCard("活跃项目", stats.activeProjects),
                createStatCard("完成项目", stats.completedProjects)
            })
            ->gap(16);
    }
};
```

## 状态管理与同步

### 应用状态协调

```cpp
class AppShellViewModel : public INotifyPropertyChanged {
private:
    ThemeManager* m_themeManager;
    NavigationModel* m_navigationModel;
    PageHost* m_pageHost;
    bool m_navExpanded = true;
    
public:
    // 主题状态
    bool isDarkMode() const {
        notifyAccess("isDarkMode");
        return m_themeManager->isDarkMode();
    }
    
    bool followSystem() const {
        notifyAccess("followSystem");
        return m_themeManager->followSystem();
    }
    
    // 导航状态
    bool navExpanded() const {
        notifyAccess("navExpanded");
        return m_navExpanded;
    }
    
    QString currentRoute() const {
        notifyAccess("currentRoute");
        return m_pageHost->currentRoute();
    }
    
    // 状态变更方法
    void toggleTheme() {
        m_themeManager->setDarkMode(!m_themeManager->isDarkMode());
    }
    
    void toggleFollowSystem() {
        m_themeManager->setFollowSystem(!m_themeManager->followSystem());
    }
    
    void setNavExpanded(bool expanded) {
        if (m_navExpanded != expanded) {
            m_navExpanded = expanded;
            notifyChanged("navExpanded");
        }
    }
    
    void navigateToRoute(const QString& route) {
        m_pageHost->navigateTo(route);
        // currentRoute 变化会通过 PageHost 通知
    }
};
```

### Shell 构建与重建

```cpp
auto createAppShell() {
    return UI::bindingHost([this]() -> WidgetPtr {
        auto vm = appShellViewModel();
        
        return UI::appShell()
            ->topBar(UI::topBar()
                ->followSystem(vm->followSystem(), m_animateFollowChange)
                ->onThemeToggle([vm]() { vm->toggleTheme(); })
                ->onFollowToggle([vm]() { vm->toggleFollowSystem(); })
            )
            ->nav(UI::navRail()
                ->dataProvider(&m_navDataProvider)
                ->expanded(vm->navExpanded())
                ->onToggleExpand([vm](bool expanded) {
                    vm->setNavExpanded(expanded);
                })
                ->onItemSelected([vm](int index) {
                    auto route = m_navDataProvider.getRoute(index);
                    vm->navigateToRoute(route);
                })
            )
            ->content(wrap(m_pageHost.get()))
            ->navCollapsible(true);
    });
}
```

## 性能优化策略

### 延迟加载

```cpp
class LazyPageHost : public PageHost {
private:
    std::unordered_map<QString, std::function<std::unique_ptr<IPage>()>> m_pageFactories;
    
public:
    // 注册页面工厂而非页面实例
    void registerPageFactory(const QString& route, 
                            std::function<std::unique_ptr<IPage>()> factory) {
        m_pageFactories[route] = std::move(factory);
    }
    
    void navigateTo(const QString& route) override {
        // 仅在首次访问时创建页面实例
        if (m_pages.find(route) == m_pages.end()) {
            auto factory = m_pageFactories.find(route);
            if (factory != m_pageFactories.end()) {
                m_pages[route] = factory->second();
            }
        }
        
        PageHost::navigateTo(route);
    }
};
```

### 组件缓存

```cpp
class CachedShellBuilder {
private:
    mutable WidgetPtr m_cachedTopBar;
    mutable WidgetPtr m_cachedNav;
    mutable bool m_topBarDirty = true;
    mutable bool m_navDirty = true;
    
public:
    WidgetPtr createAppShell() const {
        return UI::appShell()
            ->topBar(getCachedTopBar())
            ->nav(getCachedNav())
            ->content(wrap(m_pageHost.get()));
    }
    
private:
    WidgetPtr getCachedTopBar() const {
        if (m_topBarDirty || !m_cachedTopBar) {
            m_cachedTopBar = createTopBar();
            m_topBarDirty = false;
        }
        return m_cachedTopBar;
    }
    
    WidgetPtr getCachedNav() const {
        if (m_navDirty || !m_cachedNav) {
            m_cachedNav = createNavigation();
            m_navDirty = false;
        }
        return m_cachedNav;
    }
    
public:
    void invalidateTopBar() { m_topBarDirty = true; }
    void invalidateNav() { m_navDirty = true; }
};
```

## 常见使用模式

### 简单应用外壳

```cpp
auto createBasicShell() {
    return UI::appShell()
        ->topBar(UI::topBar()->followSystem(true))
        ->nav(UI::navRail()->widths(60, 200))
        ->content(UI::label()->text("主要内容区域"));
}
```

### 带状态栏的完整外壳

```cpp
auto createFullShell() {
    return UI::appShell()
        ->topBar(createCustomTopBar())
        ->nav(createCustomNavigation())
        ->content(createPageContent())
        ->statusBar(createStatusBar())
        ->topBarHeight(50)
        ->navWidth(64, 240)
        ->statusBarHeight(30);
}
```

### 响应式外壳

```cpp
auto createResponsiveShell() {
    return UI::bindingHost([this]() -> WidgetPtr {
        auto windowSize = getCurrentWindowSize();
        bool isCompact = windowSize.width() < 768;
        
        auto shell = UI::appShell();
        
        if (isCompact) {
            // 紧凑布局：隐藏导航栏，使用底部导航
            shell->nav(nullptr)
                 ->bottomNav(createBottomNavigation());
        } else {
            // 标准布局：显示侧边导航栏
            shell->nav(createSideNavigation())
                 ->bottomNav(nullptr);
        }
        
        return shell->topBar(createTopBar())
                   ->content(createMainContent());
    });
}
```

## 相关文档

- [表现层架构概览](../presentation/architecture.md) - AppShell 在整体架构中的位置
- [TopBar 组件](../presentation/components/top-bar.md) - TopBar 组件的具体配置
- [Binding 与响应式重建](../presentation/binding.md) - AppShell 中的响应式机制