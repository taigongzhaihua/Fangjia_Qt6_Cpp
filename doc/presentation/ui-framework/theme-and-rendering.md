**English** | [简体中文](../../../doc.zh-cn/presentation/ui-framework/theme-and-rendering.md)

# Theme & Rendering

## Overview

The theme and rendering system in Fangjia provides consistent visual styling, smooth animations, and efficient graphics rendering across all UI components. It supports light/dark mode switching, system theme following, and customizable color palettes.

## Theme Management

### ThemeManager

The central theme management system:

```cpp
class ThemeManager : public QObject {
    Q_OBJECT

public:
    enum class Mode {
        Light,          // Force light theme
        Dark,           // Force dark theme
        FollowSystem    // Follow system theme
    };

    // Theme control
    void setMode(Mode mode);
    Mode currentMode() const { return m_mode; }
    bool isDarkMode() const { return m_isDark; }
    bool isFollowingSystem() const { return m_mode == Mode::FollowSystem; }

    // Color palette access
    const ThemePalette& currentPalette() const;
    QColor getColor(const QString& role) const;

    // System integration
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
    QSystemTrayIcon* m_systemTray; // For system theme detection
};
```

### Theme Palettes

Structured color definitions:

```cpp
struct ThemePalette {
    // Background colors
    QColor windowBackground;
    QColor surfaceBackground;
    QColor cardBackground;
    QColor overlayBackground;

    // Foreground colors
    QColor primaryText;
    QColor secondaryText;
    QColor disabledText;
    QColor linkText;

    // Accent colors
    QColor primaryAccent;
    QColor secondaryAccent;
    QColor successColor;
    QColor warningColor;
    QColor errorColor;

    // Border and divider colors
    QColor borderColor;
    QColor dividerColor;
    QColor focusColor;

    // Interactive states
    QColor hoverColor;
    QColor pressedColor;
    QColor selectedColor;
    QColor disabledColor;

    // Component-specific colors
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

### Default Palettes

```cpp
ThemePalette createLightPalette() {
    ThemePalette palette;
    
    // Backgrounds
    palette.windowBackground = QColor(248, 249, 250);
    palette.surfaceBackground = QColor(255, 255, 255);
    palette.cardBackground = QColor(255, 255, 255);
    palette.overlayBackground = QColor(255, 255, 255, 240);
    
    // Text
    palette.primaryText = QColor(33, 37, 41);
    palette.secondaryText = QColor(108, 117, 125);
    palette.disabledText = QColor(173, 181, 189);
    palette.linkText = QColor(13, 110, 253);
    
    // Accents
    palette.primaryAccent = QColor(13, 110, 253);
    palette.secondaryAccent = QColor(108, 117, 125);
    palette.successColor = QColor(25, 135, 84);
    palette.warningColor = QColor(255, 193, 7);
    palette.errorColor = QColor(220, 53, 69);
    
    // Interactive
    palette.hoverColor = QColor(0, 0, 0, 10);
    palette.pressedColor = QColor(0, 0, 0, 20);
    palette.selectedColor = QColor(13, 110, 253, 30);
    
    return palette;
}

ThemePalette createDarkPalette() {
    ThemePalette palette;
    
    // Backgrounds
    palette.windowBackground = QColor(33, 37, 41);
    palette.surfaceBackground = QColor(52, 58, 64);
    palette.cardBackground = QColor(73, 80, 87);
    palette.overlayBackground = QColor(33, 37, 41, 240);
    
    // Text
    palette.primaryText = QColor(248, 249, 250);
    palette.secondaryText = QColor(173, 181, 189);
    palette.disabledText = QColor(108, 117, 125);
    palette.linkText = QColor(102, 179, 255);
    
    // Accents
    palette.primaryAccent = QColor(102, 179, 255);
    palette.secondaryAccent = QColor(173, 181, 189);
    palette.successColor = QColor(37, 184, 103);
    palette.warningColor = QColor(255, 205, 57);
    palette.errorColor = QColor(244, 67, 89);
    
    // Interactive
    palette.hoverColor = QColor(255, 255, 255, 15);
    palette.pressedColor = QColor(255, 255, 255, 25);
    palette.selectedColor = QColor(102, 179, 255, 30);
    
    return palette;
}
```

## Theme Propagation

### Component Theme Integration

All UI components receive theme updates through the standardized interface:

```cpp
class IUiComponent {
public:
    virtual void onThemeChanged(bool isDark) {}
    
protected:
    // Helper for theme-aware components
    QColor getThemeColor(const QString& role) const {
        return ThemeManager::instance().getColor(role);
    }
    
    const ThemePalette& currentPalette() const {
        return ThemeManager::instance().currentPalette();
    }
};
```

### UiRoot Theme Distribution

```cpp
void UiRoot::propagateThemeChange(bool isDark) {
    m_isDark = isDark;
    
    // Update self
    onThemeChanged(isDark);
    
    // Propagate to all children recursively
    if (m_topBar) {
        m_topBar->onThemeChanged(isDark);
    }
    
    if (m_content) {
        propagateToComponent(m_content.get(), isDark);
    }
    
    // Request visual update
    requestRepaint();
}

