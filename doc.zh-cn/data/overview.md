[English](../../doc/data/overview.md) | **简体中文**

# 数据管理概览

## 架构

Fangjia Qt6 C++ 中的数据层遵循响应式模式，在数据模型、视图模型和表现层之间明确分离。这确保了清晰的数据流和高效的 UI 更新。

## 核心组件

### ThemeManager

中央主题配置和系统集成：

```cpp
class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum class ThemeMode {
        Light,
        Dark,
        FollowSystem
    };

    // 主题状态管理
    void setThemeMode(ThemeMode mode);
    ThemeMode currentThemeMode() const;
    bool isDarkMode() const;
    bool isFollowingSystem() const;

    // 调色板访问
    QColor backgroundColor() const;
    QColor foregroundColor() const;
    QColor accentColor() const;
    QColor borderColor() const;

signals:
    void themeChanged(bool isDark);
    void followSystemChanged(bool following);

private:
    ThemeMode m_themeMode = ThemeMode::FollowSystem;
    bool m_isDarkMode = false;
    void updateFromSystemTheme();
};
```

### AppConfig

持久化应用程序配置：

```cpp
class AppConfig : public QObject {
    Q_OBJECT

public:
    // 窗口状态持久化
    void saveWindowGeometry(const QRect& geometry);
    QRect loadWindowGeometry() const;
    void saveWindowState(bool maximized);
    bool loadWindowState() const;

    // 主题偏好设置
    void saveThemeMode(ThemeManager::ThemeMode mode);
    ThemeManager::ThemeMode loadThemeMode() const;

    // UI 偏好设置
    void saveNavRailWidth(int width);
    int loadNavRailWidth() const;
    void saveLastActiveTab(const QString& tabId);
    QString loadLastActiveTab() const;

private:
    QSettings* m_settings;
    void ensureDefaults();
};
```

### ViewModel 模式

响应式视图模型的基类：

```cpp
class ViewModelBase : public QObject {
    Q_OBJECT

protected:
    template<typename T>
    void setProperty(T& field, const T& value, const char* propertyName) {
        if (field != value) {
            field = value;
            emit propertyChanged(propertyName);
        }
    }

signals:
    void propertyChanged(const QString& propertyName);
};

class NavigationViewModel : public ViewModelBase {
    Q_OBJECT

public:
    // 当前导航状态
    QString currentPageId() const { return m_currentPageId; }
    void setCurrentPageId(const QString& pageId) {
        setProperty(m_currentPageId, pageId, "currentPageId");
    }

    // 导航历史
    QStringList navigationHistory() const { return m_history; }
    bool canGoBack() const { return m_history.size() > 1; }
    void goBack();

    // 选项卡管理
    void addTab(const QString& tabId, const QString& title);
    void removeTab(const QString& tabId);
    void setActiveTab(const QString& tabId);

private:
    QString m_currentPageId;
    QStringList m_history;
    QHash<QString, QString> m_tabs; // tabId -> title
};
```

## 数据绑定系统

### BindingHost

响应式 UI 更新的中央绑定管理：

```cpp
class BindingHost {
public:
    // 属性绑定
    template<typename T>
    void bind(const QString& key, const T& value) {
        m_properties[key] = QVariant::fromValue(value);
        notifyChange(key);
    }

    template<typename T>
    T get(const QString& key) const {
        return m_properties.value(key).value<T>();
    }

    // 变更通知
    void observe(const QString& key, std::function<void()> callback);
    void unobserve(const QString& key);

    // 批量更新
    void beginUpdate();
    void endUpdate();

private:
    QHash<QString, QVariant> m_properties;
    QHash<QString, std::vector<std::function<void()>>> m_observers;
    bool m_inBatchUpdate = false;
    QSet<QString> m_pendingNotifications;

    void notifyChange(const QString& key);
};
```

### RebuildHost

带有环境同步的声明式 UI 重建：

```cpp
class RebuildHost {
public:
    // 组件注册
    void setComponent(std::unique_ptr<IUiComponent> component);
    
    // 环境同步
    void setViewport(const QSize& viewport);
    void setTheme(bool isDark);
    void setResourceContext(IconCache& cache, QOpenGLFunctions* gl, float dpr);

    // 重建管理
    void requestRebuild();
    bool needsRebuild() const { return m_needsRebuild; }
    void performRebuild();

private:
    std::unique_ptr<IUiComponent> m_component;
    QSize m_viewport;
    bool m_isDark = false;
    IconCache* m_iconCache = nullptr;
    QOpenGLFunctions* m_gl = nullptr;
    float m_devicePixelRatio = 1.0f;
    bool m_needsRebuild = true;

    void syncEnvironment();
};
```

## 模型集成示例

### 导航数据流

