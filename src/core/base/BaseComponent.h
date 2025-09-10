#pragma once

#include "../interfaces/IComponent.h"
#include <chrono>
#include <mutex>

namespace Fangjia::Core {

/**
 * @brief 基础组件实现
 * 基于PR #21的生命周期管理模式
 * 提供线程安全的状态管理和标准的生命周期实现
 */
class BaseComponent : public IComponent {
public:
    BaseComponent();
    virtual ~BaseComponent();

    // IComponent implementation
    void initialize() override;
    void activate() override;
    void deactivate() override;
    void cleanup() override;

    bool isInitialized() const override;
    bool isActive() const override;

    void applyTheme(bool isDark) override;
    void updateResourceContext() override;
    bool tick() override;

protected:
    // 子类可以重写的生命周期方法
    virtual void onInitialize() {}
    virtual void onActivate() {}
    virtual void onDeactivate() {}
    virtual void onCleanup() {}

    // 子类可以重写的主题和资源方法
    virtual void onThemeChanged(bool isDark) {}
    virtual void onResourceContextUpdated() {}

    // 子类可以重写的动画方法
    virtual bool onTick() { return false; }

    // 状态查询（线程安全）
    bool checkInitialized() const;
    bool checkActive() const;

    // 工具方法
    std::chrono::steady_clock::time_point getCurrentTime() const;
    int64_t getElapsedMs(const std::chrono::steady_clock::time_point& start) const;

private:
    enum class State {
        Created,
        Initialized,
        Active,
        Inactive,
        Cleaned
    };

    mutable std::mutex m_stateMutex;
    State m_state = State::Created;
    bool m_isDarkTheme = false;
    std::chrono::steady_clock::time_point m_creationTime;
};

/**
 * @brief 动画基础类
 * 基于PR #21中滚动条动画的实现模式
 */
class BaseAnimatable : public IAnimatable {
public:
    BaseAnimatable();
    virtual ~BaseAnimatable();

    // IAnimatable implementation
    void startAnimation() override;
    void stopAnimation() override;
    bool isAnimating() const override;

    void setAnimationDuration(int milliseconds) override;
    void setAnimationDelay(int milliseconds) override;
    void setAnimationCompleted(AnimationCallback callback) override;

protected:
    // 子类重写的动画方法
    virtual void onAnimationStart() {}
    virtual void onAnimationUpdate(float progress) {} // progress: 0.0 to 1.0
    virtual void onAnimationComplete() {}

    // 动画参数访问
    int getAnimationDuration() const { return m_duration; }
    int getAnimationDelay() const { return m_delay; }
    float getCurrentProgress() const;

    // 工具方法 (基于PR #21的实现模式)
    float interpolateFloat(float start, float end, float t) const;
    float easeInOut(float t) const; // smooth animation curve

    // 更新动画状态 (需要在tick中调用)
    bool updateAnimation();

private:
    mutable std::mutex m_animMutex;
    bool m_isAnimating = false;
    int m_duration = 300; // 默认300ms (PR #21中的淡出时间)
    int m_delay = 900;    // 默认900ms (PR #21中的空闲延迟)
    
    std::chrono::steady_clock::time_point m_animStartTime;
    AnimationCallback m_completedCallback;
};

} // namespace Fangjia::Core