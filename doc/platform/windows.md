**English** | [简体中文](../../doc.zh-cn/platform/windows.md)

# Windows Platform Integration

This document introduces the Windows platform-specific integration features in the Fangjia Qt6 C++ framework, including WinWindowChrome window decoration and HitTest area handling.

## WinWindowChrome - Custom Window Decoration

`WinWindowChrome` provides custom window decoration functionality for the Windows platform, replacing the default system title bar:

### Core Features

- **Custom Title Bar**: Complete replacement of system title bar with custom UI
- **Hit Test Management**: Precise control over window interaction areas
- **DPI Awareness**: Automatic scaling for high-DPI displays
- **Theme Integration**: Seamless integration with application theme system
- **Performance Optimization**: Efficient rendering without compromising system integration

### Architecture Overview

```cpp
class WinWindowChrome {
public:
    // Initialize window chrome for the target window
    void setupForWindow(QWindow* window);
    
    // Configure hit test areas
    void setTitleBarHeight(int height);
    void addDraggableArea(const QRect& area);
    void addButtonArea(const QRect& area, WindowButton button);
    
    // Theme integration
    void setDarkMode(bool dark);
    void updateColors(const WindowColors& colors);
    
    // Event handling
    bool handleNativeEvent(const QByteArray& eventType, void* message, qintptr* result);
    
private:
    void updateHitTestAreas();
    void handleHitTest(const QPoint& pos, qintptr* result);
    void applyWindowEffects();
};
```

### Hit Test Area Management

```cpp
enum class HitTestArea {
    Client,         // Normal client area
    TitleBar,       // Draggable title bar area
    MinButton,      // Minimize button
    MaxButton,      // Maximize/restore button
    CloseButton,    // Close button
    Border,         // Resizable border
    Corner          // Resize corner
};

class HitTestManager {
public:
    void defineArea(const QRect& rect, HitTestArea area);
    void clearAreas();
    HitTestArea getAreaAt(const QPoint& pos) const;
    
    // Predefined layouts
    void setupStandardTitleBar(int height, int buttonWidth);
    void setupCustomLayout(const TitleBarLayout& layout);
    
private:
    std::vector<std::pair<QRect, HitTestArea>> m_areas;
};
```

## Integration with Qt6 Window System

### Window Setup

```cpp
void setupWindowChrome(QWindow* window) {
    auto chrome = std::make_unique<WinWindowChrome>();
    
    // Basic window configuration
    chrome->setupForWindow(window);
    chrome->setTitleBarHeight(40);
    
    // Define hit test areas
    chrome->addDraggableArea(QRect(0, 0, window->width() - 138, 40));  // Title area
    chrome->addButtonArea(QRect(window->width() - 138, 0, 46, 40), WindowButton::Minimize);
    chrome->addButtonArea(QRect(window->width() - 92, 0, 46, 40), WindowButton::Maximize);
    chrome->addButtonArea(QRect(window->width() - 46, 0, 46, 40), WindowButton::Close);
    
    // Apply theme
    chrome->setDarkMode(isDarkTheme());
    
    // Store chrome instance
    window->setProperty("chrome", QVariant::fromValue(chrome.release()));
}
```

### Event Integration

```cpp
class MainWindow : public QOpenGLWindow {
protected:
    bool nativeEvent(const QByteArray& eventType, void* message, qintptr* result) override {
        if (auto* chrome = getWindowChrome()) {
            if (chrome->handleNativeEvent(eventType, message, result)) {
                return true;
            }
        }
        return QOpenGLWindow::nativeEvent(eventType, message, result);
    }
    
    void resizeEvent(QResizeEvent* event) override {
        QOpenGLWindow::resizeEvent(event);
        
        // Update hit test areas when window size changes
        if (auto* chrome = getWindowChrome()) {
            updateHitTestAreas(chrome, event->size());
        }
    }
    
private:
    WinWindowChrome* getWindowChrome() {
        return property("chrome").value<WinWindowChrome*>();
    }
    
    void updateHitTestAreas(WinWindowChrome* chrome, const QSize& size) {
        chrome->clearAreas();
        chrome->addDraggableArea(QRect(0, 0, size.width() - 138, 40));
        chrome->addButtonArea(QRect(size.width() - 138, 0, 46, 40), WindowButton::Minimize);
        chrome->addButtonArea(QRect(size.width() - 92, 0, 46, 40), WindowButton::Maximize);
        chrome->addButtonArea(QRect(size.width() - 46, 0, 46, 40), WindowButton::Close);
    }
};
```

## DPI Awareness

### High-DPI Support

```cpp
class DPIManager {
public:
    static float getWindowDPI(QWindow* window) {
        return window->devicePixelRatio();
    }
    
    static QRect scaledRect(const QRect& logicalRect, float dpi) {
        return QRect(
            std::lround(logicalRect.x() * dpi),
            std::lround(logicalRect.y() * dpi),
            std::lround(logicalRect.width() * dpi),
            std::lround(logicalRect.height() * dpi)
        );
    }
    
    static QSize scaledSize(const QSize& logicalSize, float dpi) {
        return QSize(
            std::lround(logicalSize.width() * dpi),
            std::lround(logicalSize.height() * dpi)
        );
    }
};

void updateForDPI(WinWindowChrome* chrome, float dpi) {
    // Scale title bar height
    int titleBarHeight = std::lround(40 * dpi);
    chrome->setTitleBarHeight(titleBarHeight);
    
    // Scale button dimensions
    int buttonWidth = std::lround(46 * dpi);
    
    // Update hit test areas with DPI-scaled coordinates
    // ... (implementation details)
}
```

