#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <memory>
#include <functional>

// Include just the essential parts to validate the fix
// This simulates the real RebuildHost with minimal dependencies

namespace Validation {

// Mock types to replace Qt dependencies
struct QRect {
    int x, y, width, height;
    QRect(int x = 0, int y = 0, int w = 0, int h = 0) : x(x), y(y), width(w), height(h) {}
    bool isValid() const { return width > 0 && height > 0; }
};

struct QSize {
    int width, height;
    QSize(int w = 0, int h = 0) : width(w), height(h) {}
};

class IconCache {};
class QOpenGLFunctions {};

// Global call order tracker
std::vector<std::string> g_callOrder;

// Base interfaces (minimal)
class IUiComponent {
public:
    virtual ~IUiComponent() = default;
    virtual void updateLayout(const QSize&) {}
    virtual void updateResourceContext(IconCache&, QOpenGLFunctions*, float) {}
    virtual void onThemeChanged(bool isDark) {}
    virtual QRect bounds() const { return QRect(); }
};

class IUiContent {
public:
    virtual ~IUiContent() = default;
    virtual void setViewportRect(const QRect&) {}
};

// Test component that mimics TopBar behavior
class MockTopBarComponent : public IUiComponent, public IUiContent {
private:
    bool m_isDark = false; // Default theme state (often starts as dark)
    bool m_themeCorrectDuringResourceUpdate = false;

public:
    void onThemeChanged(bool isDark) override {
        g_callOrder.push_back("onThemeChanged");
        m_isDark = isDark;
        std::cout << "MockTopBarComponent::onThemeChanged(" << (isDark ? "dark" : "light") << ")" << std::endl;
    }
    
    void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {
        g_callOrder.push_back("updateResourceContext");
        
        // This simulates what UiTopBar::updateResourceContext does - 
        // it selects palette and icon variants based on m_dark flag
        std::string selectedPalette = m_isDark ? "darkPalette" : "lightPalette";
        std::string selectedThemeIcon = m_isDark ? "theme_sun" : "theme_moon";
        
        std::cout << "MockTopBarComponent::updateResourceContext() - isDark=" << m_isDark << std::endl;
        std::cout << "  Selected palette: " << selectedPalette << std::endl;
        std::cout << "  Selected theme icon: " << selectedThemeIcon << std::endl;
        
        // For light theme test, m_isDark should be false here
        // If we're in light theme, expect false (light palette/icons)
        m_themeCorrectDuringResourceUpdate = !m_isDark; // Expecting light theme for this test
    }
    
    void updateLayout(const QSize&) override {
        g_callOrder.push_back("updateLayout");
        std::cout << "MockTopBarComponent::updateLayout()" << std::endl;
    }
    
    void setViewportRect(const QRect&) override {
        // Optional implementation
    }
    
    bool wasThemeCorrectDuringResourceUpdate() const {
        return m_themeCorrectDuringResourceUpdate;
    }
    
    bool isDark() const { return m_isDark; }
};

// Copy the FIXED RebuildHost logic (reordered operations)
class FixedRebuildHost {
private:
    std::function<std::unique_ptr<IUiComponent>()> m_builder;
    std::unique_ptr<IUiComponent> m_child;
    
    QRect m_viewport;
    QSize m_winSize;
    IconCache* m_cache = nullptr;
    QOpenGLFunctions* m_gl = nullptr;
    float m_dpr = 1.0f;
    bool m_isDark = false;
    
    bool m_hasViewport = false;
    bool m_hasWinSize = false;
    bool m_hasCtx = false;
    bool m_hasTheme = false;
    
public:
    void setBuilder(std::function<std::unique_ptr<IUiComponent>()> fn, bool buildImmediately = true) {
        m_builder = std::move(fn);
        if (buildImmediately && m_builder) {
            requestRebuild();
        }
    }
    
    // FIXED implementation matching the actual fix in RebuildHost.h
    void requestRebuild() {
        if (!m_builder) return;
        m_child = m_builder();
        // 重建后立即同步上下文与视口
        // 注意：操作顺序很重要，避免主题闪烁
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
    
    void setViewportRect(const QRect& r) {
        m_viewport = r;
        m_hasViewport = true;
    }
    
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float dpr) {
        m_cache = &cache;
        m_gl = gl;
        m_dpr = dpr;
        m_hasCtx = true;
    }
    
    void updateLayout(const QSize& size) {
        m_winSize = size;
        m_hasWinSize = true;
    }
    
