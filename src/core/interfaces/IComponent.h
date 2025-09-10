#pragma once

#include <memory>
#include <functional>

namespace Fangjia::Core {

/**
 * @brief 基础组件接口
 * 基于PR #21的设计原则：清晰的生命周期管理和事件处理
 * 
 * 设计原则：
 * - 明确的生命周期（初始化、激活、停用、清理）
 * - 清晰的边界检查和状态管理
 * - 支持主题切换和资源上下文更新
 */
class IComponent {
public:
    virtual ~IComponent() = default;

    // 生命周期管理
    virtual void initialize() = 0;
    virtual void activate() = 0;
    virtual void deactivate() = 0;
    virtual void cleanup() = 0;

    // 状态查询
    virtual bool isInitialized() const = 0;
    virtual bool isActive() const = 0;

    // 主题支持 (PR #21关键特性)
    virtual void applyTheme(bool isDark) = 0;
    
    // 资源上下文更新 (PR #21关键特性)
    virtual void updateResourceContext() = 0;
    
    // 动画支持 (PR #21关键特性)
    virtual bool tick() = 0; // 返回true表示需要继续动画
};

/**
 * @brief 事件处理接口
 * 基于PR #21中UiPage和UiScrollView的事件处理模式
 */
template<typename Point, typename MouseButton, typename WheelDelta>
class IEventHandler {
public:
    virtual ~IEventHandler() = default;
    
    // 鼠标事件处理 (PR #21核心功能)
    virtual bool onMousePress(const Point& pos, MouseButton button) = 0;
    virtual bool onMouseMove(const Point& pos) = 0;
    virtual bool onMouseRelease(const Point& pos, MouseButton button) = 0;
    
    // 滚轮事件处理 (PR #21核心改进)
    virtual bool onWheel(const Point& pos, const WheelDelta& delta) = 0;
    
    // 键盘事件处理
    virtual bool onKeyPress(int key, int modifiers) = 0;
    virtual bool onKeyRelease(int key, int modifiers) = 0;
};

/**
 * @brief 布局接口
 * 基于PR #21中的测量和排列模式
 */
template<typename Size, typename Rect>
class ILayoutable {
public:
    virtual ~ILayoutable() = default;
    
    // 测量和排列 (PR #21关键模式)
    virtual Size measure(const Size& availableSize) = 0;
    virtual void arrange(const Rect& finalRect) = 0;
    
    // 边界查询 (PR #21边界检查模式)
    virtual Rect bounds() const = 0;
    virtual bool hitTest(const typename Rect::PointType& point) const = 0;
};

/**
 * @brief 渲染接口
 * 基于PR #21的渲染和缓存模式
 */
template<typename RenderContext>
class IRenderable {
public:
    virtual ~IRenderable() = default;
    
    // 渲染 (PR #21核心方法)
    virtual void render(RenderContext& context) = 0;
    
    // 可见性控制
    virtual bool isVisible() const = 0;
    virtual void setVisible(bool visible) = 0;
    
    // 透明度支持 (PR #21动画特性)
    virtual float opacity() const = 0;
    virtual void setOpacity(float alpha) = 0;
};

/**
 * @brief 动画接口
 * 基于PR #21中滚动条动画的设计模式
 */
class IAnimatable {
public:
    virtual ~IAnimatable() = default;
    
    // 动画控制
    virtual void startAnimation() = 0;
    virtual void stopAnimation() = 0;
    virtual bool isAnimating() const = 0;
    
    // 动画参数
    virtual void setAnimationDuration(int milliseconds) = 0;
    virtual void setAnimationDelay(int milliseconds) = 0;
    
    // 动画回调
    using AnimationCallback = std::function<void()>;
    virtual void setAnimationCompleted(AnimationCallback callback) = 0;
};

} // namespace Fangjia::Core