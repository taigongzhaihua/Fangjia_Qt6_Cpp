[English](../../doc/presentation/ui-framework/theme-and-rendering.md) | **简体中文**

# 主题与渲染

## 概览

Fangjia 的主题和渲染系统为所有 UI 组件提供一致的视觉样式、流畅的动画和高效的图形渲染。它支持明/暗模式切换、跟随系统主题和可自定义的调色板。

## 主题管理

### ThemeManager

中央主题管理系统：

```cpp
class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum class Mode {
        Light,          // 强制明亮主题
        Dark,           // 强制暗黑主题
        FollowSystem    // 跟随系统主题
    };

    // 主题控制
    void setMode(Mode mode);
    Mode currentMode() const { return m_mode; }
    bool isDarkMode() const { return m_isDark; }
    bool isFollowingSystem() const { return m_mode == Mode::FollowSystem; }

    // 调色板访问
    const ThemePalette& currentPalette() const;
    QColor getColor(const QString& role) const;

    // 系统集成
    void detectSystemTheme();
    void listenForSystemChanges();

signals:
    void themeChanged(bool isDark);
    void modeChanged(Mode mode);

private:
    Mode m_mode = Mode::FollowSystem;
    bool m_isDark = false;
    ThemePalette m_lightPalette;
    ThemePalette m_darkPalette;
    QSystemTrayIcon* m_systemTray; // 用于系统主题检测
};
```

### 主题调色板

结构化的颜色定义：

```cpp
struct ThemePalette {
    // 背景颜色
    QColor windowBackground;
    QColor surfaceBackground;
    QColor cardBackground;
    QColor overlayBackground;

    // 前景颜色
    QColor primaryText;
    QColor secondaryText;
    QColor disabledText;
    QColor linkText;

    // 强调颜色
    QColor primaryAccent;
    QColor secondaryAccent;
    QColor successColor;
    QColor warningColor;
    QColor errorColor;

    // 边框和分隔线颜色
    QColor borderColor;
    QColor dividerColor;
    QColor focusColor;

    // 交互状态
    QColor hoverColor;
    QColor pressedColor;
    QColor selectedColor;
    QColor disabledColor;

    // 组件特定颜色
    struct {
        QColor background;
        QColor buttonBackground;
        QColor iconColor;
        QColor buttonHover;
        QColor buttonPressed;
    } topBar;

    struct {
        QColor background;
        QColor itemBackground;
        QColor activeBackground;
        QColor indicatorColor;
        QColor iconColor;
        QColor textColor;
    } navigation;

    struct {
        QColor tabBackground;
        QColor activeTabBackground;
        QColor contentBackground;
        QColor borderColor;
        QColor closeButtonColor;
    } tabView;
};
```

### 默认调色板

```cpp
ThemePalette createLightPalette() {
    ThemePalette palette;
    
    // 背景
    palette.windowBackground = QColor(248, 249, 250);
    palette.surfaceBackground = QColor(255, 255, 255);
    palette.cardBackground = QColor(255, 255, 255);
    palette.overlayBackground = QColor(255, 255, 255, 240);
    
    // 文本
    palette.primaryText = QColor(33, 37, 41);
    palette.secondaryText = QColor(108, 117, 125);
    palette.disabledText = QColor(173, 181, 189);
    palette.linkText = QColor(13, 110, 253);
    
    // 强调色
    palette.primaryAccent = QColor(13, 110, 253);
    palette.secondaryAccent = QColor(108, 117, 125);
    palette.successColor = QColor(25, 135, 84);
    palette.warningColor = QColor(255, 193, 7);
    palette.errorColor = QColor(220, 53, 69);
    
    // 交互
    palette.hoverColor = QColor(0, 0, 0, 10);
    palette.pressedColor = QColor(0, 0, 0, 20);
    palette.selectedColor = QColor(13, 110, 253, 30);
    
    return palette;
}

ThemePalette createDarkPalette() {
    ThemePalette palette;
    
    // 背景
    palette.windowBackground = QColor(13, 17, 23);
    palette.surfaceBackground = QColor(22, 27, 34);
    palette.cardBackground = QColor(32, 39, 48);
    palette.overlayBackground = QColor(22, 27, 34, 240);
    
    // 文本
    palette.primaryText = QColor(240, 246, 252);
    palette.secondaryText = QColor(139, 148, 158);
    palette.disabledText = QColor(89, 96, 104);
    palette.linkText = QColor(88, 166, 255);
    
    // 强调色
    palette.primaryAccent = QColor(88, 166, 255);
    palette.secondaryAccent = QColor(139, 148, 158);
    palette.successColor = QColor(63, 185, 80);
    palette.warningColor = QColor(187, 128, 9);
    palette.errorColor = QColor(248, 81, 73);
    
    // 交互
    palette.hoverColor = QColor(255, 255, 255, 10);
    palette.pressedColor = QColor(255, 255, 255, 20);
    palette.selectedColor = QColor(88, 166, 255, 30);
    
    return palette;
}
```

