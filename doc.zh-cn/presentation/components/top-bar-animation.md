[English](../../../doc/presentation/components/top-bar-animation.md) | **简体中文**

# TopBar 跟随系统动画

## 概述

TopBar 组件在切换"跟随系统主题"设置时实现了复杂的两阶段动画系统。这创建了平滑的视觉过渡，清楚地传达主题切换和跟随系统按钮之间的关系，同时保持直观的交互。

## 动画行为

### 启用跟随系统 (`followSystem(true, true)`)

当启用"跟随系统"并带动画时：

1. **阶段 1 - 主题按钮淡出** (160ms, easeInOut)
   - 主题按钮逐渐淡出至透明
   - 淡出期间按钮变为非交互状态
   - 跟随按钮保持在原始位置

2. **阶段 2 - 跟随按钮右滑** (200ms, easeInOut)  
   - 跟随按钮平滑滑动至主题按钮位置
   - 滑动期间主题按钮保持隐藏状态
   - 动画完成时跟随按钮位于主题位置

### 禁用跟随系统 (`followSystem(false, true)`)

当禁用"跟随系统"并带动画时：

1. **阶段 1 - 跟随按钮左滑** (180ms, easeInOut)
   - 跟随按钮滑回原始位置
   - 滑动期间主题按钮保持隐藏状态
   - 过渡为主题按钮返回做准备

2. **阶段 2 - 主题按钮淡入** (160ms, easeInOut)
   - 主题按钮在原始位置逐渐淡入
   - 跟随按钮稳定在原始位置
   - 动画完成时两个按钮都可见且可交互

## 实现细节

### 动画状态机

动画系统使用在 `presentation/ui/widgets/UiTopBar.cpp` 中定义的状态机：

```cpp
enum class AnimPhase {
    Idle,                    // 无动画活动
    HideTheme_FadeOut,      // 阶段 1：主题按钮淡出
    MoveFollow_Right,       // 阶段 2：跟随按钮向右移动  
    MoveFollow_Left,        // 阶段 1：跟随按钮向左移动
    ShowTheme_FadeIn        // 阶段 2：主题按钮淡入
};
```

### 持续时间缩放

所有动画持续时间使用 `scaleDuration()` 以确保在不同硬件上的一致时序：

```cpp
int UiTopBar::scaleDuration(int durationMs) {
    // 应用 2/3 缩放以获得响应感
    return static_cast<int>(durationMs * 0.67f);
}
```

标准持续时间：
- 淡出：`scaleDuration(160)` ≈ 107ms
- 右滑：`scaleDuration(200)` ≈ 133ms  
- 左滑：`scaleDuration(180)` ≈ 120ms
- 淡入：`scaleDuration(160)` ≈ 107ms

### 核心动画方法

```cpp
void UiTopBar::startAnimSequence(const bool followOn) {
    if (!m_animClock.isValid()) m_animClock.start();
    m_phaseStartAlpha = m_themeAlpha;
    m_phaseStartSlide = m_followSlide;

    if (followOn) 
        beginPhase(AnimPhase::HideTheme_FadeOut, scaleDuration(160));
    else          
        beginPhase(AnimPhase::MoveFollow_Left, scaleDuration(180));
}

void UiTopBar::beginPhase(const AnimPhase ph, const int durationMs) {
    m_animPhase = ph;
    m_animDurationMs = durationMs;
    m_phaseStartMs = m_animClock.elapsed();
}
```

## 交互与点击测试

### 主题按钮交互性

主题按钮的交互性在动画期间被仔细管理：

```cpp
bool UiTopBar::themeInteractive() const {
    if (m_followSystem && m_animPhase != AnimPhase::ShowTheme_FadeIn) {
        return m_themeAlpha > 0.6f;  // 跟随系统时的更高阈值
    }
    return m_themeAlpha > 0.4f;      // 标准阈值
}
```

**交互状态：**
- **普通模式**: 主题按钮在 alpha > 0.4 时可交互
- **跟随系统模式**: 主题按钮在 alpha > 0.6 时可交互（淡入期间除外）
- **动画期间**: 按钮响应性随可见性缩放

### 鼠标事件处理

