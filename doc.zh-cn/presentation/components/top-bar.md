[English](../../../doc/presentation/components/top-bar.md) | **简体中文**

# TopBar 组件

## 概览

TopBar 组件提供应用程序顶部的窗口控制和导航功能。它集成了窗口操作按钮（最小化、最大化、关闭）、主题切换、系统集成以及声明式 API 支持。

## 核心特性

- **窗口控制**: 集成的最小化、最大化/还原、关闭按钮
- **主题切换**: 内置明/暗主题切换功能
- **系统集成**: 与操作系统窗口装饰的无缝集成
- **声明式 API**: 支持现代链式 API 和组件装饰器系统
- **自定义内容**: 支持在标题栏区域添加自定义控件

## 基本用法

### 标准 TopBar

```cpp
auto topBar = std::make_unique<UiTopBar>();

// 设置应用程序标题
topBar->setTitle("Fangjia Qt6 C++");

// 配置窗口控制按钮
topBar->setShowMinimizeButton(true);
topBar->setShowMaximizeButton(true);
topBar->setShowCloseButton(true);

// 启用主题切换按钮
topBar->setShowThemeToggle(true);
```

### 事件处理

```cpp
// 窗口控制事件
topBar->onMinimizeClicked = []() {
    // 最小化窗口
};

topBar->onMaximizeClicked = []() {
    // 最大化/还原窗口
};

topBar->onCloseClicked = []() {
    // 关闭应用程序
};

// 主题切换事件
topBar->onThemeToggled = [](bool isDark) {
    // 处理主题变化
    ThemeManager::instance().setDarkMode(isDark);
};
```

## 系统集成

### Windows 平台集成

```cpp
class WindowsTopBar : public UiTopBar {
public:
    void updateWindowChrome() {
        if (!m_windowChrome) return;
        
        // 向 Windows 注册标题栏按钮
        m_windowChrome->addCaptionButton("minimize", m_btnMin.baseRect());
        m_windowChrome->addCaptionButton("maximize", m_btnMax.baseRect());
        m_windowChrome->addCaptionButton("close", m_btnClose.baseRect());
        
        // 设置可拖拽标题栏区域
        QRect titleBarRegion = bounds();
        titleBarRegion.setRight(m_btnMin.baseRect().left() - 8);
        m_windowChrome->setHitTestRegion(titleBarRegion, HitTestResult::Caption);
    }
};
```

## 主题适应

```cpp
void UiTopBar::onThemeChanged(bool isDark) {
    const auto& palette = currentPalette();
    
    // 更新背景色
    setBackgroundColor(palette.topBar.background);
    
    // 更新按钮颜色
    m_btnMin.setColor(palette.topBar.iconColor);
    m_btnMax.setColor(palette.topBar.iconColor);
    m_btnClose.setColor(palette.topBar.iconColor);
    
    // 更新标题文本颜色
    setTitleColor(palette.primaryText);
    
    // 更新主题切换按钮状态
    if (m_themeToggle) {
        m_themeToggle->setChecked(isDark);
    }
}
```

## 相关文档

- [Windows 平台集成](../../platform/windows.md)
- [主题与渲染](../ui-framework/theme-and-rendering.md)
- [声明式 TopBar](../ui/topbar/declarative-topbar.md)