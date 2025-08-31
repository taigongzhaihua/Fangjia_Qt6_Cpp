# Windows 平台集成

本文档介绍 Fangjia Qt6 C++ 框架在 Windows 平台上的特定集成功能，包括 WinWindowChrome 窗口装饰和 HitTest 区域处理。

## WinWindowChrome - 自定义窗口装饰

### 功能概述

`WinWindowChrome` 为 Windows 平台提供自定义窗口装饰功能，替代默认的系统标题栏：

- **自定义标题栏**: 完全控制标题栏的外观与交互
- **窗口控制按钮**: 自绘的最小化、最大化、关闭按钮
- **拖拽区域**: 定义可拖拽移动窗口的区域
- **调整尺寸**: 支持边缘拖拽调整窗口大小
- **主题集成**: 与应用主题系统无缝集成

```cpp
class WinWindowChrome {
public:
    // 初始化窗口装饰
    static void setupWindow(HWND hwnd, bool customFrame = true);
    
    // 处理 Windows 消息
    static LRESULT handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
    
    // 设置标题栏高度
    static void setTitleBarHeight(HWND hwnd, int height);
    
    // 定义窗口控制按钮区域
    static void setControlButtonsArea(HWND hwnd, const QRect& area);
};
```

### 窗口创建与初始化

```cpp
class MainOpenGlWindow : public QOpenGLWidget {
protected:
    void showEvent(QShowEvent* event) override {
        QOpenGLWidget::showEvent(event);
        
        // 在 Windows 平台启用自定义窗口装饰
#ifdef Q_OS_WIN
        auto hwnd = reinterpret_cast<HWND>(winId());
        WinWindowChrome::setupWindow(hwnd, true);
        WinWindowChrome::setTitleBarHeight(hwnd, 40);  // 40px 标题栏
        
        // 定义窗口控制按钮区域（右上角 150px 宽度）
        auto windowSize = size();
        QRect controlArea(windowSize.width() - 150, 0, 150, 40);
        WinWindowChrome::setControlButtonsArea(hwnd, controlArea);
#endif
    }
};
```

### Windows 消息处理

```cpp
bool MainOpenGlWindow::nativeEvent(const QByteArray& eventType, void* message, qintptr* result) {
#ifdef Q_OS_WIN
    if (eventType == "windows_generic_MSG") {
        MSG* msg = static_cast<MSG*>(message);
        
        // 委托给 WinWindowChrome 处理窗口装饰相关消息
        LRESULT chromeResult = WinWindowChrome::handleMessage(
            msg->hwnd, msg->message, msg->wParam, msg->lParam
        );
        
        if (chromeResult != 0) {
            *result = chromeResult;
            return true;  // 消息已处理
        }
    }
#endif
    
    return QOpenGLWidget::nativeEvent(eventType, message, result);
}
```

## HitTest 区域处理

### HitTest 概念

HitTest 确定鼠标指针在窗口中的功能区域：

- **HTCLIENT**: 客户区域（应用内容）
- **HTCAPTION**: 标题栏（可拖拽移动）
- **HTSYSMENU**: 系统菜单区域
- **HTMINBUTTON**: 最小化按钮
- **HTMAXBUTTON**: 最大化按钮  
- **HTCLOSE**: 关闭按钮
- **HTLEFT/RIGHT/TOP/BOTTOM**: 边缘（可调整尺寸）

### 区域映射实现

```cpp
class HitTestManager {
private:
    struct HitTestRegion {
        QRect bounds;
        LRESULT hitTest;
        bool enabled = true;
    };
    
    std::vector<HitTestRegion> m_regions;
    QRect m_windowBounds;
    int m_borderWidth = 4;
    
public:
    // 设置标题栏区域
    void setTitleBarArea(const QRect& area) {
        addRegion(area, HTCAPTION);
    }
    
    // 设置窗口控制按钮
    void setMinimizeButton(const QRect& area) {
        addRegion(area, HTMINBUTTON);
    }
    
    void setMaximizeButton(const QRect& area) {
        addRegion(area, HTMAXBUTTON);
    }
    
    void setCloseButton(const QRect& area) {
        addRegion(area, HTCLOSE);
    }
    
    // 执行 HitTest
    LRESULT hitTest(const QPoint& point) const;
    
private:
    void addRegion(const QRect& bounds, LRESULT hitTest) {
        m_regions.push_back({bounds, hitTest, true});
    }
};
```

### 动态区域更新

