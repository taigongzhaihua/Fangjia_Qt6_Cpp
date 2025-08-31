[English](../../doc/infrastructure/platform-windows.md) | **简体中文**

# Windows 平台集成

## 概述

Fangjia Qt6 C++ 框架包含专门的 Windows 平台集成，用于增强原生窗口行为、自定义窗口装饰，以及在 Windows 10/11 上提供最佳用户体验。

## WinWindowChrome - 自定义窗口装饰

### 原生窗口框架替换

`WinWindowChrome` 组件提供自定义窗口装饰，替换标准的 Windows 标题栏，同时保持原生窗口行为：

```cpp
class WinWindowChrome {
public:
    // 窗口装饰配置
    void setCustomTitleBar(bool enabled);
    void setTitleBarHeight(int height);
    void setResizeBorderWidth(int width);
    
    // 标题栏按钮集成
    void addCaptionButton(const QString& id, const QRect& bounds);
    void removeCaptionButton(const QString& id);
    
    // 命中测试自定义
    void setHitTestRegion(const QRect& region, HitTestResult result);
    HitTestResult performHitTest(const QPoint& position);
};
```

### 命中测试区域管理

Windows 需要精确的命中测试以确保正确的窗口行为。框架提供详细的命中测试区域配置：

```cpp
enum class HitTestResult {
    Client,        // 普通客户区
    Caption,       // 可拖拽标题栏区域
    MinButton,     // 最小化按钮
    MaxButton,     // 最大化/还原按钮
    CloseButton,   // 关闭按钮
    TopResize,     // 顶部调整边框
    BottomResize,  // 底部调整边框
    LeftResize,    // 左侧调整边框
    RightResize,   // 右侧调整边框
    TopLeftResize, // 左上角调整
    TopRightResize,// 右上角调整
    BottomLeftResize,  // 左下角调整
    BottomRightResize, // 右下角调整
    SystemMenu,    // 系统菜单区域（通常是应用图标）
    Nowhere        // 非交互区域
};
```

### 与 TopBar 组件集成

TopBar 组件与 Windows 装饰无缝集成：

```cpp
class UiTopBar : public IUiComponent {
private:
    WinWindowChrome* m_windowChrome;
    
public:
    void updateWindowChrome() {
        if (!m_windowChrome) return;
        
        // 向 Windows 注册标题栏按钮
        m_windowChrome->addCaptionButton("minimize", m_btnMin.baseRect());
        m_windowChrome->addCaptionButton("maximize", m_btnMax.baseRect());
        m_windowChrome->addCaptionButton("close", m_btnClose.baseRect());
        
        // 设置可拖拽标题栏区域
        QRect titleBarRegion = bounds();
        titleBarRegion.setRight(m_btnMin.baseRect().left() - 8); // 排除按钮
        m_windowChrome->setHitTestRegion(titleBarRegion, HitTestResult::Caption);
    }
    
    bool onMousePress(const QPoint& pos) override {
        // 在 Windows 处理之前处理按钮点击
        if (m_btnClose.contains(pos)) {
            emit closeRequested();
            return true; // 本地处理
        }
        
        return false; // 让 Windows 处理
    }
};
```

## 窗口状态管理

### 最大化/还原行为

窗口状态变化的自定义处理：

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
            
            // 获取工作区以避免任务栏
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

### 窗口事件集成

处理 Windows 特定事件：

```cpp
bool MainOpenGlWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        
        switch (msg->message) {
        case WM_NCHITTEST: {
            // 窗口装饰的自定义命中测试
            POINT pt = { GET_X_LPARAM(msg->lParam), GET_Y_LPARAM(msg->lParam) };
            ScreenToClient(msg->hwnd, &pt);
            
            HitTestResult hitTest = m_topBar->performHitTest(QPoint(pt.x, pt.y));
            *result = translateHitTest(hitTest);
            return true;
        }
        
        case WM_NCLBUTTONDOWN:
            // 处理非客户区按钮点击
            return handleNonClientClick(msg->wParam);
            
        case WM_GETMINMAXINFO: {
            // 设置最小窗口尺寸
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

## DPI 感知

### 高 DPI 支持

Windows 高 DPI 处理，支持每监视器感知：

```cpp
void MainOpenGlWindow::initializeDpiAwareness() {
    // 启用每监视器 DPI 感知 V2
    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);
    
    // 监控 DPI 变化
    connect(windowHandle(), &QWindow::screenChanged, this, &MainOpenGlWindow::onScreenChanged);
}

