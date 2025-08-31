**English** | [简体中文](../../doc.zh-cn/infrastructure/gfx.md)

# Graphics & Rendering System

This document introduces the rendering pipeline in the Fangjia Qt6 C++ framework, including the responsibilities and usage of RenderData, IconCache, and RenderUtils, as well as OpenGL rendering optimization strategies.

## Rendering Pipeline Overview

### Rendering Architecture Design

The framework adopts a "command collection + batch rendering" architecture:

1. **Command Collection Phase**: UI components add drawing requirements to `Render::FrameData`
2. **Resource Preparation Phase**: `IconCache` ensures required textures are loaded to GPU
3. **Batch Rendering Phase**: `Renderer` executes all drawing commands, minimizing state switches

```
UI Components → RenderData → IconCache → Renderer → GPU
     ↓             ↓          ↓           ↓        ↓
Generate Commands Collect   Prepare     Batch    Display
                 Commands   Textures    Render
```

### Core Components

#### Render::FrameData
Central command collection structure that accumulates all rendering operations for a single frame:

```cpp
namespace Render {
    struct FrameData {
        std::vector<RectCmd> rects;        // Rectangle drawing commands
        std::vector<ImageCmd> images;      // Image/texture drawing commands
        std::vector<TextCmd> texts;        // Text rendering commands
        
        // Command collection interface
        void addRect(const QRectF& rect, const QColor& color, float cornerRadius = 0.0f);
        void addImage(const QRectF& dest, int textureId, const QRectF& source = QRectF());
        void addText(const QRectF& rect, const QString& text, const QFont& font, const QColor& color);
    };
}
```

#### IconCache
Texture management system that handles SVG rasterization and GPU texture caching:

```cpp
class IconCache {
public:
    // Ensure texture exists and return texture ID
    int ensureSvgPx(const QString& key, const QByteArray& svgData, const QSize& size, QOpenGLFunctions* gl);
    
    // Get texture size in pixels
    QSize textureSizePx(int textureId) const;
    
    // Clear unused textures
    void cleanupUnused(QOpenGLFunctions* gl);
    
private:
    struct TextureEntry {
        GLuint textureId;
        QSize sizePx;
        qint64 lastUsed;
    };
    
    std::unordered_map<QString, TextureEntry> m_textures;
};
```

#### RenderUtils
Utility functions for common rendering operations:

```cpp
class RenderUtils {
public:
    // Generate cache key for icon
    static QString makeIconCacheKey(const QString& baseName, int sizePx);
    
    // Load and cache SVG data
    static QByteArray loadSvgCached(const QString& path);
    
    // Color manipulation utilities
    static QColor blendColors(const QColor& base, const QColor& overlay, float alpha);
    static QColor adjustBrightness(const QColor& color, float factor);
    
    // Coordinate system utilities
    static QRectF deviceToLogical(const QRectF& deviceRect, float devicePixelRatio);
    static QRectF logicalToDevice(const QRectF& logicalRect, float devicePixelRatio);
};
```

## Component Integration Patterns

### UI Component Rendering

```cpp
void UiButton::append(Render::FrameData& fd) const {
    // 1. Background rendering
    if (m_backgroundColor.isValid()) {
        fd.addRect(m_bounds, m_backgroundColor, m_cornerRadius);
    }
    
    // 2. Icon rendering (if present)
    if (m_iconTextureId > 0) {
        QRectF iconRect = calculateIconRect();
        fd.addImage(iconRect, m_iconTextureId);
    }
    
    // 3. Text rendering
    if (!m_text.isEmpty()) {
        QRectF textRect = calculateTextRect();
        fd.addText(textRect, m_text, m_font, m_textColor);
    }
    
    // 4. Border rendering (if enabled)
    if (m_borderWidth > 0) {
        fd.addRect(m_bounds, m_borderColor, m_cornerRadius, m_borderWidth);
    }
}
```

### Icon Management Pattern

