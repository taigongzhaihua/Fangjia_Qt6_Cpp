#include <iostream>
#include <QSize>
#include <QRect>
#include "src/framework/widgets/UiTabView.h"
#include "src/models/TabViewModel.h"

// Mock implementation for testing
class MockTabContent : public IUiComponent {
public:
    bool resourceContextUpdated = false;
    bool layoutUpdated = false;
    bool themeChanged = false;
    QRect viewport;
    
    void updateLayout(const QSize& windowSize) override {
        layoutUpdated = true;
        std::cout << "MockTabContent::updateLayout called with size " 
                  << windowSize.width() << "x" << windowSize.height() << std::endl;
    }
    
    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override {
        resourceContextUpdated = true;
        std::cout << "MockTabContent::updateResourceContext called with DPR " << devicePixelRatio << std::endl;
    }
    
    void onThemeChanged(bool isDark) override {
        themeChanged = true;
        std::cout << "MockTabContent::onThemeChanged called with isDark=" << isDark << std::endl;
    }
    
    void append(Render::FrameData& fd) const override {}
    bool onMousePress(const QPoint& pos) override { return false; }
    bool onMouseMove(const QPoint& pos) override { return false; }
    bool onMouseRelease(const QPoint& pos) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return QRect(0, 0, 100, 100); }
};

class MockTabContentWithViewport : public MockTabContent, public IUiContent {
public:
    void setViewportRect(const QRect& r) override {
        viewport = r;
        std::cout << "MockTabContentWithViewport::setViewportRect called with rect " 
                  << r.x() << "," << r.y() << " " << r.width() << "x" << r.height() << std::endl;
    }
};

int main() {
    std::cout << "Testing TabView fix for content mounting and theme propagation..." << std::endl;
    
    // Test 1: Test mountSelectedContent() functionality
    std::cout << "\n=== Test 1: mountSelectedContent() functionality ===" << std::endl;
    
    UiTabView tabView;
    tabView.setViewportRect(QRect(0, 0, 800, 600));
    
    MockTabContentWithViewport content1;
    MockTabContent content2;
    
    tabView.setContent(0, &content1);
    tabView.setContent(1, &content2);
    
    tabView.setTabs({"Tab 1", "Tab 2"});
    
    // Simulate having resource context
    // Note: In real code, these would be set by updateResourceContext
    // For testing, we'll just verify the method calls happen
    
    // Test fallback mode selection change
    std::cout << "\nTesting fallback mode selection change..." << std::endl;
    tabView.setSelectedIndex(0);
    
    // Test VM mode
    std::cout << "\nTesting VM mode..." << std::endl;
    TabViewModel vm;
    vm.setItems({
        {.id = "tab1", .label = "Tab 1"},
        {.id = "tab2", .label = "Tab 2"}
    });
    
    tabView.setViewModel(&vm);
    vm.setSelectedIndex(1);
    
    // Test theme propagation
    std::cout << "\n=== Test 2: Theme propagation ===" << std::endl;
    tabView.onThemeChanged(true);
    
    if (content2.themeChanged) {
        std::cout << "✓ Theme change was propagated to selected content" << std::endl;
    } else {
        std::cout << "✗ Theme change was NOT propagated to selected content" << std::endl;
    }
    
    std::cout << "\nTabView fix testing completed." << std::endl;
    return 0;
}