```cpp
void MainOpenGlWindow::updateHitTestRegions() {
    auto windowSize = size();
    
    // 标题栏区域（排除窗口控制按钮）
    QRect titleBarArea(0, 0, windowSize.width() - 150, 40);
    m_hitTestManager->setTitleBarArea(titleBarArea);
    
    // 窗口控制按钮（每个 50px 宽）
    int buttonY = 0;
    int buttonHeight = 40;
    int buttonWidth = 50;
    int startX = windowSize.width() - 150;
    
    m_hitTestManager->setMinimizeButton(QRect(startX, buttonY, buttonWidth, buttonHeight));
    m_hitTestManager->setMaximizeButton(QRect(startX + 50, buttonY, buttonWidth, buttonHeight));
    m_hitTestManager->setCloseButton(QRect(startX + 100, buttonY, buttonWidth, buttonHeight));
    
    // 边缘调整区域
    int borderWidth = 4;
    m_hitTestManager->setBorderWidth(borderWidth);
}

void MainOpenGlWindow::resizeEvent(QResizeEvent* event) {
    QOpenGLWidget::resizeEvent(event);
    updateHitTestRegions();  // 窗口尺寸变化时更新区域
}
```

## 主题集成

### 窗口装饰主题化

```cpp
class ThemedWindowChrome {
private:
    bool m_isDarkTheme = false;
    QColor m_titleBarColor;
    QColor m_buttonHoverColor;
    QColor m_buttonPressedColor;
    
public:
    void setTheme(bool isDark) {
        m_isDarkTheme = isDark;
        
        if (isDark) {
            m_titleBarColor = QColor(32, 32, 32);
            m_buttonHoverColor = QColor(64, 64, 64);
            m_buttonPressedColor = QColor(48, 48, 48);
        } else {
            m_titleBarColor = QColor(248, 248, 248);
            m_buttonHoverColor = QColor(228, 228, 228);
            m_buttonPressedColor = QColor(208, 208, 208);
        }
        
        updateWindowAppearance();
    }
    
private:
    void updateWindowAppearance() {
        // 更新系统窗口外观（Windows 10/11）
        updateDwmAttributes();
        
        // 触发重绘
        InvalidateRect(m_hwnd, nullptr, FALSE);
    }
    
    void updateDwmAttributes() {
        // 设置窗口暗色模式（Windows 10 build 18985+）
        BOOL darkMode = m_isDarkTheme ? TRUE : FALSE;
        DwmSetWindowAttribute(m_hwnd, DWMWA_USE_IMMERSIVE_DARK_MODE, 
                             &darkMode, sizeof(darkMode));
        
        // 设置标题栏颜色（Windows 11）
        COLORREF titleBarColor = RGB(m_titleBarColor.red(), 
                                    m_titleBarColor.green(), 
                                    m_titleBarColor.blue());
        DwmSetWindowAttribute(m_hwnd, DWMWA_CAPTION_COLOR, 
                             &titleBarColor, sizeof(titleBarColor));
    }
};
```

### 与应用主题同步

```cpp
void MainOpenGlWindow::onThemeChanged(bool isDark) {
    // 更新窗口装饰主题
    m_windowChrome->setTheme(isDark);
    
    // 更新应用内容主题
    if (m_uiRoot) {
        m_uiRoot->propagateThemeChange(isDark);
    }
    
    // 触发重绘
    update();
}
```

## 性能优化

### 消息处理优化

```cpp
LRESULT WinWindowChrome::handleMessage(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam) {
    switch (msg) {
    case WM_NCHITTEST:
        // 高频消息，需要快速处理
        return handleHitTest(hwnd, lParam);
        
    case WM_NCPAINT:
        // 非客户区绘制，返回 0 禁用默认绘制
        return 0;
        
    case WM_NCCALCSIZE:
        // 计算客户区尺寸
        if (wParam == TRUE) {
            return handleCalcSize(reinterpret_cast<NCCALCSIZE_PARAMS*>(lParam));
        }
        break;
        
    case WM_GETMINMAXINFO:
        // 设置窗口最小/最大尺寸
        return handleMinMaxInfo(reinterpret_cast<MINMAXINFO*>(lParam));
    }
    
    return DefWindowProc(hwnd, msg, wParam, lParam);
}
```

### HitTest 性能优化

```cpp
LRESULT HitTestManager::hitTest(const QPoint& point) const {
    // 首先检查边缘调整区域（最高优先级）
    if (point.x() < m_borderWidth) return HTLEFT;
    if (point.x() > m_windowBounds.width() - m_borderWidth) return HTRIGHT;
    if (point.y() < m_borderWidth) return HTTOP;
    if (point.y() > m_windowBounds.height() - m_borderWidth) return HTBOTTOM;
    
    // 使用空间分割优化区域查找
    for (const auto& region : m_regions) {
        if (region.enabled && region.bounds.contains(point)) {
            return region.hitTest;
        }
    }
    
    return HTCLIENT;  // 默认为客户区域
}
```