void MainOpenGlWindow::onScreenChanged(QScreen* screen) {
    float newDpr = screen->devicePixelRatio();
    
    if (m_currentDpr != newDpr) {
        // 为新 DPI 更新渲染系统
        m_iconCache->setDevicePixelRatio(newDpr);
        m_renderer->updateDpiScaling(newDpr);
        
        // 触发布局重新计算
        m_uiRoot->requestRebuild();
        
        m_currentDpr = newDpr;
    }
}
```

## Windows 11 集成

### 圆角窗口

支持 Windows 11 圆角窗口：

```cpp
void MainOpenGlWindow::applyWindows11Styling() {
    if (QOperatingSystemVersion::current() >= QOperatingSystemVersion::Windows11) {
        HWND hwnd = reinterpret_cast<HWND>(winId());
        
        // 启用圆角
        DWM_WINDOW_CORNER_PREFERENCE corner = DWMWCP_ROUND;
        DwmSetWindowAttribute(hwnd, DWMWA_WINDOW_CORNER_PREFERENCE, &corner, sizeof(corner));
        
        // 启用背景效果（可选）
        DWM_SYSTEMBACKDROP_TYPE backdrop = DWMSBT_MAINWINDOW;
        DwmSetWindowAttribute(hwnd, DWMWA_SYSTEMBACKDROP_TYPE, &backdrop, sizeof(backdrop));
    }
}
```

### 系统主题检测

与 Windows 主题变化集成：

```cpp
bool MainOpenGlWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        
        if (msg->message == WM_SETTINGCHANGE) {
            // 检查主题变化
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

## 性能优化

### DirectComposition 集成

利用 Windows DirectComposition 实现平滑动画：

```cpp
void MainOpenGlWindow::enableDirectComposition() {
    HWND hwnd = reinterpret_cast<HWND>(winId());
    
    // 启用 DirectComposition
    BOOL enable = TRUE;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_ENABLED, &enable, sizeof(enable));
    
    // 配置合成策略
    DWMNCRENDERINGPOLICY policy = DWMNCRP_USEWINDOWSTYLE;
    DwmSetWindowAttribute(hwnd, DWMWA_NCRENDERING_POLICY, &policy, sizeof(policy));
}
```

### GPU 内存管理

Windows 特定的 GPU 内存优化：

```cpp
void Renderer::optimizeForWindows() {
    // 在可用时使用 D3D11 互操作
    if (QOpenGLContext::currentContext()->hasExtension("WGL_NV_DX_interop2")) {
        enableDirectXInterop();
    }
    
    // 配置 Windows 特定的纹理格式
    configureOptimalTextureFormats();
}
```

## 无障碍集成

### Windows 讲述人支持

基本无障碍集成：

```cpp
void UiComponent::announceToScreenReader(const QString& message) {
    #ifdef Q_OS_WIN
    // 使用 Windows SAPI 进行屏幕阅读器播报
    ISpVoice* pVoice = nullptr;
    if (SUCCEEDED(CoCreateInstance(CLSID_SpVoice, nullptr, CLSCTX_ALL, IID_ISpVoice, (void**)&pVoice))) {
        std::wstring wMessage = message.toStdWString();
        pVoice->Speak(wMessage.c_str(), SPF_ASYNC, nullptr);
        pVoice->Release();
    }
    #endif
}
```

## 相关文档

- [图形与渲染系统](gfx.md)
- [架构概览](../architecture/overview.md)
- [TopBar 组件](../presentation/components/top-bar.md)