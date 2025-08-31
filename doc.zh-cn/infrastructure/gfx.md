[English](../../doc/infrastructure/gfx.md) | **简体中文**

# 渲染与图形系统

本文档介绍 Fangjia Qt6 C++ 框架中的渲染管线，包括 RenderData、IconCache、RenderUtils 的职责与使用方法，以及 OpenGL 渲染优化策略。

## 渲染管线概览

### 渲染架构设计

框架采用"命令收集 + 批量渲染"的架构：

1. **命令收集阶段**: 各 UI 组件将绘制需求添加到 `Render::FrameData`
2. **资源准备阶段**: `IconCache` 确保所需纹理已加载到 GPU
3. **批量渲染阶段**: `Renderer` 执行所有绘制命令，最小化状态切换

```
UI 组件 → RenderData → IconCache → Renderer → GPU
    ↓          ↓         ↓          ↓        ↓
 生成命令   收集命令   准备纹理   批量绘制   显示
```

## RenderData - 渲染命令系统

### 渲染命令类型

`RenderData` 定义了多种渲染命令类型：

```cpp
namespace Render {
    // 基础几何命令
    struct RectCommand {
        QRectF bounds;
        QColor color;
        float cornerRadius = 0.0f;
    };
    
    // 文本渲染命令
    struct TextCommand {
        QString text;
        QPointF position;
        QFont font;
        QColor color;
        QString cacheKey;  // 文本纹理缓存键
    };
    
    // 图标渲染命令
    struct IconCommand {
        QString iconPath;
        QRectF bounds;
        QColor tintColor;
        QString cacheKey;  // 图标纹理缓存键
    };
    
    // 边框命令
    struct BorderCommand {
        QRectF bounds;
        QColor color;
        float width;
        float cornerRadius = 0.0f;
    };
}
```

### FrameData 收集器

`FrameData` 作为单帧渲染命令的收集器：

```cpp
class FrameData {
private:
    std::vector<Render::RectCommand> m_rects;
    std::vector<Render::TextCommand> m_texts;
    std::vector<Render::IconCommand> m_icons;
    std::vector<Render::BorderCommand> m_borders;
    
public:
    // 添加渲染命令
    void addRect(const QRectF& bounds, const QColor& color, float radius = 0.0f);
    void addText(const QString& text, const QPointF& pos, const QFont& font, const QColor& color);
    void addIcon(const QString& path, const QRectF& bounds, const QColor& tint = QColor());
    void addBorder(const QRectF& bounds, const QColor& color, float width, float radius = 0.0f);
    
    // 批量访问（供 Renderer 使用）
    const std::vector<RectCommand>& rects() const { return m_rects; }
    const std::vector<TextCommand>& texts() const { return m_texts; }
    const std::vector<IconCommand>& icons() const { return m_icons; }
    const std::vector<BorderCommand>& borders() const { return m_borders; }
    
    // 帧结束清理
    void clear();
};
```

### 组件渲染集成

UI 组件通过 `append()` 方法添加渲染命令：

```cpp
class UiButton : public IUiComponent {
public:
    void append(Render::FrameData& frameData) const override {
        // 按钮背景
        frameData.addRect(m_bounds, m_backgroundColor, m_cornerRadius);
        
        // 按钮边框
        if (m_borderWidth > 0) {
            frameData.addBorder(m_bounds, m_borderColor, m_borderWidth, m_cornerRadius);
        }
        
        // 按钮文本
        auto textKey = RenderUtils::makeTextCacheKey(m_text, m_font, m_textColor);
        frameData.addText(m_text, m_textPosition, m_font, m_textColor);
        
        // 按钮图标（如果有）
        if (!m_iconPath.isEmpty()) {
            auto iconKey = RenderUtils::makeIconCacheKey(m_iconPath, m_iconSize, m_iconTint);
            frameData.addIcon(m_iconPath, m_iconBounds, m_iconTint);
        }
    }
};
```

## IconCache - 纹理缓存系统

### 缓存管理策略

`IconCache` 负责管理所有图标和文本的 GPU 纹理：

