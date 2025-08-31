# Data Management Overview

## Architecture

The data layer in Fangjia Qt6 C++ follows a reactive pattern with clear separation between data models, view models, and the presentation layer. This ensures clean data flow and efficient UI updates.

## Core Components

### ThemeManager

Central theme configuration and system integration:

```cpp
class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum class ThemeMode {
        Light,
        Dark,
        FollowSystem
    };

    // Theme state management
    void setThemeMode(ThemeMode mode);
    ThemeMode currentThemeMode() const;
    bool isDarkMode() const;
    bool isFollowingSystem() const;

    // Color palette access
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

Persistent application configuration:

```cpp
class AppConfig : public QObject {
    Q_OBJECT

public:
    // Window state persistence
    void saveWindowGeometry(const QRect& geometry);
    QRect loadWindowGeometry() const;
    void saveWindowState(bool maximized);
    bool loadWindowState() const;

    // Theme preferences
    void saveThemeMode(ThemeManager::ThemeMode mode);
    ThemeManager::ThemeMode loadThemeMode() const;

    // UI preferences
    void saveNavRailWidth(int width);
    int loadNavRailWidth() const;
    void saveLastActiveTab(const QString& tabId);
    QString loadLastActiveTab() const;

private:
    QSettings* m_settings;
    void ensureDefaults();
};
```

### ViewModel Pattern

Base class for reactive view models:

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
    // Current navigation state
    QString currentPageId() const { return m_currentPageId; }
    void setCurrentPageId(const QString& pageId) {
        setProperty(m_currentPageId, pageId, "currentPageId");
    }

    // Navigation history
    QStringList navigationHistory() const { return m_history; }
    bool canGoBack() const { return m_history.size() > 1; }
    void goBack();

    // Tab management
    void addTab(const QString& tabId, const QString& title);
    void removeTab(const QString& tabId);
    void setActiveTab(const QString& tabId);

private:
    QString m_currentPageId;
    QStringList m_history;
    QHash<QString, QString> m_tabs; // tabId -> title
};
```

## Data Binding System

### BindingHost

Central binding management for reactive UI updates:

```cpp
class BindingHost {
public:
    // Property binding
    template<typename T>
    void bind(const QString& key, const T& value) {
        m_properties[key] = QVariant::fromValue(value);
        notifyChange(key);
    }

    template<typename T>
    T get(const QString& key) const {
        return m_properties.value(key).value<T>();
    }

    // Change notifications
    void observe(const QString& key, std::function<void()> callback);
    void unobserve(const QString& key);

    // Batch updates
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

Declarative UI rebuilding with environment synchronization:

```cpp
class RebuildHost {
public:
    // Component registration
    void setComponent(std::unique_ptr<IUiComponent> component);
    
    // Environment synchronization
    void setViewport(const QSize& viewport);
    void setTheme(bool isDark);
    void setResourceContext(IconCache& cache, QOpenGLFunctions* gl, float dpr);

    // Rebuild management
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

## Model Integration Examples

### Navigation Data Flow

```cpp
// View Model → UI Component binding
class NavigationController {
private:
    NavigationViewModel* m_viewModel;
    UiNavRail* m_navRail;
    BindingHost* m_bindingHost;

public:
    void initialize() {
        // Bind view model to UI
        m_bindingHost->observe("currentPageId", [this]() {
            m_navRail->setActiveItem(m_viewModel->currentPageId());
        });

        // Handle UI events
        connect(m_navRail, &UiNavRail::itemClicked, [this](const QString& pageId) {
            m_viewModel->setCurrentPageId(pageId);
        });
    }
};
```

### Theme Data Propagation

```cpp
class ThemeController {
private:
    ThemeManager* m_themeManager;
    UiRoot* m_uiRoot;

public:
    void initialize() {
        // Propagate theme changes to UI tree
        connect(m_themeManager, &ThemeManager::themeChanged, [this](bool isDark) {
            m_uiRoot->propagateThemeChange(isDark);
        });

        // Handle system theme changes
        connect(m_themeManager, &ThemeManager::followSystemChanged, [this](bool following) {
            if (following) {
                m_themeManager->updateFromSystemTheme();
            }
        });
    }
};
```

### Configuration Persistence

```cpp
class ConfigurationManager {
private:
    AppConfig* m_config;
    ThemeManager* m_themeManager;
    NavigationViewModel* m_navigationVM;

public:
    void saveSession() {
        // Save theme preferences
        m_config->saveThemeMode(m_themeManager->currentThemeMode());

        // Save navigation state
        m_config->saveLastActiveTab(m_navigationVM->currentPageId());

        // Save window state (handled by main window)
    }

    void restoreSession() {
        // Restore theme
        auto themeMode = m_config->loadThemeMode();
        m_themeManager->setThemeMode(themeMode);

        // Restore navigation
        auto lastTab = m_config->loadLastActiveTab();
        if (!lastTab.isEmpty()) {
            m_navigationVM->setCurrentPageId(lastTab);
        }
    }
};
```

## Data Validation & Processing

### Input Validation

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

### Asynchronous Data Loading

```cpp
class DataLoader : public QObject {
    Q_OBJECT

public:
    void loadDataAsync(const QString& source) {
        // Use QFuture for async operations
        auto future = QtConcurrent::run([source]() {
            // Simulate data loading
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

## Performance Considerations

### Lazy Loading

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

### Change Tracking

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

## Related Documentation

- [UI Framework Overview](../presentation/ui-framework/overview.md)
- [Architecture Overview](../architecture/overview.md)
- [Data Binding System](../presentation/binding.md)