```cpp
void UiComponent::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) {
    if (!m_iconPath.isEmpty()) {
        // Calculate appropriate icon size based on DPI
        const int logicalSize = 24;  // Design size
        const int pixelSize = std::lround(logicalSize * devicePixelRatio);
        
        // Generate cache key
        const QString cacheKey = RenderUtils::makeIconCacheKey("button_icon", pixelSize);
        
        // Load and ensure texture
        const QByteArray svgData = RenderUtils::loadSvgCached(m_iconPath);
        m_iconTextureId = cache.ensureSvgPx(cacheKey, svgData, QSize(pixelSize, pixelSize), gl);
        
        // Store logical size for layout calculations
        m_iconLogicalSize = QSize(logicalSize, logicalSize);
    }
}
```

### Theme-Aware Rendering

```cpp
void UiThemedComponent::onThemeChanged(bool isDark) {
    // Update colors based on theme
    m_backgroundColor = isDark ? QColor(45, 45, 45) : QColor(250, 250, 250);
    m_textColor = isDark ? QColor(255, 255, 255) : QColor(0, 0, 0);
    m_borderColor = isDark ? QColor(70, 70, 70) : QColor(200, 200, 200);
    
    // Update icon textures if theme-dependent
    if (m_hasThemeIcons) {
        m_iconPath = isDark ? m_darkIconPath : m_lightIconPath;
        m_iconTextureId = 0;  // Force reload in next updateResourceContext
    }
}
```

## Performance Optimization Strategies

### Texture Management

#### Efficient Cache Keys
```cpp
QString RenderUtils::makeIconCacheKey(const QString& baseName, int sizePx) {
    return QString("%1_%2px").arg(baseName).arg(sizePx);
}

// Usage pattern that enables sharing
QString buttonKey = RenderUtils::makeIconCacheKey("close", 16);  // Shared across all close buttons
QString navKey = RenderUtils::makeIconCacheKey("home", 24);      // Shared across all home icons
```

#### Batch Resource Loading
```cpp
class ComponentResourceManager {
public:
    void preloadCommonIcons(IconCache& cache, QOpenGLFunctions* gl, float dpr) {
        struct IconSpec { QString name; QString path; int logicalSize; };
        
        std::vector<IconSpec> commonIcons = {
            {"close", ":/icons/window/close.svg", 16},
            {"minimize", ":/icons/window/minimize.svg", 16},
            {"maximize", ":/icons/window/maximize.svg", 16},
            {"home", ":/icons/nav/home.svg", 24},
            {"settings", ":/icons/nav/settings.svg", 24}
        };
        
        for (const auto& icon : commonIcons) {
            int pixelSize = std::lround(icon.logicalSize * dpr);
            QString key = RenderUtils::makeIconCacheKey(icon.name, pixelSize);
            QByteArray svgData = RenderUtils::loadSvgCached(icon.path);
            cache.ensureSvgPx(key, svgData, QSize(pixelSize, pixelSize), gl);
        }
    }
};
```

### Command Optimization

#### Command Batching
```cpp
class FrameOptimizer {
public:
    static void optimizeFrameData(Render::FrameData& fd) {
        // Sort commands by type for better GPU batching
        std::stable_sort(fd.rects.begin(), fd.rects.end(), 
            [](const Render::RectCmd& a, const Render::RectCmd& b) {
                return a.color.rgba() < b.color.rgba();  // Group by color
            });
        
        std::stable_sort(fd.images.begin(), fd.images.end(),
            [](const Render::ImageCmd& a, const Render::ImageCmd& b) {
                return a.textureId < b.textureId;  // Group by texture
            });
        
        // Merge adjacent identical rectangle commands
        mergeAdjacentRects(fd.rects);
    }
    
private:
    static void mergeAdjacentRects(std::vector<Render::RectCmd>& rects) {
        // Implementation for merging compatible adjacent rectangles
        // to reduce draw calls
    }
};
```

## Integration with Qt6 OpenGL

