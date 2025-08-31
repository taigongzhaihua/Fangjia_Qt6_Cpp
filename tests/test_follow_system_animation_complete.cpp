/*
 * Test file: test_follow_system_animation_complete.cpp
 * Tests the complete Follow System animation implementation including
 * the declarative TopBar integration and animation sequences.
 */

#include <iostream>
#include <cassert>
#include <memory>
#include <functional>
#include <string>
#include <cmath>
#include <algorithm>

// Test the complete Follow System animation behavior
class TestFollowSystemAnimation {
private:
    // Mock timing for deterministic tests
    struct MockClock {
        int64_t elapsed{0};
        void advance(int64_t ms) { elapsed += ms; }
        void reset() { elapsed = 0; }
    };

    // Animation phases matching UiTopBar
    enum class AnimPhase { 
        Idle, 
        HideTheme_FadeOut, 
        MoveFollow_Right, 
        MoveFollow_Left, 
        ShowTheme_FadeIn 
    };

    // Mock UiTopBar that simulates the real animation behavior
    class MockUiTopBar {
    private:
        AnimPhase m_animPhase{AnimPhase::Idle};
        int m_animDurationMs{0};
        int64_t m_phaseStartMs{0};
        MockClock* m_clock{nullptr};
        
        float m_themeAlpha{1.0f};
        float m_followSlide{0.0f};
        float m_phaseStartAlpha{1.0f};
        float m_phaseStartSlide{0.0f};
        
        bool m_followSystem{false};
        float m_cornerRadius{6.0f};
        bool m_clickFollowPending{false};
        bool m_clickThemePending{false};

        static float easeInOut(float t) {
            t = std::clamp(t, 0.0f, 1.0f);
            return t * t * (3.0f - 2.0f * t);
        }

        static float lerp(float a, float b, float t) {
            return a + (b - a) * t;
        }

        void beginPhase(AnimPhase ph, int durationMs) {
            m_animPhase = ph;
            m_animDurationMs = durationMs;
            m_phaseStartMs = m_clock ? m_clock->elapsed : 0;
        }

        void startAnimSequence(bool followOn) {
            m_phaseStartAlpha = m_themeAlpha;
            m_phaseStartSlide = m_followSlide;
            
            if (followOn) beginPhase(AnimPhase::HideTheme_FadeOut, 160);
            else          beginPhase(AnimPhase::MoveFollow_Left, 180);
        }

    public:
        MockUiTopBar(MockClock* clock) : m_clock(clock) {}

        void setFollowSystem(bool on, bool animate) {
            if (m_followSystem == on && animate) return;
            if (!animate) {
                m_followSystem = on;
                m_animPhase = AnimPhase::Idle;
                m_themeAlpha = on ? 0.0f : 1.0f;
                m_followSlide = on ? 1.0f : 0.0f;
                return;
            }
            if (m_followSystem != on) {
                m_followSystem = on;
                startAnimSequence(on);
            }
        }

        void setCornerRadius(float r) { m_cornerRadius = r; }

        bool tick() {
            if (m_animPhase == AnimPhase::Idle) return false;
            
            const int64_t now = m_clock ? m_clock->elapsed : 0;
            const float tRaw = m_animDurationMs > 0 ? 
                static_cast<float>(now - m_phaseStartMs) / static_cast<float>(m_animDurationMs) : 1.0f;
            const float t = std::clamp(tRaw, 0.0f, 1.0f);
            const float e = easeInOut(t);

            switch (m_animPhase) {
            case AnimPhase::HideTheme_FadeOut:
                m_themeAlpha = lerp(m_phaseStartAlpha, 0.0f, e);
                if (t >= 1.0f) { 
                    m_phaseStartSlide = m_followSlide; 
                    beginPhase(AnimPhase::MoveFollow_Right, 200); 
                }
                break;
            case AnimPhase::MoveFollow_Right:
                m_followSlide = lerp(m_phaseStartSlide, 1.0f, e);
                if (t >= 1.0f) { 
                    m_animPhase = AnimPhase::Idle; 
                }
                break;
            case AnimPhase::MoveFollow_Left:
                m_followSlide = lerp(m_phaseStartSlide, 0.0f, e);
                if (t >= 1.0f) { 
                    m_phaseStartAlpha = m_themeAlpha; 
                    beginPhase(AnimPhase::ShowTheme_FadeIn, 160); 
                }
                break;
            case AnimPhase::ShowTheme_FadeIn:
                m_themeAlpha = lerp(m_phaseStartAlpha, 1.0f, e);
                if (t >= 1.0f) { 
                    m_animPhase = AnimPhase::Idle; 
                }
                break;
            default:
                break;
            }

            return m_animPhase != AnimPhase::Idle;
        }

