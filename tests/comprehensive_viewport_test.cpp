#include <iostream>
#include <cassert>
#include <memory>
#include <qsize.h>
#include <qrect.h>
#include "presentation/ui/containers/UiRoot.h"
#include "presentation/ui/base/UiComponent.hpp"
#include "presentation/ui/base/UiContent.hpp" 
#include "presentation/ui/base/ILayoutable.hpp"

/**
 * Comprehensive test for UiRoot viewport ordering fix with multiple scenarios
 */

// Mock component that implements only IUiContent (not ILayoutable)
class MockContentOnly : public IUiComponent, public IUiContent {
public:
    QRect m_viewport;
    bool m_updateLayoutCalled = false;
    bool m_viewportSetBeforeUpdate = false;
    
    void updateLayout(const QSize&) override {
        m_updateLayoutCalled = true;
        m_viewportSetBeforeUpdate = !m_viewport.isEmpty();
    }
    
    void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
    void append(Render::FrameData&) const override {}
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool onWheel(const QPoint&, const QPoint&) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return m_viewport.isEmpty() ? QRect(0,0,1,1) : m_viewport; }
    void onThemeChanged(bool) override {}
    
    void setViewportRect(const QRect& r) override { m_viewport = r; }
};

// Mock component that implements only ILayoutable (not IUiContent)
class MockLayoutableOnly : public IUiComponent, public ILayoutable {
public:
    QRect m_arrangeRect;
    bool m_updateLayoutCalled = false;
    bool m_arrangeCalledBeforeUpdate = false;
    
    void updateLayout(const QSize&) override {
        m_updateLayoutCalled = true;
        m_arrangeCalledBeforeUpdate = !m_arrangeRect.isEmpty();
    }
    
    void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
    void append(Render::FrameData&) const override {}
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool onWheel(const QPoint&, const QPoint&) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return m_arrangeRect.isEmpty() ? QRect(0,0,1,1) : m_arrangeRect; }
    void onThemeChanged(bool) override {}
    
    QSize measure(const SizeConstraints& cs) override {
        return QSize(std::clamp(100, cs.minW, cs.maxW), 
                   std::clamp(50, cs.minH, cs.maxH));
    }
    void arrange(const QRect& finalRect) override { m_arrangeRect = finalRect; }
};

// Mock simple component that implements neither
class MockSimpleComponent : public IUiComponent {
public:
    bool m_updateLayoutCalled = false;
    
    void updateLayout(const QSize&) override { m_updateLayoutCalled = true; }
    void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
    void append(Render::FrameData&) const override {}
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool onWheel(const QPoint&, const QPoint&) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return QRect(0, 0, 100, 50); }
    void onThemeChanged(bool) override {}
};

int main() {
    std::cout << "Testing UiRoot comprehensive viewport ordering..." << std::endl;
    
    UiRoot root;
    MockContentOnly contentOnly;
    MockLayoutableOnly layoutableOnly;  
    MockSimpleComponent simpleComponent;
    
    // Add all types of components
    root.add(&contentOnly);
    root.add(&layoutableOnly);
    root.add(&simpleComponent);
    
    // Call updateLayout
    QSize windowSize(1024, 768);
    root.updateLayout(windowSize);
    
    // Test 1: IUiContent-only component should have viewport set before updateLayout
    assert(contentOnly.m_updateLayoutCalled);
    assert(contentOnly.m_viewportSetBeforeUpdate);
    assert(contentOnly.m_viewport == QRect(0, 0, 1024, 768));
    std::cout << "✅ IUiContent-only component: viewport set before updateLayout" << std::endl;
    
    // Test 2: ILayoutable-only component should have arrange called before updateLayout
    assert(layoutableOnly.m_updateLayoutCalled);
    assert(layoutableOnly.m_arrangeCalledBeforeUpdate);
    assert(layoutableOnly.m_arrangeRect == QRect(0, 0, 1024, 768));
    std::cout << "✅ ILayoutable-only component: arrange called before updateLayout" << std::endl;
    
    // Test 3: Simple component should still have updateLayout called
    assert(simpleComponent.m_updateLayoutCalled);
    std::cout << "✅ Simple component: updateLayout called correctly" << std::endl;
    
    // Test 4: Verify bounds are computed correctly after layout
    assert(contentOnly.bounds() == QRect(0, 0, 1024, 768));
    assert(layoutableOnly.bounds() == QRect(0, 0, 1024, 768));
    assert(simpleComponent.bounds() == QRect(0, 0, 100, 50));
    std::cout << "✅ All components have correct bounds after layout" << std::endl;
    
    std::cout << "✅ All comprehensive tests PASSED!" << std::endl;
    
    // Test window resize behavior
    std::cout << "\nTesting window resize behavior..." << std::endl;
    
    QSize newWindowSize(1920, 1080);
    root.updateLayout(newWindowSize);
    
    // Verify components adapt to new window size
    assert(contentOnly.m_viewport == QRect(0, 0, 1920, 1080));
    assert(layoutableOnly.m_arrangeRect == QRect(0, 0, 1920, 1080));
    std::cout << "✅ Components correctly adapt to window resize" << std::endl;
    
    std::cout << "✅ ALL TESTS PASSED - UiRoot fix is working correctly!" << std::endl;
    return 0;
}