# Windows Platform Integration

## Overview

The Fangjia Qt6 C++ framework includes specialized Windows platform integration for enhanced native window behavior, custom window chrome, and optimal user experience on Windows 10/11.

## WinWindowChrome - Custom Window Decoration

### Native Window Frame Replacement

The `WinWindowChrome` component provides custom window decoration that replaces the standard Windows title bar while maintaining native window behavior:

```cpp
class WinWindowChrome {
public:
    // Window chrome configuration
    void setCustomTitleBar(bool enabled);
    void setTitleBarHeight(int height);
    void setResizeBorderWidth(int width);
    
    // Caption button integration
    void addCaptionButton(const QString& id, const QRect& bounds);
    void removeCaptionButton(const QString& id);
    
    // Hit test customization
    void setHitTestRegion(const QRect& region, HitTestResult result);
    HitTestResult performHitTest(const QPoint& position);
};
```

### Hit Test Region Management

Windows requires precise hit testing for proper window behavior. The framework provides detailed hit test zone configuration:

```cpp
enum class HitTestResult {
    Client,        // Normal client area
    Caption,       // Draggable title bar area
    MinButton,     // Minimize button
    MaxButton,     // Maximize/restore button
    CloseButton,   // Close button
    TopResize,     // Top resize border
    BottomResize,  // Bottom resize border
    LeftResize,    // Left resize border
    RightResize,   // Right resize border
    TopLeftResize, // Top-left corner resize
    TopRightResize,// Top-right corner resize
    BottomLeftResize,  // Bottom-left corner resize
    BottomRightResize, // Bottom-right corner resize
    SystemMenu,    // System menu area (typically app icon)
    Nowhere        // Non-interactive area
};
```

### Integration with TopBar Component

The TopBar component integrates seamlessly with Windows chrome:

```cpp
class UiTopBar : public IUiComponent {
private:
    WinWindowChrome* m_windowChrome;
    
public:
    void updateWindowChrome() {
        if (!m_windowChrome) return;
        
        // Register caption buttons with Windows
        m_windowChrome->addCaptionButton("minimize", m_btnMin.baseRect());
        m_windowChrome->addCaptionButton("maximize", m_btnMax.baseRect());
        m_windowChrome->addCaptionButton("close", m_btnClose.baseRect());
        
        // Set draggable title bar region
        QRect titleBarRegion = bounds();
        titleBarRegion.setRight(m_btnMin.baseRect().left() - 8); // Exclude buttons
        m_windowChrome->setHitTestRegion(titleBarRegion, HitTestResult::Caption);
    }
    
    bool onMousePress(const QPoint& pos) override {
        // Handle button clicks before Windows processing
        if (m_btnClose.contains(pos)) {
            emit closeRequested();
            return true; // Handled locally
        }
        
        return false; // Let Windows handle
    }
};
```

## Window State Management

### Maximize/Restore Behavior

Custom handling of window state changes:

```cpp
class MainOpenGlWindow : public QOpenGLWidget {
private:
    bool m_isMaximized = false;
    QRect m_normalGeometry;
    
public:
    void toggleMaximized() {
        if (m_isMaximized) {
            restoreWindow();
        } else {
            maximizeWindow();
        }
    }
    
private:
    void maximizeWindow() {
        if (!m_isMaximized) {
            m_normalGeometry = geometry();
            
            // Get work area to avoid taskbar
            QScreen* screen = QApplication::primaryScreen();
            QRect workArea = screen->availableGeometry();
            setGeometry(workArea);
            
            m_isMaximized = true;
            updateTopBarButtons();
        }
    }
    
    void restoreWindow() {
        if (m_isMaximized && !m_normalGeometry.isEmpty()) {
            setGeometry(m_normalGeometry);
            m_isMaximized = false;
            updateTopBarButtons();
        }
    }
};
```

### Window Events Integration

Handle Windows-specific events:

```cpp
bool MainOpenGlWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        
        switch (msg->message) {
        case WM_NCHITTEST: {
            // Custom hit testing for window chrome
            POINT pt = { GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam) };
            ScreenToClient(msg->hwnd, &pt);
            
            HitTestResult hitTest = m_topBar->performHitTest(QPoint(pt.x, pt.y));
            *result = translateHitTest(hitTest);
            return true;
        }
        
        case WM_NCLBUTTONDOWN:
            // Handle non-client button clicks
            return handleNonClientClick(msg->wParam);
            
        case WM_GETMINMAXINFO: {
            // Set minimum window size
            MINMAXINFO* info = reinterpret_cast<MINMAXINFO*>(msg->lParam);
            info->ptMinTrackSize.x = 400;
            info->ptMinTrackSize.y = 300;
            return true;
        }
        }
    }
    
    return QOpenGLWidget::nativeEvent(eventType, message, result);
}
```

## DPI Awareness

### High-DPI Support

Windows high-DPI handling with per-monitor awareness:

```cpp
void MainOpenGlWindow::initializeDpiAwareness() {
    // Enable per-monitor DPI awareness V2
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    
    // Monitor DPI changes
    connect(windowHandle(), &QWindow::screenChanged, this, &MainOpenGlWindow::onScreenChanged);
}

void MainOpenGlWindow::onScreenChanged(QScreen* screen) {
    float newDpr = screen->devicePixelRatio();
    
    if (m_currentDpr != newDpr) {
        // Update rendering system for new DPI
        m_iconCache->setDevicePixelRatio(newDpr);
        m_renderer->updateDpiScaling(newDpr);
        
        // Trigger layout recalculation
        m_uiRoot->requestRebuild();
        
        m_currentDpr = newDpr;
    }
}
```

## Windows 11 Integration

### Rounded Window Corners

Support for Windows 11 rounded window corners:

```cpp
void MainOpenGlWindow::applyWindows11Styling() {
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows11) {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        
        // Enable rounded corners
        DWM_WINDOW_CORNER_PREFERENCE corner = DWMWCP_ROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));
        
        // Enable backdrop effects (optional)
        DWM_SYSTEMBACKDROP_TYPE backdrop = DWMSBT_MAINWINDOW;
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));
    }
}
```

### System Theme Detection

Integrate with Windows theme changes:

```cpp
bool MainOpenGlWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        
        if (msg->message == WM_SETTINGCHANGE) {
            // Check for theme changes
            if (QString::fromWCharArray(reinterpret_cast<wchar_t*>(msg->lParam)) == L"ImmersiveColorSet") {
                bool isDark = isWindowsDarkTheme();
                if (m_themeManager->isFollowingSystem()) {
                    m_themeManager->setDarkMode(isDark);
                }
                return true;
            }
        }
    }
    
    return QOpenGLWidget::nativeEvent(eventType, message, result);
}

bool MainOpenGlWindow::isWindowsDarkTheme() {
    QSettings registry("HKEY_CURRENT_USER\\Software\\Microsoft\\Windows\\CurrentVersion\\Themes\\Personalize",
                      QSettings::NativeFormat);
    return registry.value("AppsUseLightTheme", 1).toInt() == 0;
}
```

## Performance Optimizations

### DirectComposition Integration

Leverage Windows DirectComposition for smooth animations:

```cpp
void MainOpenGlWindow::enableDirectComposition() {
    HWND hwnd = reinterpret_cast<HWND>(winId());
    
    // Enable DirectComposition
    BOOL enable = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_ENABLED, &enable, sizeof(enable));
    
    // Configure composition policy
    DWMNCRENDERINGPOLICY policy = DWMNCRP_USEWINDOWSTYLE;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));
}
```

### GPU Memory Management

Windows-specific GPU memory optimization:

```cpp
void Renderer::optimizeForWindows() {
    // Use D3D11 interop when available
    if (QOpenGLContext::currentContext()->hasExtension("WGL_NV_DX_interop2")) {
        enableDirectXInterop();
    }
    
    // Configure Windows-specific texture formats
    configureOptimalTextureFormats();
}
```

## Accessibility Integration

### Windows Narrator Support

Basic accessibility integration:

```cpp
void UiComponent::announceToScreenReader(const QString& message) {
    #ifdef Q_OS_WIN
    // Use Windows SAPI for screen reader announcements
    ISpVoice* pVoice = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice))) {
        std::wstring wMessage = message.toStdWString();
        pVoice->Speak(wMessage.c_str(), SPF_ASYNC, nullptr);
        pVoice->Release();
    }
    #endif
}
```

## Related Documentation

- [Graphics & Rendering System](gfx.md)
- [Architecture Overview](../architecture/overview.md)
- [TopBar Component](../presentation/components/top-bar.md)