#include "BaseComponent.h"
#include <stdexcept>
#include <algorithm>

namespace Fangjia::Core {

// ==================== BaseComponent ====================

BaseComponent::BaseComponent() 
    : m_creationTime(std::chrono::steady_clock::now()) {
}

BaseComponent::~BaseComponent() {
    if (m_state != State::Cleaned) {
        cleanup();
    }
}

void BaseComponent::initialize() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (m_state != State::Created) {
        throw std::logic_error("Component can only be initialized once");
    }
    
    try {
        onInitialize();
        m_state = State::Initialized;
    } catch (...) {
        m_state = State::Created; // rollback on failure
        throw;
    }
}

void BaseComponent::activate() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (m_state != State::Initialized && m_state != State::Inactive) {
        throw std::logic_error("Component must be initialized before activation");
    }
    
    try {
        onActivate();
        m_state = State::Active;
    } catch (...) {
        // Don't rollback state on activation failure
        throw;
    }
}

void BaseComponent::deactivate() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (m_state != State::Active) {
        return; // already inactive
    }
    
    onDeactivate();
    m_state = State::Inactive;
}

void BaseComponent::cleanup() {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    if (m_state == State::Cleaned) {
        return; // already cleaned
    }
    
    if (m_state == State::Active) {
        onDeactivate();
    }
    
    onCleanup();
    m_state = State::Cleaned;
}

bool BaseComponent::isInitialized() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_state != State::Created;
}

bool BaseComponent::isActive() const {
    std::lock_guard<std::mutex> lock(m_stateMutex);
    return m_state == State::Active;
}

void BaseComponent::applyTheme(bool isDark) {
    {
        std::lock_guard<std::mutex> lock(m_stateMutex);
        m_isDarkTheme = isDark;
    }
    onThemeChanged(isDark);
}

void BaseComponent::updateResourceContext() {
    onResourceContextUpdated();
}

bool BaseComponent::tick() {
    return onTick();
}

bool BaseComponent::checkInitialized() const {
    return isInitialized();
}

bool BaseComponent::checkActive() const {
    return isActive();
}

std::chrono::steady_clock::time_point BaseComponent::getCurrentTime() const {
    return std::chrono::steady_clock::now();
}

int64_t BaseComponent::getElapsedMs(const std::chrono::steady_clock::time_point& start) const {
    return std::chrono::duration_cast<std::chrono::milliseconds>(
        getCurrentTime() - start
    ).count();
}

// ==================== BaseAnimatable ====================

BaseAnimatable::BaseAnimatable() = default;

BaseAnimatable::~BaseAnimatable() {
    stopAnimation();
}

void BaseAnimatable::startAnimation() {
    std::lock_guard<std::mutex> lock(m_animMutex);
    if (m_isAnimating) {
        return; // already animating
    }
    
    m_animStartTime = std::chrono::steady_clock::now();
    m_isAnimating = true;
    
    onAnimationStart();
}

void BaseAnimatable::stopAnimation() {
    std::lock_guard<std::mutex> lock(m_animMutex);
    if (!m_isAnimating) {
        return; // not animating
    }
    
    m_isAnimating = false;
    onAnimationComplete();
    
    if (m_completedCallback) {
        m_completedCallback();
    }
}

bool BaseAnimatable::isAnimating() const {
    std::lock_guard<std::mutex> lock(m_animMutex);
    return m_isAnimating;
}

void BaseAnimatable::setAnimationDuration(int milliseconds) {
    std::lock_guard<std::mutex> lock(m_animMutex);
    m_duration = std::max(1, milliseconds); // minimum 1ms
}

void BaseAnimatable::setAnimationDelay(int milliseconds) {
    std::lock_guard<std::mutex> lock(m_animMutex);
    m_delay = std::max(0, milliseconds);
}

void BaseAnimatable::setAnimationCompleted(AnimationCallback callback) {
    std::lock_guard<std::mutex> lock(m_animMutex);
    m_completedCallback = callback;
}

float BaseAnimatable::getCurrentProgress() const {
    std::lock_guard<std::mutex> lock(m_animMutex);
    if (!m_isAnimating) {
        return 0.0f;
    }
    
    const auto now = std::chrono::steady_clock::now();
    const auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - m_animStartTime
    ).count();
    
    if (elapsed < m_delay) {
        return 0.0f; // still in delay phase
    }
    
    const auto animElapsed = elapsed - m_delay;
    if (animElapsed >= m_duration) {
        return 1.0f; // animation complete
    }
    
    return static_cast<float>(animElapsed) / static_cast<float>(m_duration);
}

float BaseAnimatable::interpolateFloat(float start, float end, float t) const {
    return start + (end - start) * std::clamp(t, 0.0f, 1.0f);
}

float BaseAnimatable::easeInOut(float t) const {
    // Cubic ease-in-out curve (smooth animation)
    if (t < 0.5f) {
        return 4.0f * t * t * t;
    } else {
        const float f = 2.0f * t - 2.0f;
        return 1.0f + f * f * f / 2.0f;
    }
}

bool BaseAnimatable::updateAnimation() {
    std::lock_guard<std::mutex> lock(m_animMutex);
    if (!m_isAnimating) {
        return false;
    }
    
    const float progress = getCurrentProgress();
    const float easedProgress = easeInOut(progress);
    
    onAnimationUpdate(easedProgress);
    
    if (progress >= 1.0f) {
        m_isAnimating = false;
        onAnimationComplete();
        
        if (m_completedCallback) {
            m_completedCallback();
        }
        return false; // animation finished
    }
    
    return true; // continue animation
}

} // namespace Fangjia::Core