## 兼容性处理

### Windows 版本适配

```cpp
class WindowsVersionDetector {
public:
    static bool isWindows11OrLater() {
        OSVERSIONINFOEX osvi = {};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        
        // Windows 11 = 10.0.22000+
        return (osvi.dwMajorVersion > 10) || 
               (osvi.dwMajorVersion == 10 && osvi.dwBuildNumber >= 22000);
    }
    
    static bool supportsImmersiveDarkMode() {
        OSVERSIONINFOEX osvi = {};
        osvi.dwOSVersionInfoSize = sizeof(osvi);
        
        // Windows 10 build 18985+
        return (osvi.dwMajorVersion > 10) || 
               (osvi.dwMajorVersion == 10 && osvi.dwBuildNumber >= 18985);
    }
};

void WinWindowChrome::setupWindow(HWND hwnd, bool customFrame) {
    if (customFrame) {
        // 移除默认窗口装饰
        SetWindowLong(hwnd, GWL_STYLE, 
                     GetWindowLong(hwnd, GWL_STYLE) & ~(WS_CAPTION | WS_THICKFRAME));
        
        // Windows 11 特定设置
        if (WindowsVersionDetector::isWindows11OrLater()) {
            setupWindows11Features(hwnd);
        }
        
        // 暗色模式支持
        if (WindowsVersionDetector::supportsImmersiveDarkMode()) {
            enableImmersiveDarkMode(hwnd);
        }
    }
}
```

### DPI 感知处理

```cpp
void WinWindowChrome::handleDpiChange(HWND hwnd, int newDpi) {
    // 更新 DPI 相关的尺寸
    float dpiScale = newDpi / 96.0f;
    
    int newTitleBarHeight = static_cast<int>(40 * dpiScale);
    int newButtonWidth = static_cast<int>(50 * dpiScale);
    int newBorderWidth = static_cast<int>(4 * dpiScale);
    
    // 更新窗口装饰尺寸
    setTitleBarHeight(hwnd, newTitleBarHeight);
    updateControlButtonSizes(hwnd, newButtonWidth);
    updateBorderWidth(hwnd, newBorderWidth);
    
    // 重新计算窗口布局
    SetWindowPos(hwnd, nullptr, 0, 0, 0, 0, 
                SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
}
```

## 常见问题与解决方案

### 标题栏闪烁问题

```cpp
// 问题：主题切换时标题栏出现闪烁
// 解决：使用双缓冲绘制
void ThemedWindowChrome::paintTitleBar(HDC hdc, const QRect& area) {
    // 创建内存 DC 进行离屏绘制
    HDC memDC = CreateCompatibleDC(hdc);
    HBITMAP memBitmap = CreateCompatibleBitmap(hdc, area.width(), area.height());
    HBITMAP oldBitmap = static_cast<HBITMAP>(SelectObject(memDC, memBitmap));
    
    // 在内存 DC 中绘制标题栏
    drawTitleBarContent(memDC, area);
    
    // 一次性复制到屏幕
    BitBlt(hdc, area.left(), area.top(), area.width(), area.height(),
           memDC, 0, 0, SRCCOPY);
    
    // 清理资源
    SelectObject(memDC, oldBitmap);
    DeleteObject(memBitmap);
    DeleteDC(memDC);
}
```

### 窗口拖拽卡顿

```cpp
// 问题：窗口拖拽时出现卡顿
// 解决：优化 WM_NCHITTEST 处理
LRESULT WinWindowChrome::handleHitTest(HWND hwnd, LPARAM lParam) {
    // 使用缓存减少重复计算
    static QPoint lastPoint;
    static LRESULT lastResult;
    
    QPoint currentPoint(GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam));
    
    if (currentPoint == lastPoint) {
        return lastResult;  // 返回缓存结果
    }
    
    lastPoint = currentPoint;
    lastResult = performHitTest(hwnd, currentPoint);
    return lastResult;
}
```

## 相关文档

- [表现层架构概览](../presentation/architecture.md) - UI 系统与窗口装饰的集成
- [渲染与图形系统](../infrastructure/gfx.md) - 自定义绘制在窗口装饰中的应用
- [App Shell 与应用组装](../application/app-shell.md) - 应用外壳与窗口装饰的协调