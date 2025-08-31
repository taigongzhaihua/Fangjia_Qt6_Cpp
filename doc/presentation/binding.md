# Binding 与响应式重建

本文档介绍 Fangjia Qt6 C++ 框架中的数据绑定机制，包括 binding/observe/requestRebuild 的使用模式与最佳实践。

## 响应式编程概念

### BindingHost - 响应式容器

`BindingHost` 是响应式 UI 系统的核心，它能够：

- **自动依赖追踪**: 检测构建函数中访问的外部状态
- **变更响应**: 当依赖状态变化时自动触发重建
- **生命周期管理**: 管理重建产生的 Widget 实例

```cpp
// 创建响应式 UI 容器
m_shellHost = UI::bindingHost([this]() -> WidgetPtr {
    // 这个 lambda 中访问的所有外部状态都会被自动追踪
    const bool isDark = m_themeMgr->isDarkMode();
    const auto currentPage = m_navigationModel->currentPage();
    
    return appShell()
        ->topBar(UI::topBar()
            ->followSystem(m_themeMgr->followSystem())
            ->onThemeToggle([this]() { toggleTheme(); })
        )
        ->nav(UI::navRail()->dataProvider(&m_navDataProvider))
        ->content(createPageContent(currentPage));
});
```

### 依赖追踪机制

BindingHost 通过以下方式实现自动依赖追踪：

1. **访问检测**: 记录构建过程中访问的所有观察对象
2. **变更通知**: 被访问的对象发生变化时通知 BindingHost
3. **重建触发**: BindingHost 在下一个事件循环中重新执行构建函数
4. **实例替换**: 用新构建的 Widget 替换旧实例

## 观察对象与通知机制

### INotifyPropertyChanged 接口

实现此接口的对象可作为响应式数据源：

```cpp
class ThemeManager : public INotifyPropertyChanged {
private:
    bool m_isDark = false;
    ThemeMode m_mode = ThemeMode::Manual;
    
public:
    bool isDarkMode() const { 
        notifyAccess("isDarkMode");  // 通知访问
        return m_isDark; 
    }
    
    void setDarkMode(bool dark) {
        if (m_isDark != dark) {
            m_isDark = dark;
            notifyChanged("isDarkMode");  // 通知变更
        }
    }
    
    ThemeMode mode() const {
        notifyAccess("mode");
        return m_mode;
    }
    
    void setMode(ThemeMode mode) {
        if (m_mode != mode) {
            m_mode = mode;
            notifyChanged("mode");
        }
    }
};
```

### 手动观察与通知

对于不支持自动追踪的对象，可使用手动方式：

```cpp
// 手动注册观察
m_shellHost->observe(m_dataModel, [this]() {
    // 数据模型变化时的处理逻辑
    requestRebuild();
});

// 手动触发重建
m_shellHost->requestRebuild();
```

## RebuildHost - 重建顺序管理

### 重建生命周期

`RebuildHost` 确保重建过程按正确顺序执行：

```
requestRebuild() 执行顺序：
1. 设置 viewport（给 IUiContent/ILayoutable）
2. 调用 onThemeChanged(isDark)
3. 更新资源上下文 updateResourceContext(...)  
4. 调用 updateLayout(...)
```

这一顺序设计的重要性：
- **避免闪烁**: 确保组件在获得新布局前已完成主题适配
- **资源一致性**: 保证图标缓存键、文本缓存键与当前主题匹配
- **动画连续性**: 支持平滑的主题切换动画

### 重建性能优化

```cpp
class OptimizedComponent : public IUiComponent {
private:
    mutable bool m_layoutDirty = true;
    mutable QSize m_cachedSize;
    
public:
    void requestRebuild() override {
        m_layoutDirty = true;
        // 仅在必要时触发重建
        if (shouldRebuild()) {
            RebuildHost::requestRebuild();
        }
    }
    
    QSize measure(const SizeConstraints& constraints) override {
        if (!m_layoutDirty && m_lastConstraints == constraints) {
            return m_cachedSize;  // 使用缓存结果
        }
        
        m_cachedSize = computeLayout(constraints);
        m_layoutDirty = false;
        m_lastConstraints = constraints;
        return m_cachedSize;
    }
};
```

## 使用模式与最佳实践

### 状态管理模式