### Per-Monitor DPI Awareness

```cpp
class PerMonitorDPIHandler {
public:
    void handleDPIChanged(QWindow* window, float newDPI) {
        auto* chrome = getWindowChrome(window);
        if (!chrome) return;
        
        // Update chrome for new DPI
        updateForDPI(chrome, newDPI);
        
        // Notify UI components to update their resources
        emit dpiChanged(newDPI);
        
        // Trigger layout recalculation
        window->requestUpdate();
    }
    
    void setupDPIMonitoring(QWindow* window) {
        // Windows 10/11 per-monitor DPI change notifications
        connect(window, &QWindow::screenChanged, this, [this, window]() {
            float newDPI = window->devicePixelRatio();
            handleDPIChanged(window, newDPI);
        });
    }
};
```

## Theme Integration

### Windows-Specific Theme Support

```cpp
class WindowsThemeIntegration {
public:
    struct WindowColors {
        QColor titleBarActive;
        QColor titleBarInactive;
        QColor buttonHover;
        QColor buttonPressed;
        QColor closeButtonHover;
        QColor closeButtonPressed;
        QColor text;
        QColor border;
    };
    
    static WindowColors getSystemColors(bool darkMode) {
        WindowColors colors;
        
        if (darkMode) {
            colors.titleBarActive = QColor(32, 32, 32);
            colors.titleBarInactive = QColor(43, 43, 43);
            colors.buttonHover = QColor(64, 64, 64);
            colors.buttonPressed = QColor(80, 80, 80);
            colors.closeButtonHover = QColor(196, 43, 28);
            colors.closeButtonPressed = QColor(153, 27, 18);
            colors.text = QColor(255, 255, 255);
            colors.border = QColor(54, 54, 54);
        } else {
            colors.titleBarActive = QColor(255, 255, 255);
            colors.titleBarInactive = QColor(240, 240, 240);
            colors.buttonHover = QColor(229, 229, 229);
            colors.buttonPressed = QColor(204, 204, 204);
            colors.closeButtonHover = QColor(232, 17, 35);
            colors.closeButtonPressed = QColor(186, 13, 28);
            colors.text = QColor(0, 0, 0);
            colors.border = QColor(204, 204, 204);
        }
        
        return colors;
    }
    
    static bool isSystemDarkMode() {
        // Query Windows registry for system theme preference
        QSettings settings("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                          QSettings::NativeFormat);
        return settings.value("AppsUseLightTheme", 1).toInt() == 0;
    }
};
```

### Dynamic Theme Updates

```cpp
void updateWindowChrome(WinWindowChrome* chrome, bool darkMode) {
    // Get appropriate colors for current theme
    auto colors = WindowsThemeIntegration::getSystemColors(darkMode);
    
    // Apply colors to window chrome
    chrome->setDarkMode(darkMode);
    chrome->updateColors(colors);
    
    // Update window frame if needed
    chrome->applyWindowEffects();
}

class ThemeChangeMonitor : public QObject {
    Q_OBJECT
    
public:
    void startMonitoring() {
        // Monitor Windows theme changes
        m_settingsWatcher = std::make_unique<QFileSystemWatcher>();
        m_settingsWatcher->addPath(getThemeRegistryPath());
        
        connect(m_settingsWatcher.get(), &QFileSystemWatcher::fileChanged,
                this, &ThemeChangeMonitor::onSystemThemeChanged);
    }
    
signals:
    void systemThemeChanged(bool isDark);
    
private slots:
    void onSystemThemeChanged() {
        bool isDark = WindowsThemeIntegration::isSystemDarkMode();
        emit systemThemeChanged(isDark);
    }
    
private:
    std::unique_ptr<QFileSystemWatcher> m_settingsWatcher;
    
    QString getThemeRegistryPath() {
        // Return path to monitor for registry changes
        // Implementation depends on Windows version
        return QString();
    }
};
```

## Performance Considerations

### Efficient Hit Testing

```cpp
class OptimizedHitTester {
public:
    void rebuild(const std::vector<std::pair<QRect, HitTestArea>>& areas) {
        // Build spatial index for faster lookups
        m_quadTree.clear();
        for (const auto& [rect, area] : areas) {
            m_quadTree.insert(rect, area);
        }
    }
    
    HitTestArea testPoint(const QPoint& point) {
        return m_quadTree.query(point);
    }
    
private:
    SpatialIndex<HitTestArea> m_quadTree;
};
```

### Minimal Window Updates