void UiRoot::propagateToComponent(IUiComponent* component, bool isDark) {
    component->onThemeChanged(isDark);
    
    // Recursively propagate to child components
    if (auto container = dynamic_cast<IUiContainer*>(component)) {
        for (auto& child : container->children()) {
            propagateToComponent(child.get(), isDark);
        }
    }
}
```

### RebuildHost Environment Sync

For declarative UI components, theme updates are synchronized during rebuilds:

```cpp
void RebuildHost::requestRebuild() {
    if (!m_needsRebuild) {
        m_needsRebuild = true;
        QTimer::singleShot(0, this, &RebuildHost::performRebuild);
    }
}

void RebuildHost::performRebuild() {
    if (!m_component || !m_needsRebuild) return;
    
    // Synchronize environment in correct order:
    // 1. Apply theme first (affects resource keys)
    if (m_themeValid) {
        m_component->onThemeChanged(m_isDark);
    }
    
    // 2. Update resource context (with correct theme)
    if (m_resourceContextValid) {
        m_component->updateResourceContext(*m_iconCache, m_gl, m_devicePixelRatio);
    }
    
    // 3. Update layout last (may depend on resources)
    if (m_viewportValid) {
        m_component->updateLayout(m_viewport);
    }
    
    m_needsRebuild = false;
}
```

## Rendering Integration

### Theme-Aware Rendering

Components apply theme colors during render command generation:

```cpp
void UiButton::append(Render::FrameData& frameData) const {
    const auto& palette = currentPalette();
    
    // Determine button state colors
    QColor bgColor = palette.surfaceBackground;
    if (m_isPressed) {
        bgColor = palette.pressedColor;
    } else if (m_isHovered) {
        bgColor = palette.hoverColor;
    }
    
    // Background rectangle
    frameData.addRect(m_bounds, bgColor, m_cornerRadius);
    
    // Text with theme-appropriate color
    QColor textColor = m_isEnabled ? palette.primaryText : palette.disabledText;
    frameData.addText(m_text, m_textPosition, m_font, textColor);
    
    // Icon with theme tinting
    if (!m_iconPath.isEmpty()) {
        QColor iconColor = m_isEnabled ? palette.primaryText : palette.disabledText;
        frameData.addIcon(m_iconPath, m_iconBounds, iconColor);
    }
}
```

### Resource Context Updates

IconCache responds to theme changes by invalidating cached resources:

```cpp
void IconCache::onThemeChanged(bool isDark) {
    if (m_isDark != isDark) {
        // Theme change invalidates all cached icons (color tinting changes)
        clearIconCache();
        m_isDark = isDark;
    }
}

QOpenGLTexture* IconCache::getIconTexture(const QString& path, const QSize& size, const QColor& tint) {
    // Include theme state in cache key
    QString cacheKey = QString("%1_%2x%3_%4_%5_%6")
        .arg(path)
        .arg(size.width()).arg(size.height())
        .arg(tint.name())
        .arg(QString::number(m_devicePixelRatio, 'f', 1))
        .arg(m_isDark ? "dark" : "light");
    
    // Check cache or create new texture
    auto it = m_iconTextures.find(cacheKey);
    if (it != m_iconTextures.end()) {
        return it->second;
    }
    
    auto texture = createIconTexture(path, size, tint);
    m_iconTextures[cacheKey] = texture;
    return texture;
}
```

## Animation & Transitions

### Theme Transition Animations

Smooth transitions when switching themes:

```cpp
class ThemeTransitionManager {
public:
    void startThemeTransition(bool toDark, int duration = 300) {
        if (m_activeTransition) {
            m_activeTransition->stop();
        }
        
        m_activeTransition = new QPropertyAnimation(this, "transitionProgress");
        m_activeTransition->setDuration(duration);
        m_activeTransition->setStartValue(0.0);
        m_activeTransition->setEndValue(1.0);
        m_activeTransition->setEasingCurve(QEasingCurve::OutCubic);
        
        m_fromDark = !toDark;
        m_toDark = toDark;
        
        connect(m_activeTransition, &QPropertyAnimation::valueChanged, 
                this, &ThemeTransitionManager::updateTransition);
        connect(m_activeTransition, &QPropertyAnimation::finished,
                this, &ThemeTransitionManager::finishTransition);
        
        m_activeTransition->start();
    }
    
private:
    void updateTransition(const QVariant& value) {
        float progress = value.toFloat();
        
        // Interpolate theme colors
        ThemePalette interpolated = interpolatePalettes(
            m_fromDark ? m_darkPalette : m_lightPalette,
            m_toDark ? m_darkPalette : m_lightPalette,
            progress
        );
        
        // Apply interpolated theme
        emit transitionPaletteChanged(interpolated);
    }
    