按钮点击测试考虑动画状态：

```cpp
bool UiTopBar::onMousePress(const QPoint& pos) {
    // 检查主题按钮（如果可交互）
    if (m_btnTheme.contains(pos) && themeInteractive()) {
        // 处理主题切换
        return true;
    }
    
    // 检查跟随按钮
    if (m_btnFollow.contains(pos)) {
        // 处理跟随系统切换
        return true;
    }
    
    // 检查系统按钮（始终可交互）
    if (m_btnClose.contains(pos) || m_btnMax.contains(pos) || m_btnMin.contains(pos)) {
        // 处理系统按钮点击
        return true;
    }
    
    return false;
}
```

## 窗口集成模式

### 从主窗口触发动画

从 `apps/fangjia/MainOpenGlWindow.cpp`：

```cpp
void MainOpenGlWindow::onFollowSystemToggle() const {
    // 为下次重建设置动画标志
    const_cast<MainOpenGlWindow*>(this)->m_animateFollowChange = true;
    
    // 切换 ThemeManager 状态
    setFollowSystem(!followSystem());
    
    // 下个事件循环将触发 RebuildHost 重建和动画
}
```

### Shell 配置

在声明式 shell 构建器中：

```cpp
auto bar = UI::topBar()->followSystem(followSystem, m_animateFollowChange);
```

**流程：**
1. 用户点击"跟随系统"按钮
2. `onFollowSystemToggle()` 设置动画标志并切换 ThemeManager
3. 下个事件循环触发 UI 重建
4. TopBar 接收带动画标志的新 `followSystem` 状态
5. 动画序列自动开始

## 环境同步

### RebuildHost 顺序

`UI::RebuildHost::requestRebuild()` 以正确顺序同步环境以防止视觉错误：

1. **主题应用**: 首先调用 `onThemeChanged(isDark)`
2. **资源上下文**: 使用正确主题更新 `updateResourceContext(...)`
3. **布局更新**: 最后调用 `updateLayout(...)`

这防止 TopBar 在重建期间临时使用错误的颜色或缓存的图标键。

### 主题与资源上下文协调

```cpp
void RebuildHost::performRebuild() {
    if (!m_component || !m_needsRebuild) return;
    
    // 首先应用主题（影响资源缓存键）
    if (m_themeValid) {
        m_component->onThemeChanged(m_isDark);
    }
    
    // 使用正确的主题上下文更新资源
    if (m_resourceContextValid) {
        m_component->updateResourceContext(*m_iconCache, m_gl, m_devicePixelRatio);
    }
    
    // 布局最后（可能依赖主题化资源）
    if (m_viewportValid) {
        m_component->updateLayout(m_viewport);
    }
    
    m_needsRebuild = false;
}
```

## 动画性能

### 帧率独立性

动画使用 `QElapsedTimer` 实现一致的时序：

```cpp
bool UiTopBar::tick() {
    if (m_animPhase == AnimPhase::Idle) return false;
    
    // 计算自阶段开始的经过时间
    const int elapsed = m_animClock.elapsed() - m_phaseStartMs;
    const float t = std::clamp(static_cast<float>(elapsed) / m_animDurationMs, 0.0f, 1.0f);
    
    // 应用缓动曲线
    const float e = easeInOut(t);
    
    // 基于当前阶段更新动画值
    // ... 特定阶段的动画逻辑
    
    return m_animPhase != AnimPhase::Idle;
}
```

### 缓动函数

使用三次缓动的平滑动画曲线：

```cpp
float UiTopBar::easeInOut(float t) {
    // 三次缓入缓出曲线
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    } else {
        const float f = 2.0f * t - 2.0f;
        return 1.0f + f * f * f / 2.0f;
    }
}
```

### 内存效率

- 动画状态使用最少内存（几个浮点值）
- 动画期间无动态分配
- 重用现有按钮结构
- 高效的插值计算

## 相关文档

- [TopBar 组件概览](top-bar.md)
- [UI 框架概览](../ui-framework/overview.md)
- [主题系统](../ui-framework/theme-and-rendering.md)
- [图形与渲染](../../infrastructure/gfx.md)