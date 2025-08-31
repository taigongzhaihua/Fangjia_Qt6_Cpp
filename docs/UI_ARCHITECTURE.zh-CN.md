# UI 架构详解（中文文档）

本文档详细说明 UI 框架的核心架构，包括 IUiComponent 生命周期、UiRoot 职责、主题传播机制和事件处理流程。

## IUiComponent 生命周期

所有 UI 组件都实现 `IUiComponent` 接口，其生命周期方法按以下严格顺序调用：

### 生命周期顺序

1. **`updateLayout(const QSize& winSize)`**
   - 基于窗口逻辑尺寸计算布局
   - 确定组件的位置和大小
   - 布局计算不依赖纹理或资源上下文

2. **`updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float dpr)`**
   - 更新纹理缓存和 OpenGL 上下文
   - 处理设备像素比（DPR）相关的资源
   - 确保图标和纹理在正确的上下文中准备就绪

3. **`append(Render::FrameData& fd) const`**
   - 生成绘制命令并添加到帧数据
   - 此时布局和资源都已准备就绪
   - 只读操作，不修改组件状态

4. **`tick()`**
   - 推进动画状态
   - 返回是否仍需要重绘（动画是否活跃）
   - 可能修改组件的动画相关状态

### 事件转发

除生命周期方法外，IUiComponent 还处理交互事件：

- **鼠标事件**: `onMousePress`, `onMouseMove`, `onMouseRelease`
- **滚轮事件**: `onWheel`
- **键盘事件**: 通过 `IKeyInput` 接口（如果实现）
- **主题事件**: `onThemeChanged(bool isDark)`（默认转发至 `applyTheme`）

## UiRoot 职责

`UiRoot`（位于 `presentation/ui/containers/UiRoot.h`）作为顶级容器，承担以下关键职责：

### 统一事件分发

```cpp
// UiRoot 统一处理和分发所有事件
class UiRoot : public IUiComponent, public IFocusContainer {
public:
    // 事件分发到子组件
    bool onMousePress(const QPointF& localPos) override;
    bool onMouseMove(const QPointF& localPos) override;
    bool onMouseRelease(const QPointF& localPos) override;
    bool onWheel(const QPointF& localPos, const QPointF& delta) override;
};
```

### 指针捕获管理

- 跟踪哪个组件当前捕获了鼠标
- 确保拖拽操作的连续性
- 管理 hover 状态和鼠标离开事件

### 焦点管理

作为 `IFocusContainer`，UiRoot 管理焦点树：

- **IFocusable**: 可接收焦点的组件（如输入框、按钮）
- **IFocusContainer**: 可包含焦点子项的容器
- 焦点导航（Tab 键切换）
- 焦点获得/丢失事件

### 主题传播

```cpp
// UiRoot::propagateThemeChange 递归下发主题
void UiRoot::propagateThemeChange(bool isDark) {
    // 先更新自身主题
    onThemeChanged(isDark);
    
    // 递归传播到所有子组件
    for (auto& child : m_children) {
        if (child) {
            child->onThemeChanged(isDark);
        }
    }
}
```

### 统一生命周期驱动

UiRoot 负责驱动所有顶级组件的完整生命周期：

```cpp
void UiRoot::updateFrame(const QSize& winSize, IconCache& cache, 
                        QOpenGLFunctions* gl, float dpr) {
    // 1. 布局阶段
    updateLayout(winSize);
    
    // 2. 资源上下文阶段
    updateResourceContext(cache, gl, dpr);
    
    // 3. 渲染命令生成在外部调用 append()
    // 4. 动画推进在外部调用 tick()
}
```

## 主题变化顺序与避免闪烁

为避免主题切换时的视觉闪烁，系统采用了精心设计的更新顺序。

### RebuildHost 的设计

声明式重建容器 `UI::RebuildHost`（位于 `presentation/ui/declarative/RebuildHost.h`）在 `requestRebuild()` 中实现了"先应用主题，再更新资源上下文"的关键设计：

```cpp
void RebuildHost::requestRebuild() {
    if (!m_builder) return;
    m_child = m_builder();
    
    if (m_child) {
        // 1. 首先设置视口（布局计算可能需要）
        if (m_hasViewport) {
            if (auto* c = dynamic_cast<IUiContent*>(m_child.get())) {
                c->setViewportRect(m_viewport);
            }
        }
        
        // 2. 在更新资源上下文之前应用主题，确保调色板和图标选择使用正确的主题状态
        if (m_hasTheme) {
            m_child->onThemeChanged(m_isDark);
        }
        
        // 3. 更新资源上下文（现在组件已有正确的主题状态）
        if (m_hasCtx) {
            m_child->updateResourceContext(*m_cache, m_gl, m_dpr);
        }
        
        // 4. 最后更新布局（通常不依赖资源上下文）
        if (m_hasWinSize) {
            m_child->updateLayout(m_winSize);
        }
    }
}
```