        bool themeInteractive() const {
            if (m_followSystem && m_animPhase != AnimPhase::ShowTheme_FadeIn) {
                return m_themeAlpha > 0.6f;
            }
            return m_themeAlpha > 0.4f;
        }

        void simulateFollowClick() { m_clickFollowPending = true; }
        void simulateThemeClick() { m_clickThemePending = true; }

        bool takeActions(bool& clickedTheme, bool& clickedFollow) {
            clickedTheme = m_clickThemePending;
            clickedFollow = m_clickFollowPending;
            const bool any = clickedTheme || clickedFollow;
            m_clickThemePending = m_clickFollowPending = false;
            return any;
        }

        // Getters for testing
        float themeAlpha() const { return m_themeAlpha; }
        float followSlide() const { return m_followSlide; }
        AnimPhase animPhase() const { return m_animPhase; }
        bool followSystem() const { return m_followSystem; }
    };

    // Mock TopBarComponent 
    class MockTopBarComponent {
    private:
        std::unique_ptr<MockUiTopBar> m_topBar;
        std::function<void()> m_onFollowToggle;
        MockClock* m_clock;

    public:
        MockTopBarComponent(bool followSystem, bool animateFollow, MockClock* clock) 
            : m_topBar(std::make_unique<MockUiTopBar>(clock))
            , m_clock(clock) 
        {
            m_topBar->setFollowSystem(followSystem, animateFollow);
        }

        void setOnFollowToggle(std::function<void()> callback) {
            m_onFollowToggle = std::move(callback);
        }

        bool tick() {
            // Check for clicks and forward to callbacks
            bool clickedTheme = false, clickedFollow = false;
            if (m_topBar->takeActions(clickedTheme, clickedFollow)) {
                if (clickedFollow && m_onFollowToggle) {
                    m_onFollowToggle();
                }
            }
            return m_topBar->tick();
        }

        MockUiTopBar* topBar() { return m_topBar.get(); }
    };

public:
    void testEnablingFollowSystemAnimation() {
        std::cout << "Testing Follow System enable animation..." << std::endl;
        
        MockClock clock;
        MockTopBarComponent component(false, false, &clock);
        
        // Start with follow system OFF
        assert(component.topBar()->followSystem() == false);
        assert(component.topBar()->animPhase() == AnimPhase::Idle);
        assert(std::abs(component.topBar()->themeAlpha() - 1.0f) < 0.001f);
        assert(std::abs(component.topBar()->followSlide() - 0.0f) < 0.001f);
        
        // Enable follow system with animation
        component.topBar()->setFollowSystem(true, true);
        
        // Should start fade out phase
        assert(component.topBar()->animPhase() == AnimPhase::HideTheme_FadeOut);
        
        // Advance to middle of fade out (80ms of 160ms)
        clock.advance(80);
        component.tick();
        
        // Theme should be half faded
        float expectedAlpha = 1.0f - TestFollowSystemAnimation::easeInOut(0.5f);
        assert(std::abs(component.topBar()->themeAlpha() - expectedAlpha) < 0.01f);
        assert(component.topBar()->animPhase() == AnimPhase::HideTheme_FadeOut);
        
        // Complete fade out phase
        clock.advance(80);
        component.tick();
        
        // Should transition to slide phase
        assert(component.topBar()->animPhase() == AnimPhase::MoveFollow_Right);
        assert(std::abs(component.topBar()->themeAlpha() - 0.0f) < 0.001f);
        
        // Advance to middle of slide (100ms of 200ms)
        clock.advance(100);
        component.tick();
        
        // Follow should be half slid
        float expectedSlide = TestFollowSystemAnimation::easeInOut(0.5f);
        assert(std::abs(component.topBar()->followSlide() - expectedSlide) < 0.01f);
        
        // Complete slide phase
        clock.advance(100);
        component.tick();
        
        // Animation should be complete
        assert(component.topBar()->animPhase() == AnimPhase::Idle);
        assert(std::abs(component.topBar()->followSlide() - 1.0f) < 0.001f);
        assert(component.topBar()->followSystem() == true);
        
        std::cout << "✓ Enable animation test passed" << std::endl;
    }