### Context Management
```cpp
class RenderContext {
public:
    void initialize(QOpenGLContext* context) {
        m_gl = context->functions();
        m_extraFunctions = context->extraFunctions();
        
        // Setup default OpenGL state
        m_gl->glEnable(GL_BLEND);
        m_gl->glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        m_gl->glDisable(GL_DEPTH_TEST);
        m_gl->glDisable(GL_CULL_FACE);
    }
    
    void beginFrame(const QSize& framebufferSize) {
        m_gl->glViewport(0, 0, framebufferSize.width(), framebufferSize.height());
        m_gl->glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        m_gl->glClear(GL_COLOR_BUFFER_BIT);
    }
    
    void endFrame() {
        // Flush any pending commands
        m_gl->glFlush();
    }
    
private:
    QOpenGLFunctions* m_gl = nullptr;
    QOpenGLExtraFunctions* m_extraFunctions = nullptr;
};
```

### Shader Management
```cpp
class ShaderProgram {
public:
    bool compile(const QString& vertexSource, const QString& fragmentSource) {
        m_program = std::make_unique<QOpenGLShaderProgram>();
        
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Vertex, vertexSource)) {
            return false;
        }
        
        if (!m_program->addShaderFromSourceCode(QOpenGLShader::Fragment, fragmentSource)) {
            return false;
        }
        
        if (!m_program->link()) {
            return false;
        }
        
        // Cache uniform locations
        m_uniformLocations["mvpMatrix"] = m_program->uniformLocation("mvpMatrix");
        m_uniformLocations["color"] = m_program->uniformLocation("color");
        m_uniformLocations["texture"] = m_program->uniformLocation("texture");
        
        return true;
    }
    
    void bind() {
        m_program->bind();
    }
    
    void setUniform(const QString& name, const QMatrix4x4& matrix) {
        auto it = m_uniformLocations.find(name);
        if (it != m_uniformLocations.end()) {
            m_program->setUniformValue(it->second, matrix);
        }
    }
    
private:
    std::unique_ptr<QOpenGLShaderProgram> m_program;
    std::unordered_map<QString, int> m_uniformLocations;
};
```

## Common Usage Patterns

### Simple Icon Rendering
```cpp
void setupSvgIcon(IconCache& cache, QOpenGLFunctions* gl, float dpr) {
    const QString iconPath = ":/icons/example.svg";
    const int logicalSize = 24;
    const int pixelSize = std::lround(logicalSize * dpr);
    
    const QString key = RenderUtils::makeIconCacheKey("example", pixelSize);
    const QByteArray svgData = RenderUtils::loadSvgCached(iconPath);
    
    m_textureId = cache.ensureSvgPx(key, svgData, QSize(pixelSize, pixelSize), gl);
}

void renderIcon(Render::FrameData& fd, const QRectF& rect) {
    if (m_textureId > 0) {
        fd.addImage(rect, m_textureId);
    }
}
```

### Complex UI Component
```cpp
void UiCard::append(Render::FrameData& fd) const {
    // Background with rounded corners
    fd.addRect(m_bounds, m_backgroundColor, 8.0f);
    
    // Header area
    QRectF headerRect(m_bounds.x(), m_bounds.y(), m_bounds.width(), 48);
    fd.addRect(headerRect, m_headerColor, 8.0f); // Only top corners rounded
    
    // Icon
    if (m_iconTextureId > 0) {
        QRectF iconRect(headerRect.x() + 16, headerRect.y() + 12, 24, 24);
        fd.addImage(iconRect, m_iconTextureId);
    }
    
    // Title text
    QRectF titleRect(headerRect.x() + 56, headerRect.y(), headerRect.width() - 72, headerRect.height());
    fd.addText(titleRect, m_title, m_titleFont, m_titleColor);
    
    // Content area
    QRectF contentRect(m_bounds.x() + 16, headerRect.bottom() + 16, 
                      m_bounds.width() - 32, m_bounds.height() - headerRect.height() - 32);
    fd.addText(contentRect, m_content, m_contentFont, m_contentColor);
    
    // Border (optional)
    if (m_borderWidth > 0) {
        fd.addRect(m_bounds, m_borderColor, 8.0f, m_borderWidth);
    }
}
```

## Related Documentation

- [Presentation Architecture Overview](../presentation/architecture.md) - How graphics system integrates with UI components
- [Windows Platform Integration](platform-windows.md) - Platform-specific rendering considerations
- [Theme & Rendering](../presentation/ui-framework/theme-and-rendering.md) - Theme management and color systems