## 组件主题化

### 主题感知基类

```cpp
class ThemeAwareComponent : public IUiComponent {
public:
    void onThemeChanged(bool isDark) override {
        m_isDark = isDark;
        applyTheme();
        requestRepaint();
    }

protected:
    // 便捷的颜色访问器
    QColor backgroundColor() const {
        return m_isDark ? m_darkPalette.surfaceBackground : m_lightPalette.surfaceBackground;
    }
    
    QColor textColor() const {
        return m_isDark ? m_darkPalette.primaryText : m_lightPalette.primaryText;
    }
    
    QColor accentColor() const {
        return m_isDark ? m_darkPalette.primaryAccent : m_lightPalette.primaryAccent;
    }
    
    QColor borderColor() const {
        return m_isDark ? m_darkPalette.borderColor : m_lightPalette.borderColor;
    }

    // 子类重写以应用主题
    virtual void applyTheme() = 0;

private:
    bool m_isDark = false;
    ThemePalette m_lightPalette;
    ThemePalette m_darkPalette;
};
```

### 动态颜色插值

```cpp
class ColorInterpolator {
public:
    // 在明暗主题颜色之间平滑过渡
    static QColor interpolateThemeColors(const QColor& lightColor, 
                                       const QColor& darkColor, 
                                       float progress) {
        return QColor(
            lightColor.red() + (darkColor.red() - lightColor.red()) * progress,
            lightColor.green() + (darkColor.green() - lightColor.green()) * progress,
            lightColor.blue() + (darkColor.blue() - lightColor.blue()) * progress,
            lightColor.alpha() + (darkColor.alpha() - lightColor.alpha()) * progress
        );
    }
    
    // 基于时间的颜色过渡
    static QColor animateColor(const QColor& fromColor, 
                              const QColor& toColor, 
                              qint64 elapsed, 
                              int duration) {
        float progress = qBound(0.0f, static_cast<float>(elapsed) / duration, 1.0f);
        
        // 应用缓动函数
        progress = QEasingCurve(QEasingCurve::OutCubic).valueForProgress(progress);
        
        return interpolateThemeColors(fromColor, toColor, progress);
    }
};
```

## 渲染优化

### 渲染管线

```cpp
class RenderPipeline {
public:
    // 渲染阶段
    enum class Stage {
        Background,    // 背景和基础几何
        Content,       // 文本和图标
        Overlay,       // 覆盖层和特效
        Debug          // 调试信息
    };
    
    // 渲染队列管理
    void addRenderCommand(Stage stage, const RenderCommand& command);
    void executeStage(Stage stage);
    void executeAll();
    
    // 优化
    void enableBatching(bool enabled);
    void enableCulling(bool enabled);
    void setViewport(const QRect& viewport);
    
private:
    QHash<Stage, QVector<RenderCommand>> m_renderQueues;
    QRect m_viewport;
    bool m_batchingEnabled = true;
    bool m_cullingEnabled = true;
};
```

### 图标主题化

```cpp
class ThemedIconCache : public IconCache {
public:
    // 主题感知图标加载
    QOpenGLTexture* getThemedIcon(const QString& iconPath, 
                                const QSize& size, 
                                bool isDark) {
        QString themeKey = QString("%1_%2_%3x%4")
                          .arg(iconPath)
                          .arg(isDark ? "dark" : "light")
                          .arg(size.width())
                          .arg(size.height());
        
        auto it = m_themedCache.find(themeKey);
        if (it != m_themedCache.end()) {
            return it.value();
        }
        
        // 加载并应用主题
        QOpenGLTexture* texture = loadAndThemeIcon(iconPath, size, isDark);
        m_themedCache[themeKey] = texture;
        return texture;
    }
    
    void onThemeChanged(bool isDark) override {
        // 仅清理相反主题的缓存
        QString oldTheme = isDark ? "light" : "dark";
        auto it = m_themedCache.begin();
        while (it != m_themedCache.end()) {
            if (it.key().contains(oldTheme)) {
                delete it.value();
                it = m_themedCache.erase(it);
            } else {
                ++it;
            }
        }
    }

private:
    QHash<QString, QOpenGLTexture*> m_themedCache;
    
    QOpenGLTexture* loadAndThemeIcon(const QString& path, 
                                   const QSize& size, 
                                   bool isDark);
};
```

## 动画系统

### 主题过渡动画