```cpp
class IconCache {
private:
    std::unordered_map<QString, QOpenGLTexture*> m_iconTextures;
    std::unordered_map<QString, QOpenGLTexture*> m_textTextures;
    QOpenGLFunctions* m_gl;
    float m_devicePixelRatio;
    
public:
    // 图标纹理获取（自动加载）
    QOpenGLTexture* getIconTexture(const QString& path, const QSize& size, const QColor& tint);
    
    // 文本纹理获取（自动渲染）
    QOpenGLTexture* getTextTexture(const QString& text, const QFont& font, const QColor& color);
    
    // 资源管理
    void setDevicePixelRatio(float dpr);  // DPR 变化时更新
    void onThemeChanged(bool isDark);     // 主题变化时清理缓存
    void cleanup();                       // 清理所有纹理
};
```

### 缓存键生成

`RenderUtils` 提供统一的缓存键生成功能：

```cpp
namespace RenderUtils {
    // 图标缓存键：路径 + 尺寸 + 色调 + DPR + 主题
    QString makeIconCacheKey(const QString& path, const QSize& size, 
                            const QColor& tint, float dpr, bool isDark) {
        return QString("%1_%2x%3_%4_%5_%6")
               .arg(path)
               .arg(size.width()).arg(size.height())
               .arg(tint.name())
               .arg(QString::number(dpr, 'f', 1))
               .arg(isDark ? "dark" : "light");
    }
    
    // 文本缓存键：文本 + 字体 + 颜色 + DPR
    QString makeTextCacheKey(const QString& text, const QFont& font, 
                            const QColor& color, float dpr) {
        return QString("%1_%2_%3pt_%4_%5")
               .arg(text)
               .arg(font.family())
               .arg(font.pointSize())
               .arg(color.name())
               .arg(QString::number(dpr, 'f', 1));
    }
}
```

### DPR 与主题适配

```cpp
void IconCache::updateResourceContext(float dpr, bool isDark) {
    if (m_devicePixelRatio != dpr || m_isDarkTheme != isDark) {
        // DPR 或主题变化时清理缓存，强制重新生成
        cleanup();
        m_devicePixelRatio = dpr;
        m_isDarkTheme = isDark;
    }
}

QOpenGLTexture* IconCache::getIconTexture(const QString& path, const QSize& size, const QColor& tint) {
    auto key = RenderUtils::makeIconCacheKey(path, size, tint, m_devicePixelRatio, m_isDarkTheme);
    
    auto it = m_iconTextures.find(key);
    if (it != m_iconTextures.end()) {
        return it->second;  // 返回缓存的纹理
    }
    
    // 加载并缓存新纹理
    auto texture = loadIconTexture(path, size * m_devicePixelRatio, tint);
    m_iconTextures[key] = texture;
    return texture;
}
```

## Renderer - OpenGL 渲染器

### 渲染状态管理

`Renderer` 负责 OpenGL 状态管理与绘制优化：

```cpp
class Renderer {
private:
    QOpenGLShaderProgram* m_rectShader;
    QOpenGLShaderProgram* m_textShader;
    QOpenGLShaderProgram* m_iconShader;
    QOpenGLBuffer m_vertexBuffer;
    QOpenGLVertexArrayObject m_vao;
    
public:
    void render(const Render::FrameData& frameData, const QSize& viewport);
    
private:
    void renderRects(const std::vector<Render::RectCommand>& rects);
    void renderTexts(const std::vector<Render::TextCommand>& texts);
    void renderIcons(const std::vector<Render::IconCommand>& icons);
    void renderBorders(const std::vector<Render::BorderCommand>& borders);
};
```

### 批量渲染优化

```cpp
void Renderer::renderRects(const std::vector<Render::RectCommand>& rects) {
    if (rects.empty()) return;
    
    // 设置着色器程序
    m_rectShader->bind();
    m_vao.bind();
    
    // 批量上传顶点数据
    std::vector<RectVertex> vertices;
    vertices.reserve(rects.size() * 6);  // 每个矩形 6 个顶点
    
    for (const auto& rect : rects) {
        addRectVertices(vertices, rect);
    }
    
    m_vertexBuffer.write(0, vertices.data(), vertices.size() * sizeof(RectVertex));
    
    // 一次绘制调用渲染所有矩形
    glDrawArrays(GL_TRIANGLES, 0, vertices.size());
    
    m_vao.release();
    m_rectShader->release();
}
```

### 坐标系转换

框架统一使用逻辑坐标系，Renderer 负责转换：