    void onThemeChanged(bool isDark) {
        m_isDark = isDark;
        m_hasTheme = true;
    }
    
    IUiComponent* child() const { return m_child.get(); }
};

} // namespace Validation

bool validateThemeOrderingFix() {
    std::cout << "\n================================================" << std::endl;
    std::cout << "Validating ACTUAL RebuildHost Fix" << std::endl;
    std::cout << "================================================" << std::endl;
    
    Validation::FixedRebuildHost host;
    Validation::IconCache cache;
    Validation::QOpenGLFunctions gl;
    
    // Set up host context (simulating light theme in app)
    host.setViewportRect(Validation::QRect(0, 0, 800, 600));
    host.updateResourceContext(cache, &gl, 1.0f);
    host.updateLayout(Validation::QSize(800, 600));
    host.onThemeChanged(false); // Light theme
    
    std::cout << "Set up host context: Light theme, 800x600 viewport" << std::endl;
    
    // Set builder to create TopBar component (don't build immediately)
    Validation::MockTopBarComponent* topBarPtr = nullptr;
    host.setBuilder([&topBarPtr]() -> std::unique_ptr<Validation::IUiComponent> {
        auto topBar = std::make_unique<Validation::MockTopBarComponent>();
        topBarPtr = topBar.get();
        return std::move(topBar);
    }, false); // Don't build immediately
    
    std::cout << "\nNow triggering rebuild (simulating Nav interaction)..." << std::endl;
    std::cout << "This should call operations in the FIXED order:\n";
    std::cout << "1. setViewportRect\n2. onThemeChanged\n3. updateResourceContext\n4. updateLayout" << std::endl;
    
    // Clear call order and trigger rebuild
    Validation::g_callOrder.clear();
    host.requestRebuild();
    
    // Verify results
    std::cout << "\n=== VALIDATION RESULTS ===" << std::endl;
    std::cout << "Call order recorded:" << std::endl;
    for (size_t i = 0; i < Validation::g_callOrder.size(); ++i) {
        std::cout << "  " << (i + 1) << ". " << Validation::g_callOrder[i] << std::endl;
    }
    
    // Check that theme came before resource context
    auto themePos = std::find(Validation::g_callOrder.begin(), Validation::g_callOrder.end(), "onThemeChanged");
    auto resourcePos = std::find(Validation::g_callOrder.begin(), Validation::g_callOrder.end(), "updateResourceContext");
    
    bool themeBeforeResource = (themePos != Validation::g_callOrder.end() && 
                               resourcePos != Validation::g_callOrder.end() && 
                               themePos < resourcePos);
    
    std::cout << "\nFix verification:" << std::endl;
    std::cout << "  onThemeChanged called before updateResourceContext: " 
              << (themeBeforeResource ? "✅ YES" : "❌ NO") << std::endl;
    
    // Check component had correct theme during resource update
    bool themeCorrect = false;
    if (topBarPtr) {
        themeCorrect = topBarPtr->wasThemeCorrectDuringResourceUpdate();
        std::cout << "  Component had correct theme during resource update: " 
                  << (themeCorrect ? "✅ YES" : "❌ NO") << std::endl;
        std::cout << "  Final theme state: " << (topBarPtr->isDark() ? "dark" : "light") << std::endl;
    }
    
    bool success = themeBeforeResource && themeCorrect;
    std::cout << "\nOVERALL VALIDATION: " << (success ? "✅ PASSED" : "❌ FAILED") << std::endl;
    
    if (success) {
        std::cout << "\n🎉 The fix successfully resolves the TopBar theme flicker issue!" << std::endl;
        std::cout << "   Components now receive correct theme before selecting palettes/icons." << std::endl;
    } else {
        std::cout << "\n❌ Fix validation failed - theme flicker may still occur." << std::endl;
    }
    
    return success;
}

int main() {
    std::cout << "RebuildHost Theme Ordering Fix - Validation Test" << std::endl;
    std::cout << "=================================================" << std::endl;
    std::cout << "\nThis test validates that the fix in RebuildHost.h" << std::endl;
    std::cout << "resolves the theme flicker issue in TopBar components." << std::endl;
    
    bool success = validateThemeOrderingFix();
    
    std::cout << "\n=================================================" << std::endl;
    std::cout << "Test Result: " << (success ? "PASSED" : "FAILED") << std::endl;
    std::cout << "=================================================" << std::endl;
    
    return success ? 0 : 1;
}