```cpp
class ThemeTransitionAnimator : public QObject {
    Q_OBJECT

public:
    // 过渡配置
    void setTransitionDuration(int ms) { m_duration = ms; }
    void setEasingCurve(QEasingCurve::Type easing) { m_easing = easing; }
    
    // 开始主题过渡
    void startTransition(bool tooDark) {
        m_targetDark = tooDark;
        m_animation->setDuration(m_duration);
        m_animation->setEasingCurve(m_easing);
        m_animation->start();
    }

private slots:
    void onAnimationValueChanged(const QVariant& value) {
        float progress = value.toFloat();
        emit transitionProgress(progress, m_targetDark);
    }
    
    void onAnimationFinished() {
        emit transitionFinished(m_targetDark);
    }

signals:
    void transitionProgress(float progress, bool toDark);
    void transitionFinished(bool isDark);

private:
    QPropertyAnimation* m_animation;
    int m_duration = 200;
    QEasingCurve::Type m_easing = QEasingCurve::OutCubic;
    bool m_targetDark = false;
};
```

### 组件动画状态

```cpp
class AnimatedComponent : public ThemeAwareComponent {
public:
    // 动画状态
    enum class AnimationState {
        Idle,
        Hovering,
        Pressing,
        Transitioning
    };
    
    void startHoverAnimation() {
        if (m_animationState != AnimationState::Hovering) {
            m_animationState = AnimationState::Hovering;
            animateToState(m_animationState);
        }
    }
    
    void startPressAnimation() {
        m_animationState = AnimationState::Pressing;
        animateToState(m_animationState);
    }
    
    void returnToIdle() {
        m_animationState = AnimationState::Idle;
        animateToState(m_animationState);
    }

protected:
    virtual void animateToState(AnimationState state) = 0;
    
    QColor getStateColor(AnimationState state) const {
        switch (state) {
        case AnimationState::Idle:
            return backgroundColor();
        case AnimationState::Hovering:
            return blendColors(backgroundColor(), hoverColor(), 0.1f);
        case AnimationState::Pressing:
            return blendColors(backgroundColor(), pressedColor(), 0.2f);
        default:
            return backgroundColor();
        }
    }

private:
    AnimationState m_animationState = AnimationState::Idle;
    
    QColor blendColors(const QColor& base, const QColor& overlay, float alpha) const {
        return QColor(
            base.red() + (overlay.red() - base.red()) * alpha,
            base.green() + (overlay.green() - base.green()) * alpha,
            base.blue() + (overlay.blue() - base.blue()) * alpha
        );
    }
};
```

## 系统集成

### 系统主题检测

```cpp
class SystemThemeWatcher : public QObject {
    Q_OBJECT

public:
    // 开始监视系统主题变化
    void startWatching() {
#ifdef Q_OS_WIN
        watchWindowsTheme();
#elif defined(Q_OS_MAC)
        watchMacOSTheme();
#elif defined(Q_OS_LINUX)
        watchLinuxTheme();
#endif
    }
    
    bool isSystemDarkMode() const {
#ifdef Q_OS_WIN
        return isWindowsDarkMode();
#elif defined(Q_OS_MAC)
        return isMacOSDarkMode();
#elif defined(Q_OS_LINUX)
        return isLinuxDarkMode();
#else
        return false;
#endif
    }

signals:
    void systemThemeChanged(bool isDark);

private:
#ifdef Q_OS_WIN
    void watchWindowsTheme();
    bool isWindowsDarkMode() const;
#endif

#ifdef Q_OS_MAC
    void watchMacOSTheme();
    bool isMacOSDarkMode() const;
#endif

#ifdef Q_OS_LINUX
    void watchLinuxTheme();
    bool isLinuxDarkMode() const;
#endif
};
```

### 高 DPI 适配

```cpp
class DpiAwareRenderer {
public:
    // DPI 缩放
    void setDevicePixelRatio(float dpr) {
        if (m_devicePixelRatio != dpr) {
            m_devicePixelRatio = dpr;
            updateScaling();
        }
    }
    
    // 缩放感知的绘制
    void drawScaledRect(const QRectF& rect, const QColor& color) {
        QRectF scaledRect = rect * m_devicePixelRatio;
        // 渲染到高 DPI 缓冲区
        drawRect(scaledRect, color);
    }
    
    void drawScaledText(const QString& text, const QPointF& pos, const QFont& font) {
        QFont scaledFont = font;
        scaledFont.setPointSizeF(font.pointSizeF() * m_devicePixelRatio);
        QPointF scaledPos = pos * m_devicePixelRatio;
        drawText(text, scaledPos, scaledFont);
    }

private:
    float m_devicePixelRatio = 1.0f;
    
    void updateScaling() {
        // 更新渲染管线的缩放设置
        updateProjectionMatrix();
        invalidateTextureCache();
    }
};
```

## 相关文档

- [UI 框架概览](overview.md)
- [布局系统](layouts.md)
- [数据绑定](../binding.md)
- [图形与渲染](../../infrastructure/gfx.md)