```cpp
class WindowUpdateOptimizer {
public:
    void scheduleUpdate(QWindow* window, const QRect& region) {
        auto& pending = m_pendingUpdates[window];
        pending = pending.united(region);
        
        if (!m_updateTimer.isActive()) {
            m_updateTimer.start(16); // ~60 FPS
        }
    }
    
private slots:
    void processPendingUpdates() {
        for (auto it = m_pendingUpdates.begin(); it != m_pendingUpdates.end(); ++it) {
            QWindow* window = it.key();
            const QRect& region = it.value();
            
            // Trigger minimal repaint
            window->requestUpdate();
        }
        
        m_pendingUpdates.clear();
    }
    
private:
    QHash<QWindow*, QRect> m_pendingUpdates;
    QTimer m_updateTimer;
};
```

## Common Integration Patterns

### Basic Window Chrome Setup

```cpp
auto setupBasicChrome(QWindow* window) {
    auto chrome = std::make_unique<WinWindowChrome>();
    chrome->setupForWindow(window);
    chrome->setTitleBarHeight(32);
    
    // Standard three-button layout
    int windowWidth = window->width();
    chrome->addDraggableArea(QRect(0, 0, windowWidth - 96, 32));
    chrome->addButtonArea(QRect(windowWidth - 96, 0, 32, 32), WindowButton::Minimize);
    chrome->addButtonArea(QRect(windowWidth - 64, 0, 32, 32), WindowButton::Maximize);
    chrome->addButtonArea(QRect(windowWidth - 32, 0, 32, 32), WindowButton::Close);
    
    return chrome;
}
```

### Advanced Custom Chrome

```cpp
class AdvancedWindowChrome {
public:
    void setupCustomLayout(QWindow* window) {
        m_chrome = std::make_unique<WinWindowChrome>();
        m_chrome->setupForWindow(window);
        
        // Custom title bar with logo and controls
        setupCustomTitleBar(window->size());
        
        // Custom resize borders
        setupResizeBorders(window->size());
        
        // Theme integration
        connectThemeSignals();
    }
    
private:
    void setupCustomTitleBar(const QSize& windowSize) {
        const int titleHeight = 48;
        const int logoWidth = 120;
        const int buttonWidth = 46;
        const int buttonCount = 3;
        
        // Logo area (non-draggable)
        m_chrome->addClientArea(QRect(8, 8, logoWidth, titleHeight - 16));
        
        // Title area (draggable)
        int titleStart = logoWidth + 16;
        int titleWidth = windowSize.width() - titleStart - (buttonWidth * buttonCount) - 8;
        m_chrome->addDraggableArea(QRect(titleStart, 0, titleWidth, titleHeight));
        
        // Window control buttons
        int buttonStart = windowSize.width() - (buttonWidth * buttonCount);
        m_chrome->addButtonArea(QRect(buttonStart, 0, buttonWidth, titleHeight), WindowButton::Minimize);
        m_chrome->addButtonArea(QRect(buttonStart + buttonWidth, 0, buttonWidth, titleHeight), WindowButton::Maximize);
        m_chrome->addButtonArea(QRect(buttonStart + buttonWidth * 2, 0, buttonWidth, titleHeight), WindowButton::Close);
    }
    
    void setupResizeBorders(const QSize& windowSize) {
        const int borderWidth = 4;
        
        // Top border
        m_chrome->addBorderArea(QRect(0, 0, windowSize.width(), borderWidth), BorderSide::Top);
        
        // Bottom border
        m_chrome->addBorderArea(QRect(0, windowSize.height() - borderWidth, windowSize.width(), borderWidth), BorderSide::Bottom);
        
        // Left border
        m_chrome->addBorderArea(QRect(0, 0, borderWidth, windowSize.height()), BorderSide::Left);
        
        // Right border
        m_chrome->addBorderArea(QRect(windowSize.width() - borderWidth, 0, borderWidth, windowSize.height()), BorderSide::Right);
        
        // Corners (larger hit areas for easier resizing)
        const int cornerSize = 16;
        m_chrome->addCornerArea(QRect(0, 0, cornerSize, cornerSize), Corner::TopLeft);
        m_chrome->addCornerArea(QRect(windowSize.width() - cornerSize, 0, cornerSize, cornerSize), Corner::TopRight);
        m_chrome->addCornerArea(QRect(0, windowSize.height() - cornerSize, cornerSize, cornerSize), Corner::BottomLeft);
        m_chrome->addCornerArea(QRect(windowSize.width() - cornerSize, windowSize.height() - cornerSize, cornerSize, cornerSize), Corner::BottomRight);
    }
    
    void connectThemeSignals() {
        // Connect to application theme manager
        connect(ThemeManager::instance(), &ThemeManager::themeChanged,
                this, [this](bool isDark) {
                    auto colors = WindowsThemeIntegration::getSystemColors(isDark);
                    m_chrome->updateColors(colors);
                });
    }
    
    std::unique_ptr<WinWindowChrome> m_chrome;
};
```

## Related Documentation

- [Graphics & Rendering System](../infrastructure/gfx.md) - Rendering integration with custom window chrome
- [Presentation Architecture Overview](../presentation/architecture.md) - How window chrome integrates with UI components
- [TopBar Component](../presentation/components/top-bar.md) - Custom title bar implementation