### 避免闪烁的关键

这个顺序设计的重要性：

1. **主题优先**: 在生成资源缓存键之前确保主题状态正确
2. **缓存一致性**: 避免在错误主题下生成图标缓存键
3. **视觉连续性**: 防止在重建期间出现临时的错误调色或图标

### 缓存键的主题依赖

文本和图标缓存键都包含主题信息：

```cpp
// RenderUtils::makeIconCacheKey
QString iconCacheKey = RenderUtils::makeIconCacheKey(
    baseKey, 
    pixelSize, 
    isDark ? "dark" : "light"
);

// RenderUtils::makeTextCacheKey  
QString textCacheKey = RenderUtils::makeTextCacheKey(
    text, 
    pixelSize, 
    color  // 颜色本身可能是主题相关的
);
```

如果主题状态在资源上下文更新时不正确，会导致：
- 图标使用错误的明/暗变体
- 文本使用错误的主题颜色
- 缓存键不匹配导致重复加载

## MainOpenGlWindow 中的主题驱动

主窗口类（`apps/fangjia/MainOpenGlWindow.cpp`）统一驱动主题与资源上下文更新：

### initializeGL 中的初始化

```cpp
void MainOpenGlWindow::initializeGL() {
    // ... OpenGL 初始化 ...
    
    // 应用初始主题
    applyTheme(m_isDarkTheme);
    
    // 初始化资源上下文
    updateResourceContexts();
}
```

### applyTheme 中的统一更新

```cpp
void MainOpenGlWindow::applyTheme(bool isDark) {
    m_isDarkTheme = isDark;
    
    // 1. 更新主题管理器状态（如果需要）
    if (m_themeMgr) {
        m_themeMgr->setDarkTheme(isDark);
    }
    
    // 2. 通过 UiRoot 传播主题到所有组件
    if (m_uiRoot) {
        m_uiRoot->propagateThemeChange(isDark);
    }
    
    // 3. 更新资源上下文（在主题状态正确后）
    updateResourceContexts();
    
    // 4. 请求重绘
    update();
}
```

### 声明式子树的主题同步

对于使用 `BindingHost` 的声明式子树：

```cpp
// 主题变化触发重建
m_shellHost->observe(m_themeMgr, &ThemeManager::themeChanged, [this]() {
    // 重建时 RebuildHost 会按正确顺序同步主题和资源
    m_shellHost->requestRebuild();
});
```

## 组件间协作模式

### 容器与子组件

```cpp
// 容器负责传递生命周期事件到子组件
class UiPanel : public IUiComponent, public ILayoutable {
public:
    void updateLayout(const QSize& winSize) override {
        // 1. 自身布局计算
        calculateLayout(winSize);
        
        // 2. 传递到子组件
        for (auto& child : m_children) {
            if (child) {
                child->updateLayout(winSize);
            }
        }
    }
    
    void onThemeChanged(bool isDark) override {
        // 传播主题到子组件
        for (auto& child : m_children) {
            if (child) {
                child->onThemeChanged(isDark);
            }
        }
    }
};
```

### 声明式与运行时组件桥接

```cpp
// ComponentWrapper 将运行时组件包装为声明式组件
class ComponentWrapper : public Widget {
    void onThemeChanged(bool isDark) override {
        // 转发主题事件到包装的运行时组件
        if (m_wrappedComponent) {
            m_wrappedComponent->onThemeChanged(isDark);
        }
        
        // 调用基类处理装饰器主题
        Widget::onThemeChanged(isDark);
    }
};
```

## 性能考虑

### 渐进式更新

- **布局失效**: 只有影响布局的变化才触发 `updateLayout`
- **资源失效**: 只有 DPR 变化或缓存失效才重新加载资源  
- **重绘最小化**: `tick()` 返回 false 时跳过不必要的重绘

### 缓存策略

- **图标缓存**: 基于路径、尺寸、主题的键值缓存
- **文本缓存**: 基于内容、字体、颜色的键值缓存
- **布局缓存**: 容器可缓存子项的测量结果

### 事件过滤

- **边界检查**: 早期剔除不在组件范围内的事件
- **捕获优先**: 被捕获的组件优先处理后续事件
- **焦点管理**: 只有焦点组件接收键盘事件

## 相关文档

- [声明式 TopBar](./DECLARATIVE_NAV_TOPBAR.zh-CN.md) - TopBar 动画和主题集成
- [布局系统](./LAYOUTS.zh-CN.md) - ILayoutable 接口和容器布局  
- [声明式概览](./DECLARATIVE_OVERVIEW.zh-CN.md) - Widget 装饰器和 BindingHost
- [滚动容器](./SCROLL_VIEW.zh-CN.md) - UiScrollView 的视口管理