```cpp
void Renderer::updateProjectionMatrix(const QSize& viewport, float dpr) {
    // 逻辑坐标 → 设备坐标 → NDC
    QMatrix4x4 projection;
    
    // Qt 坐标系（左上角原点）→ OpenGL 坐标系（左下角原点）
    projection.ortho(0, viewport.width(),      // left, right
                    viewport.height(), 0,      // bottom, top (翻转 Y 轴)
                    -1, 1);                   // near, far
    
    // 应用 DPR 缩放
    projection.scale(dpr, dpr, 1.0f);
    
    // 更新所有着色器的投影矩阵
    m_rectShader->setUniformValue("u_projection", projection);
    m_textShader->setUniformValue("u_projection", projection);
    m_iconShader->setUniformValue("u_projection", projection);
}
```

## 性能优化策略

### 渲染批次合并

```cpp
// 相同纹理的图标可以合并为单次绘制调用
void Renderer::renderIconsBatched(const std::vector<Render::IconCommand>& icons) {
    // 按纹理分组
    std::unordered_map<QOpenGLTexture*, std::vector<const IconCommand*>> batches;
    
    for (const auto& icon : icons) {
        auto texture = m_iconCache->getIconTexture(icon.iconPath, icon.bounds.size().toSize(), icon.tintColor);
        batches[texture].push_back(&icon);
    }
    
    // 逐纹理批量渲染
    for (const auto& [texture, commands] : batches) {
        texture->bind();
        renderIconBatch(commands);
        texture->release();
    }
}
```

### 脏区域更新

```cpp
class OptimizedRenderer {
private:
    QRect m_lastViewport;
    std::vector<QRect> m_dirtyRegions;
    
public:
    void markDirty(const QRect& region) {
        m_dirtyRegions.push_back(region);
    }
    
    void render(const Render::FrameData& frameData) {
        if (m_dirtyRegions.empty()) {
            return;  // 无需重绘
        }
        
        // 仅渲染脏区域内的内容
        for (const auto& region : m_dirtyRegions) {
            setScissorRect(region);
            renderRegion(frameData, region);
        }
        
        m_dirtyRegions.clear();
    }
};
```

### 纹理压缩与格式优化

```cpp
QOpenGLTexture* IconCache::loadIconTexture(const QString& path, const QSize& size, const QColor& tint) {
    // 根据内容选择最佳纹理格式
    QImage image(path);
    
    if (image.hasAlphaChannel()) {
        // 带透明通道：使用 RGBA8
        image = image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        image = image.convertToFormat(QImage::Format_RGBA8888);
    } else {
        // 无透明通道：使用 RGB8 节省内存
        image = image.scaled(size, Qt::IgnoreAspectRatio, Qt::SmoothTransformation);
        image = image.convertToFormat(QImage::Format_RGB888);
    }
    
    // 应用色调调制
    if (tint.isValid()) {
        applyTint(image, tint);
    }
    
    auto texture = new QOpenGLTexture(image);
    texture->setMinificationFilter(QOpenGLTexture::Linear);
    texture->setMagnificationFilter(QOpenGLTexture::Linear);
    
    return texture;
}
```

## 常见问题与解决方案

### 纹理内存管理

```cpp
class IconCache {
private:
    static constexpr size_t MAX_CACHE_SIZE = 64 * 1024 * 1024;  // 64MB 限制
    size_t m_currentCacheSize = 0;
    
    void evictLeastRecentlyUsed() {
        // LRU 缓存淘汰策略
        while (m_currentCacheSize > MAX_CACHE_SIZE && !m_textureUsage.empty()) {
            auto oldest = m_textureUsage.front();
            removeTexture(oldest.first);
            m_textureUsage.pop_front();
        }
    }
};
```

### DPR 变化处理

```cpp
void MainOpenGlWindow::resizeGL(int w, int h) {
    auto newDpr = devicePixelRatio();
    
    if (m_lastDpr != newDpr) {
        // DPR 变化，通知缓存系统清理
        m_iconCache->setDevicePixelRatio(newDpr);
        m_renderer->updateProjectionMatrix(QSize(w, h), newDpr);
        m_lastDpr = newDpr;
    }
    
    // 更新 viewport
    m_renderer->setViewport(QSize(w, h));
}
```

## 相关文档

- [表现层架构概览](../presentation/architecture.md) - UI 组件如何使用渲染系统
- [UI 基础部件与容器](../presentation/ui/components.md) - 组件的渲染实现示例
- [Windows 平台集成](../platform/windows.md) - 平台特定的渲染优化