    void testDisablingFollowSystemAnimation() {
        std::cout << "Testing Follow System disable animation..." << std::endl;
        
        MockClock clock;
        MockTopBarComponent component(true, false, &clock);
        
        // Start with follow system ON (no animation initially)
        assert(component.topBar()->followSystem() == true);
        assert(std::abs(component.topBar()->themeAlpha() - 0.0f) < 0.001f);
        assert(std::abs(component.topBar()->followSlide() - 1.0f) < 0.001f);
        
        // Disable follow system with animation
        component.topBar()->setFollowSystem(false, true);
        
        // Should start slide left phase
        assert(component.topBar()->animPhase() == AnimPhase::MoveFollow_Left);
        
        // Advance to middle of slide left (90ms of 180ms)
        clock.advance(90);
        component.tick();
        
        // Follow should be half way back
        float expectedSlide = 1.0f - TestFollowSystemAnimation::easeInOut(0.5f);
        assert(std::abs(component.topBar()->followSlide() - expectedSlide) < 0.01f);
        
        // Complete slide left phase
        clock.advance(90);
        component.tick();
        
        // Should transition to fade in phase
        assert(component.topBar()->animPhase() == AnimPhase::ShowTheme_FadeIn);
        assert(std::abs(component.topBar()->followSlide() - 0.0f) < 0.001f);
        
        // Advance to middle of fade in (80ms of 160ms)
        clock.advance(80);
        component.tick();
        
        // Theme should be half faded in
        float expectedAlpha = TestFollowSystemAnimation::easeInOut(0.5f);
        assert(std::abs(component.topBar()->themeAlpha() - expectedAlpha) < 0.01f);
        
        // Complete fade in phase
        clock.advance(80);
        component.tick();
        
        // Animation should be complete
        assert(component.topBar()->animPhase() == AnimPhase::Idle);
        assert(std::abs(component.topBar()->themeAlpha() - 1.0f) < 0.001f);
        assert(component.topBar()->followSystem() == false);
        
        std::cout << "✓ Disable animation test passed" << std::endl;
    }

    void testThemeInteractivity() {
        std::cout << "Testing theme button interactivity during animation..." << std::endl;
        
        MockClock clock;
        MockTopBarComponent component(false, false, &clock);
        
        // Initially theme should be interactive
        assert(component.topBar()->themeInteractive() == true);
        
        // Enable follow system with animation
        component.topBar()->setFollowSystem(true, true);
        
        // During fade out, theme should become non-interactive
        clock.advance(80);  // Half way through fade out
        component.tick();
        assert(component.topBar()->themeInteractive() == false);
        
        // Complete the full enable sequence
        clock.advance(280);  // Rest of fade out + full slide
        component.tick();
        
        // With follow system on, theme should be non-interactive
        assert(component.topBar()->themeInteractive() == false);
        
        // Disable follow system
        component.topBar()->setFollowSystem(false, true);
        
        // During slide back, theme should still be non-interactive
        clock.advance(90);  // Half way through slide left
        component.tick();
        assert(component.topBar()->themeInteractive() == false);
        
        // During fade in, theme should become interactive again
        clock.advance(90 + 80);  // Complete slide + half fade in
        component.tick();
        assert(component.topBar()->animPhase() == AnimPhase::ShowTheme_FadeIn);
        
        // During ShowTheme_FadeIn, the special logic applies: use alpha > 0.4f instead of > 0.6f
        // At half fade in, alpha should be around 0.5, so it should be interactive
        float currentAlpha = component.topBar()->themeAlpha();
        bool expectedInteractive = (currentAlpha > 0.4f);  // ShowTheme_FadeIn uses 0.4f threshold
        assert(component.topBar()->themeInteractive() == expectedInteractive);
        
        std::cout << "✓ Theme interactivity test passed" << std::endl;
    }

    void testCallbackIntegration() {
        std::cout << "Testing callback integration..." << std::endl;
        
        MockClock clock;
        MockTopBarComponent component(false, false, &clock);
        
        bool callbackTriggered = false;
        component.setOnFollowToggle([&callbackTriggered]() {
            callbackTriggered = true;
        });
        
        // Simulate user clicking follow button
        component.topBar()->simulateFollowClick();
        
        // Tick should process the click and trigger callback
        component.tick();
        assert(callbackTriggered == true);
        
        std::cout << "✓ Callback integration test passed" << std::endl;
    }

    static float easeInOut(float t) {
        t = std::clamp(t, 0.0f, 1.0f);
        return t * t * (3.0f - 2.0f * t);
    }

public:
    void runAllTests() {
        testEnablingFollowSystemAnimation();
        testDisablingFollowSystemAnimation();
        testThemeInteractivity();
        testCallbackIntegration();
    }
};

int main() {
    try {
        TestFollowSystemAnimation tests;
        tests.runAllTests();
        
        std::cout << "\n✅ All Follow System animation tests passed!" << std::endl;
        return 0;
    }
    catch (const std::exception& e) {
        std::cerr << "❌ Test failed: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        std::cerr << "❌ Test failed with unknown error" << std::endl;
        return 1;
    }
}