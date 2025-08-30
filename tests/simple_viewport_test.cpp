#include <iostream>
#include <cassert>
#include <qsize.h>
#include <qrect.h>
#include "presentation/ui/containers/UiRoot.h"
#include "presentation/ui/base/UiComponent.hpp"
#include "presentation/ui/base/UiContent.hpp" 
#include "presentation/ui/base/ILayoutable.hpp"

/**
 * Simple test to validate UiRoot viewport ordering fix
 */
class MockDeclarativeContainer : public IUiComponent, public IUiContent, public ILayoutable {
public:
    QRect m_viewport;
    QRect m_arrangeRect;
    QRect m_computedContentRect;
    bool m_updateLayoutCalled = false;
    bool m_arrangeCalledBeforeUpdate = false;
    bool m_viewportSetBeforeUpdate = false;
    
    void updateLayout(const QSize& windowSize) override {
        m_updateLayoutCalled = true;
        
        // Check if viewport and arrange were called before updateLayout
        m_viewportSetBeforeUpdate = !m_viewport.isEmpty();
        m_arrangeCalledBeforeUpdate = !m_arrangeRect.isEmpty();
        
        // Simulate what real containers do: compute layout based on current viewport
        m_computedContentRect = contentRect();
    }
    
    void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
    void append(Render::FrameData&) const override {}
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool onWheel(const QPoint&, const QPoint&) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return m_viewport; }
    void onThemeChanged(bool) override {}
    
    // IUiContent
    void setViewportRect(const QRect& r) override { 
        m_viewport = r; 
    }
    
    // ILayoutable  
    QSize measure(const SizeConstraints& cs) override {
        return QSize(std::clamp(200, cs.minW, cs.maxW), 
                   std::clamp(100, cs.minH, cs.maxH));
    }
    void arrange(const QRect& finalRect) override { 
        m_arrangeRect = finalRect; 
    }
    
private:
    QRect contentRect() const {
        // Simulate how real containers compute content area from viewport
        return m_viewport.adjusted(10, 10, -10, -10); // 10px margins
    }
};

int main() {
    std::cout << "Testing UiRoot viewport ordering fix..." << std::endl;
    
    UiRoot root;
    MockDeclarativeContainer container;
    
    // Add container to root
    root.add(&container);
    
    // Call updateLayout - this should now call viewport/arrange BEFORE updateLayout
    QSize windowSize(800, 600);
    root.updateLayout(windowSize);
    
    // Verify the fix worked
    assert(container.m_updateLayoutCalled);
    assert(container.m_viewportSetBeforeUpdate); // Viewport should be set before updateLayout
    assert(container.m_arrangeCalledBeforeUpdate); // Arrange should be called before updateLayout
    
    // Verify correct viewport was set
    assert(container.m_viewport == QRect(0, 0, 800, 600));
    assert(container.m_arrangeRect == QRect(0, 0, 800, 600));
    
    // Verify container computed content rect correctly with valid viewport
    assert(container.m_computedContentRect == QRect(10, 10, 780, 580));
    
    std::cout << "✅ UiRoot viewport ordering fix test PASSED!" << std::endl;
    std::cout << "✅ Container had valid viewport (" << container.m_viewport.width() << "x" << container.m_viewport.height() << ") during updateLayout" << std::endl;
    std::cout << "✅ Content rect computed correctly: " << container.m_computedContentRect.width() << "x" << container.m_computedContentRect.height() << std::endl;
    
    return 0;
}