    ThemePalette interpolatePalettes(const ThemePalette& from, const ThemePalette& to, float t) {
        ThemePalette result;
        result.windowBackground = lerpColor(from.windowBackground, to.windowBackground, t);
        result.surfaceBackground = lerpColor(from.surfaceBackground, to.surfaceBackground, t);
        // ... interpolate all colors
        return result;
    }
    
    QColor lerpColor(const QColor& from, const QColor& to, float t) {
        return QColor(
            static_cast<int>(from.red() + (to.red() - from.red()) * t),
            static_cast<int>(from.green() + (to.green() - from.green()) * t),
            static_cast<int>(from.blue() + (to.blue() - from.blue()) * t),
            static_cast<int>(from.alpha() + (to.alpha() - from.alpha()) * t)
        );
    }
};
```

### Component Animation Integration

Components can implement smooth theme transitions:

```cpp
class AnimatedThemeComponent : public IUiComponent {
protected:
    void onThemeChanged(bool isDark) override {
        const auto& newPalette = currentPalette();
        
        if (m_themeAnimation) {
            m_themeAnimation->stop();
        }
        
        // Animate color changes
        m_themeAnimation = new QPropertyAnimation(this, "animatedBackgroundColor");
        m_themeAnimation->setDuration(200);
        m_themeAnimation->setStartValue(m_currentBackgroundColor);
        m_themeAnimation->setEndValue(newPalette.surfaceBackground);
        m_themeAnimation->setEasingCurve(QEasingCurve::OutCubic);
        
        connect(m_themeAnimation, &QPropertyAnimation::valueChanged,
                this, [this]() { requestRepaint(); });
        
        m_themeAnimation->start();
    }
    
    Q_PROPERTY(QColor animatedBackgroundColor READ currentBackgroundColor WRITE setCurrentBackgroundColor)
    
    QColor currentBackgroundColor() const { return m_currentBackgroundColor; }
    void setCurrentBackgroundColor(const QColor& color) { m_currentBackgroundColor = color; }
    
private:
    QColor m_currentBackgroundColor;
    QPropertyAnimation* m_themeAnimation = nullptr;
};
```

## Custom Styling

### Style Customization API

```cpp
class StyleManager {
public:
    // Component style overrides
    void setComponentStyle(const QString& componentType, const QString& styleSheet);
    void setComponentProperty(const QString& componentType, const QString& property, const QVariant& value);
    
    // Global style properties
    void setGlobalProperty(const QString& property, const QVariant& value);
    QVariant getGlobalProperty(const QString& property) const;
    
    // Custom palettes
    void registerCustomPalette(const QString& name, const ThemePalette& palette);
    void setActivePalette(const QString& name);
    
private:
    QHash<QString, QHash<QString, QVariant>> m_componentStyles;
    QHash<QString, QVariant> m_globalProperties;
    QHash<QString, ThemePalette> m_customPalettes;
};
```

### CSS-like Styling

```cpp
// Example: Custom button styling
styleManager.setComponentStyle("UiButton", R"(
    background-color: @primaryAccent;
    text-color: @windowBackground;
    border-radius: 8px;
    padding: 12px 24px;
    
    &:hover {
        background-color: @primaryAccent.lighter(110%);
    }
    
    &:pressed {
        background-color: @primaryAccent.darker(110%);
    }
    
    &:disabled {
        background-color: @disabledColor;
        text-color: @disabledText;
    }
)");
```

## Performance Optimization

### Theme Caching

```cpp
class ThemeCache {
public:
    // Cached color calculations
    QColor getCachedColor(const QString& role, bool isDark) const;
    void setCachedColor(const QString& role, bool isDark, const QColor& color);
    
    // Cached gradients and brushes
    QLinearGradient getCachedGradient(const QString& name, bool isDark) const;
    QBrush getCachedBrush(const QString& name, bool isDark) const;
    
    // Cache invalidation
    void invalidateCache();
    void invalidateForTheme(bool isDark);
    
private:
    struct CacheKey {
        QString role;
        bool isDark;
        bool operator==(const CacheKey& other) const {
            return role == other.role && isDark == other.isDark;
        }
    };
    
    QHash<CacheKey, QColor> m_colorCache;
    QHash<CacheKey, QLinearGradient> m_gradientCache;
    QHash<CacheKey, QBrush> m_brushCache;
};
```

### Render State Optimization

```cpp
class RenderStateManager {
public:
    // Minimize theme-related state changes
    void pushThemeState(const ThemePalette& palette);
    void popThemeState();
    
    // Batch theme updates
    void beginThemeUpdate();
    void endThemeUpdate();
    
    // Optimize color conversions
    QColor getNormalizedColor(const QColor& color) const;
    
private:
    std::stack<ThemePalette> m_paletteStack;
    bool m_inBatchUpdate = false;
    QHash<QRgb, QColor> m_normalizedColors;
};
```

## Related Documentation

- [UI Framework Overview](overview.md)
- [Graphics & Rendering System](../../infrastructure/gfx.md)
- [TopBar Component](../components/top-bar.md)
- [Architecture Overview](../../architecture/overview.md)