```cpp
// View Model → UI Component 绑定
class NavigationController {
private:
    NavigationViewModel* m_viewModel;
    UiNavRail* m_navRail;
    BindingHost* m_bindingHost;

public:
    void initialize() {
        // 将视图模型绑定到 UI
        m_bindingHost->observe("currentPageId", [this]() {
            m_navRail->setActiveItem(m_viewModel->currentPageId());
        });

        // 处理 UI 事件
        connect(m_navRail, &UiNavRail::itemClicked, [this](const QString& pageId) {
            m_viewModel->setCurrentPageId(pageId);
        });
    }
};
```

### 主题数据传播

```cpp
class ThemeController {
private:
    ThemeManager* m_themeManager;
    UiRoot* m_uiRoot;

public:
    void initialize() {
        // 将主题变化传播到 UI 树
        connect(m_themeManager, &ThemeManager::themeChanged, [this](bool isDark) {
            m_uiRoot->propagateThemeChange(isDark);
        });

        // 处理系统主题变化
        connect(m_themeManager, &ThemeManager::followSystemChanged, [this](bool following) {
            if (following) {
                m_themeManager->updateFromSystemTheme();
            }
        });
    }
};
```

### 配置持久化

```cpp
class ConfigurationManager {
private:
    AppConfig* m_config;
    ThemeManager* m_themeManager;
    NavigationViewModel* m_navigationVM;

public:
    void saveSession() {
        // 保存主题偏好设置
        m_config->saveThemeMode(m_themeManager->currentThemeMode());

        // 保存导航状态
        m_config->saveLastActiveTab(m_navigationVM->currentPageId());

        // 保存窗口状态（由主窗口处理）
    }

    void restoreSession() {
        // 恢复主题
        auto themeMode = m_config->loadThemeMode();
        m_themeManager->setThemeMode(themeMode);

        // 恢复导航
        auto lastTab = m_config->loadLastActiveTab();
        if (!lastTab.isEmpty()) {
            m_navigationVM->setCurrentPageId(lastTab);
        }
    }
};
```

## 数据验证与处理

### 输入验证

```cpp
class DataValidator {
public:
    struct ValidationResult {
        bool isValid;
        QString errorMessage;
    };

    static ValidationResult validateEmail(const QString& email) {
        QRegularExpression emailRegex(R"([a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,})");
        if (!emailRegex.match(email).hasMatch()) {
            return {false, "Invalid email format"};
        }
        return {true, QString()};
    }

    static ValidationResult validatePath(const QString& path) {
        QFileInfo info(path);
        if (!info.exists()) {
            return {false, "Path does not exist"};
        }
        if (!info.isReadable()) {
            return {false, "Path is not readable"};
        }
        return {true, QString()};
    }
};
```

### 异步数据加载

```cpp
class DataLoader : public QObject {
    Q_OBJECT

public:
    void loadDataAsync(const QString& source) {
        // 使用 QFuture 进行异步操作
        auto future = QtConcurrent::run([source]() {
            // 模拟数据加载
            QThread::sleep(1);
            return loadDataFromSource(source);
        });

        auto watcher = new QFutureWatcher<QVariantList>();
        connect(watcher, &QFutureWatcher<QVariantList>::finished, [this, watcher]() {
            emit dataLoaded(watcher->result());
            watcher->deleteLater();
        });

        watcher->setFuture(future);
    }

signals:
    void dataLoaded(const QVariantList& data);

private:
    static QVariantList loadDataFromSource(const QString& source);
};
```

## 性能考虑

### 延迟加载

```cpp
template<typename T>
class LazyProperty {
private:
    mutable std::optional<T> m_value;
    std::function<T()> m_loader;

public:
    LazyProperty(std::function<T()> loader) : m_loader(loader) {}

    const T& value() const {
        if (!m_value.has_value()) {
            m_value = m_loader();
        }
        return m_value.value();
    }

    void invalidate() {
        m_value.reset();
    }
};
```

### 变更跟踪

```cpp
class ChangeTracker {
private:
    QSet<QString> m_changedProperties;
    QHash<QString, QVariant> m_originalValues;

public:
    void beginTracking(const QString& property, const QVariant& value) {
        if (!m_originalValues.contains(property)) {
            m_originalValues[property] = value;
        }
    }

    void markChanged(const QString& property) {
        m_changedProperties.insert(property);
    }

    bool hasChanges() const {
        return !m_changedProperties.isEmpty();
    }

    QSet<QString> getChangedProperties() const {
        return m_changedProperties;
    }

    void commitChanges() {
        m_changedProperties.clear();
        m_originalValues.clear();
    }
};
```

## 相关文档

- [UI 框架概览](../presentation/ui-framework/overview.md)
- [架构概览](../architecture/overview.md)
- [数据绑定系统](../presentation/binding.md)