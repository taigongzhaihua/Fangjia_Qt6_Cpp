#include <iostream>
#include <cassert>
#include <qsize.h>
#include <qrect.h>
#include "presentation/ui/containers/UiPage.h"
#include "presentation/ui/base/UiComponent.hpp"
#include "presentation/ui/base/UiContent.hpp" 
#include "presentation/ui/base/ILayoutable.hpp"

/**
 * Test to validate UiPage layout contract implementation
 * Tests that UiPage now properly implements ILayoutable and IUiContent
 * and forwards arrange/viewport calls to child content
 */
class MockPageContent : public IUiComponent, public IUiContent, public ILayoutable {
public:
    QRect m_viewport;
    QRect m_arrangeRect;
    QSize m_measureRequest;
    SizeConstraints m_lastConstraints;
    bool m_setViewportCalled = false;
    bool m_arrangeCalled = false;
    bool m_measureCalled = false;
    
    void updateLayout(const QSize& windowSize) override {}
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
        m_setViewportCalled = true;
    }
    
    // ILayoutable  
    QSize measure(const SizeConstraints& cs) override {
        m_lastConstraints = cs;
        m_measureCalled = true;
        m_measureRequest = QSize(std::clamp(150, cs.minW, cs.maxW), 
                               std::clamp(100, cs.minH, cs.maxH));
        return m_measureRequest;
    }
    void arrange(const QRect& finalRect) override { 
        m_arrangeRect = finalRect; 
        m_arrangeCalled = true;
    }
};

int main() {
    std::cout << "Testing UiPage layout contract implementation..." << std::endl;
    
    UiPage page;
    MockPageContent content;
    
    // Set up page with content
    page.setContent(&content);
    page.setTitle("Test Page");
    
    // Test 1: UiPage should implement ILayoutable
    std::cout << "Test 1: Checking ILayoutable implementation..." << std::endl;
    ILayoutable* layoutable = dynamic_cast<ILayoutable*>(&page);
    assert(layoutable != nullptr);
    std::cout << "✅ UiPage implements ILayoutable" << std::endl;
    
    // Test 2: UiPage should implement IUiContent
    std::cout << "Test 2: Checking IUiContent implementation..." << std::endl;
    IUiContent* uiContent = dynamic_cast<IUiContent*>(&page);
    assert(uiContent != nullptr);
    std::cout << "✅ UiPage implements IUiContent" << std::endl;
    
    // Test 3: Test measure() method
    std::cout << "Test 3: Testing measure() method..." << std::endl;
    SizeConstraints cs;
    cs.minW = 100; cs.minH = 50;
    cs.maxW = 800; cs.maxH = 600;
    
    QSize measured = layoutable->measure(cs);
    assert(content.m_measureCalled);
    
    // Page should account for margins + padding + title area (kTitleAreaH = 84)
    const int frameW = page.margins().left() + page.margins().right() + 
                      page.padding().left() + page.padding().right();
    const int frameH = page.margins().top() + page.margins().bottom() + 
                      page.padding().top() + page.padding().bottom() + 84; // kTitleAreaH
    
    // Measured size should be content size + frame
    QSize expectedSize(content.m_measureRequest.width() + frameW, 
                      content.m_measureRequest.height() + frameH);
    expectedSize.setWidth(std::clamp(expectedSize.width(), cs.minW, cs.maxW));
    expectedSize.setHeight(std::clamp(expectedSize.height(), cs.minH, cs.maxH));
    
    assert(measured == expectedSize);
    std::cout << "✅ measure() returns correct size: " << measured.width() << "x" << measured.height() << std::endl;
    
    // Test 4: Test arrange() method
    std::cout << "Test 4: Testing arrange() method..." << std::endl;
    content.m_setViewportCalled = false;
    content.m_arrangeCalled = false;
    
    QRect finalRect(0, 0, 400, 300);
    layoutable->arrange(finalRect);
    
    // Page should have stored viewport
    assert(page.bounds() == finalRect);
    
    // Content should have received setViewportRect and arrange calls
    assert(content.m_setViewportCalled);
    assert(content.m_arrangeCalled);
    
    // Content viewport should be within the page's content area
    QRectF expectedContentRect = page.contentRectF();
    assert(content.m_viewport == expectedContentRect.toRect());
    assert(content.m_arrangeRect == expectedContentRect.toRect());
    
    std::cout << "✅ arrange() forwards correctly to content" << std::endl;
    std::cout << "✅ Content viewport: " << content.m_viewport.width() << "x" << content.m_viewport.height() << std::endl;
    
    // Test 5: Test setViewportRect() method 
    std::cout << "Test 5: Testing setViewportRect() method..." << std::endl;
    content.m_setViewportCalled = false;
    content.m_arrangeCalled = false;
    
    QRect newViewport(50, 50, 350, 250);
    uiContent->setViewportRect(newViewport);
    
    assert(page.bounds() == newViewport);
    std::cout << "✅ setViewportRect() works correctly" << std::endl;
    
    std::cout << "\n🎉 All UiPage layout contract tests PASSED!" << std::endl;
    std::cout << "✅ UiPage now properly implements ILayoutable and IUiContent" << std::endl;
    std::cout << "✅ UiPage forwards arrange/viewport calls to child content" << std::endl;
    std::cout << "✅ UiPage measurement considers margins + padding + title area" << std::endl;
    
    return 0;
}