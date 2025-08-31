**English** | [简体中文](../../doc.zh-cn/presentation/binding.md)

# Binding and Reactive Rebuild

This document describes the data binding mechanism in the Fangjia Qt6 C++ framework, including usage patterns and best practices for binding/observe/requestRebuild.

## Reactive Programming Concepts

### BindingHost - Reactive Container

`BindingHost` is the core of the reactive UI system, providing:

- **Automatic dependency tracking**: Detects external state accessed in builder functions
- **Change response**: Automatically triggers rebuilds when dependent state changes
- **Lifecycle management**: Manages Widget instances created during rebuilds

```cpp
// Create reactive UI container
m_shellHost = UI::bindingHost([this]() -> WidgetPtr {
    // All external state accessed in this lambda is automatically tracked
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

### Dependency Tracking Mechanism

BindingHost implements automatic dependency tracking through:

1. **Access detection**: Records all observable objects accessed during build process
2. **Change notification**: Objects notify BindingHost when they change
3. **Rebuild trigger**: BindingHost re-executes the builder function in the next event loop
4. **Instance replacement**: Replaces old Widget with newly built instance

## Observable Objects and Notification Mechanism

### INotifyPropertyChanged Interface

Objects implementing this interface can serve as reactive data sources:

```cpp
class ThemeManager : public INotifyPropertyChanged {
private:
    bool m_isDark = false;
    ThemeMode m_mode = ThemeMode::Manual;
    
public:
    bool isDarkMode() const { 
        notifyAccess("isDarkMode");  // Notify access
        return m_isDark; 
    }
    
    void setDarkMode(bool dark) {
        if (m_isDark != dark) {
            m_isDark = dark;
            notifyChanged("isDarkMode");  // Notify change
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

### Manual Observation and Notification

For objects that don't support automatic tracking, manual approaches can be used:

```cpp
// Manual observation registration using observe helper
UI::observe(m_dataModel, &DataModel::dataChanged, [this]() {
    // Handle data model changes
    requestRebuild();
});

// Manual rebuild trigger
m_shellHost->requestRebuild();
```

## RebuildHost - Rebuild Order Management

### Rebuild Lifecycle

`RebuildHost` ensures the rebuild process executes in the correct order:

```
requestRebuild() execution order:
1. Set viewport (for IUiContent/ILayoutable)
2. Call onThemeChanged(isDark)
3. Update resource context updateResourceContext(...)  
4. Call updateLayout(...)
```

The importance of this order design:
- **Avoid flickering**: Ensures components complete theme adaptation before receiving new layout
- **Resource consistency**: Guarantees icon cache keys and text cache keys match current theme
- **Animation continuity**: Supports smooth theme transition animations

### Rebuild Performance Optimization

```cpp
class OptimizedComponent : public IUiComponent {
private:
    mutable bool m_layoutDirty = true;
    mutable QSize m_cachedSize;
    
public:
    void requestRebuild() override {
        m_layoutDirty = true;
        // Only trigger rebuild when necessary
        if (shouldRebuild()) {
            RebuildHost::requestRebuild();
        }
    }
    
    QSize measure(const SizeConstraints& constraints) override {
        if (!m_layoutDirty && m_lastConstraints == constraints) {
            return m_cachedSize;  // Use cached result
        }
        
        m_cachedSize = computeLayout(constraints);
        m_layoutDirty = false;
        m_lastConstraints = constraints;
        return m_cachedSize;
    }
};
```

## Usage Patterns and Best Practices

### State Management Pattern

**Centralized state management**:
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
    
    // Combine multiple state changes
    void updateAppState(const AppState& newState) {
        beginBatchUpdate();  // Begin batch update
        
        if (newState.theme != m_theme->mode()) {
            m_theme->setMode(newState.theme);
        }
        
        if (newState.currentPage != m_navigation->currentPage()) {
            m_navigation->navigateTo(newState.currentPage);
        }
        
        endBatchUpdate();    // End batch update, trigger single rebuild
    }
};
```

### Conditional Rendering Pattern

```cpp
auto createConditionalUI() {
    return UI::bindingHost([this]() -> WidgetPtr {
        const auto user = m_userService->currentUser();
        
        if (!user) {
            // Not logged in state
            return UI::panel()
                ->children({ createLoginForm() })
                ->padding(32);
        }
        
        // Logged in state
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

### List Rendering Pattern

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
                            ->text("Action")
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

## Performance Optimization Strategies

### Avoiding Excessive Rebuilds

```cpp
// ❌ Wrong: Triggers rebuild on every access
auto badExample() {
    return UI::bindingHost([this]() -> WidgetPtr {
        return UI::label()->text(
            QString("Current time: %1").arg(QTime::currentTime().toString())
        );
    });
}

// ✅ Correct: Use timer to update state
class TimeDisplayModel : public INotifyPropertyChanged {
private:
    QString m_currentTime;
    QTimer m_timer;
    
public:
    TimeDisplayModel() {
        connect(&m_timer, &QTimer::timeout, [this]() {
            updateCurrentTime();
        });
        m_timer.start(1000);  // Update every second
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

### Component Granularity Control

```cpp
// Split frequently changing parts into independent BindingHost instances
auto createOptimizedLayout() {
    return UI::grid()
        ->rows({ /* ... */ })
        ->children({
            // Static header
            UI::topBar()->followSystem(true),
            
            // Dynamic content area (independent rebuild)
            UI::bindingHost([this]() -> WidgetPtr {
                return createDynamicContent();
            }),
            
            // Static status bar
            UI::statusBar()
        });
}
```

## Common Issues and Solutions

### Circular Dependency Detection

```cpp
// Framework automatically detects and prevents circular dependencies
class ComponentA : public INotifyPropertyChanged {
    void updateFromB() {
        auto value = m_componentB->getValue();  // Access B
        // If B simultaneously accesses A, circular dependency warning is triggered
    }
};
```

### Memory Leak Protection

```cpp
// BindingHost automatically manages Widget lifecycle
class SafeComponent {
    std::unique_ptr<UI::BindingHost> m_host;
    
public:
    SafeComponent() {
        m_host = UI::bindingHost([this]() -> WidgetPtr {
            // Widget instances are managed by BindingHost, no manual cleanup needed
            return createUI();
        });
    }
    
    // BindingHost automatically cleans up all Widgets on destruction
    ~SafeComponent() = default;
};
```

## Using Connectors for Signal Binding

For more complex scenarios, use the `connect` method to set up signal bindings:

```cpp
auto createReactiveComponent() {
    return UI::bindingHost([this]() -> WidgetPtr {
        return createMyUI();
    })
    ->connect([this](UI::RebuildHost* host) {
        // Use observe helper to connect ViewModel signals to rebuild
        UI::observe(m_viewModel, &ViewModel::dataChanged, [host]() {
            host->requestRebuild();
        });
        
        UI::observe(m_themeManager, &ThemeManager::themeChanged, [host]() {
            host->requestRebuild();
        });
    });
}
```

## Related Documentation

- [Presentation Layer Architecture Overview](architecture.md) - Collaboration mechanism between BindingHost and UiRoot
- [UI Basic Components and Containers](ui/components.md) - How to use reactive components in containers
- [Declarative TopBar Component](ui/topbar/declarative-topbar.md) - Examples of reactive features in TopBar