**集中式状态管理**：
```cpp
class AppViewModel : public INotifyPropertyChanged {
private:
    std::unique_ptr<ThemeManager> m_theme;
    std::unique_ptr<NavigationModel> m_navigation;
    std::unique_ptr<DataModel> m_data;
    
public:
    ThemeManager* theme() const { 
        notifyAccess("theme");
        return m_theme.get(); 
    }
    
    NavigationModel* navigation() const {
        notifyAccess("navigation");
        return m_navigation.get();
    }
    
    // 组合多个状态变更
    void updateAppState(const AppState& newState) {
        beginBatchUpdate();  // 批量更新开始
        
        if (newState.theme != m_theme->mode()) {
            m_theme->setMode(newState.theme);
        }
        
        if (newState.currentPage != m_navigation->currentPage()) {
            m_navigation->navigateTo(newState.currentPage);
        }
        
        endBatchUpdate();    // 批量更新结束，触发单次重建
    }
};
```

### 条件渲染模式

```cpp
auto createConditionalUI() {
    return UI::bindingHost([this]() -> WidgetPtr {
        const auto user = m_userService->currentUser();
        
        if (!user) {
            // 未登录状态
            return UI::panel()
                ->children({ createLoginForm() })
                ->padding(32);
        }
        
        // 已登录状态
        return UI::grid()
            ->rows({ /* ... */ })
            ->children({
                createUserProfile(user),
                createMainContent(),
                createUserActions()
            });
    });
}
```

### 列表渲染模式

```cpp
auto createDynamicList() {
    return UI::bindingHost([this]() -> WidgetPtr {
        const auto items = m_dataModel->getItems();
        
        std::vector<WidgetPtr> children;
        children.reserve(items.size());
        
        for (const auto& item : items) {
            children.push_back(
                UI::panel()
                    ->children({
                        UI::label()->text(item.title),
                        UI::label()->text(item.subtitle),
                        UI::button()
                            ->text("操作")
                            ->onClick([this, id = item.id]() {
                                handleItemAction(id);
                            })
                    })
                    ->padding(12)
                    ->margin(0, 0, 8, 0)
            );
        }
        
        return UI::scrollView(
            UI::panel()->children(std::move(children))
        );
    });
}
```

## 性能优化策略

### 避免过度重建

```cpp
// ❌ 错误：每次访问都触发重建
auto badExample() {
    return UI::bindingHost([this]() -> WidgetPtr {
        return UI::label()->text(
            QString("当前时间: %1").arg(QTime::currentTime().toString())
        );
    });
}

// ✅ 正确：使用定时器更新状态
class TimeDisplayModel : public INotifyPropertyChanged {
private:
    QString m_currentTime;
    QTimer m_timer;
    
public:
    TimeDisplayModel() {
        connect(&m_timer, &QTimer::timeout, [this]() {
            updateCurrentTime();
        });
        m_timer.start(1000);  // 每秒更新
    }
    
    QString currentTime() const {
        notifyAccess("currentTime");
        return m_currentTime;
    }
    
private:
    void updateCurrentTime() {
        auto newTime = QTime::currentTime().toString();
        if (m_currentTime != newTime) {
            m_currentTime = newTime;
            notifyChanged("currentTime");
        }
    }
};
```

### 组件粒度控制

```cpp
// 将频繁变化的部分拆分为独立的 BindingHost
auto createOptimizedLayout() {
    return UI::grid()
        ->rows({ /* ... */ })
        ->children({
            // 静态标题栏
            UI::topBar()->followSystem(true),
            
            // 动态内容区（独立重建）
            UI::bindingHost([this]() -> WidgetPtr {
                return createDynamicContent();
            }),
            
            // 静态状态栏
            UI::statusBar()
        });
}
```

## 常见问题与解决方案

### 循环依赖检测

```cpp
// 框架会自动检测并阻止循环依赖
class ComponentA : public INotifyPropertyChanged {
    void updateFromB() {
        auto value = m_componentB->getValue();  // 访问 B
        // 此时如果 B 同时访问 A，会触发循环依赖警告
    }
};
```

### 内存泄漏防护

```cpp
// BindingHost 自动管理 Widget 生命周期
class SafeComponent {
    std::unique_ptr<UI::BindingHost> m_host;
    
public:
    SafeComponent() {
        m_host = UI::bindingHost([this]() -> WidgetPtr {
            // Widget 实例由 BindingHost 管理，无需手动释放
            return createUI();
        });
    }
    
    // 析构时 BindingHost 自动清理所有 Widget
    ~SafeComponent() = default;
};
```

## 相关文档

- [表现层架构概览](architecture.md) - BindingHost 与 UiRoot 的协作机制
- [UI 基础部件与容器](ui/components.md) - 如何在容器中使用响应式组件
- [声明式 TopBar 组件](ui/topbar/declarative-topbar.md) - TopBar 中的响应式特性示例