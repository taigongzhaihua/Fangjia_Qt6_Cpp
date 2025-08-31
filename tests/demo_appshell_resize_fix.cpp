#include <iostream>
#include <cassert>
#include <vector>
#include <string>

/**
 * Mock demonstration of AppShell content resizing with the fix
 * Shows how UiPage now participates in layout contract and forwards
 * arrange/viewport changes to child content, enabling proper shrinking
 */

struct QRect {
    int x, y, w, h;
    QRect(int x=0, int y=0, int w=0, int h=0) : x(x), y(y), w(w), h(h) {}
    bool isEmpty() const { return w <= 0 || h <= 0; }
    int width() const { return w; }
    int height() const { return h; }
    QRect adjusted(int l, int t, int r, int b) const { return QRect(x+l, y+t, w+r-l, h+b-t); }
};

struct QSize {
    int w, h;
    QSize(int w=0, int h=0) : w(w), h(h) {}
    int width() const { return w; }
    int height() const { return h; }
};

// Mock interfaces
class IUiContent {
public:
    virtual ~IUiContent() = default;
    virtual void setViewportRect(const QRect& r) = 0;
};

struct SizeConstraints {
    int minW = 0, minH = 0;
    int maxW = 9999, maxH = 9999;
};

class ILayoutable {
public:
    virtual ~ILayoutable() = default;
    virtual QSize measure(const SizeConstraints& cs) = 0;
    virtual void arrange(const QRect& finalRect) = 0;
};

// Mock UiPage (simplified version with our fixes)
class MockUiPage : public IUiContent, public ILayoutable {
private:
    QRect m_viewport;
    IUiContent* m_content = nullptr;
    ILayoutable* m_layoutableContent = nullptr;
    static constexpr int kTitleAreaH = 84;
    static constexpr int kMargins = 8;
    static constexpr int kPadding = 16;
    
public:
    void setContent(IUiContent* content, ILayoutable* layoutable = nullptr) {
        m_content = content;
        m_layoutableContent = layoutable;
    }
    
    QRect contentRect() const {
        // Content area = viewport - margins - padding - title area
        return m_viewport.adjusted(kMargins + kPadding, kMargins + kPadding + kTitleAreaH, 
                                 -(kMargins + kPadding), -(kMargins + kPadding));
    }
    
    // IUiContent implementation
    void setViewportRect(const QRect& r) override { 
        m_viewport = r; 
        
        // Forward to child content (this is the key fix!)
        if (m_content) {
            QRect childRect = contentRect();
            m_content->setViewportRect(childRect);
        }
    }
    
    // ILayoutable implementation (new!)
    QSize measure(const SizeConstraints& cs) override {
        const int frameW = 2 * (kMargins + kPadding);
        const int frameH = 2 * (kMargins + kPadding) + kTitleAreaH;
        
        QSize contentSize(0, 0);
        if (m_layoutableContent) {
            SizeConstraints childCs;
            childCs.maxW = std::max(0, cs.maxW - frameW);
            childCs.maxH = std::max(0, cs.maxH - frameH);
            contentSize = m_layoutableContent->measure(childCs);
        }
        
        return QSize(contentSize.width() + frameW, contentSize.height() + frameH);
    }
    
    void arrange(const QRect& finalRect) override {
        // Store viewport and forward to child (this is the key fix!)
        m_viewport = finalRect;
        
        if (m_content) {
            QRect childRect = contentRect();
            m_content->setViewportRect(childRect);
        }
        
        if (m_layoutableContent) {
            QRect childRect = contentRect();
            m_layoutableContent->arrange(childRect);
        }
    }
    
    QRect bounds() const { return m_viewport; }
};

// Mock page content that tracks resize events
class MockPageContent : public IUiContent, public ILayoutable {
public:
    QRect m_viewport;
    std::vector<std::string> m_resizeLog;
    
    void setViewportRect(const QRect& r) override { 
        m_viewport = r; 
        m_resizeLog.push_back("setViewport: " + std::to_string(r.width()) + "x" + std::to_string(r.height()));
    }
    
    QSize measure(const SizeConstraints& cs) override {
        return QSize(std::min(300, cs.maxW), std::min(200, cs.maxH));
    }
    
    void arrange(const QRect& finalRect) override {
        m_resizeLog.push_back("arrange: " + std::to_string(finalRect.width()) + "x" + std::to_string(finalRect.height()));
    }
};

// Mock AppShell using Grid layout (simplified)
class MockAppShell : public ILayoutable {
private:
    MockUiPage* m_page = nullptr;
    
public:
    void setContent(MockUiPage* page) { m_page = page; }
    
    QSize measure(const SizeConstraints& cs) override {
        return QSize(cs.maxW, cs.maxH); // Shell takes full space
    }
    
    void arrange(const QRect& finalRect) override {
        if (m_page) {
            // In real AppShell, this would be computed by Grid layout
            // Content area excludes nav (200px) and top bar (42px)
            QRect contentArea(200, 42, finalRect.width() - 200, finalRect.height() - 42);
            
            // Forward to page (which now implements ILayoutable!)
            m_page->arrange(contentArea);
        }
    }
};

int main() {
    std::cout << "🔍 Testing AppShell content resize behavior with UiPage layout contract fix...\n" << std::endl;
    
    MockPageContent content;
    MockUiPage page;
    MockAppShell shell;
    
    // Set up the hierarchy
    page.setContent(&content, &content);
    shell.setContent(&page);
    
    std::cout << "📏 Test 1: Initial layout (800x600 window)" << std::endl;
    QRect initialWindow(0, 0, 800, 600);
    shell.arrange(initialWindow);
    
    std::cout << "   Page viewport: " << page.bounds().width() << "x" << page.bounds().height() << std::endl;
    std::cout << "   Content viewport: " << content.m_viewport.width() << "x" << content.m_viewport.height() << std::endl;
    std::cout << "   Resize events: " << content.m_resizeLog.size() << std::endl;
    
    // Verify initial layout
    assert(!content.m_viewport.isEmpty());
    assert(content.m_resizeLog.size() >= 1);
    
    std::cout << "\n📏 Test 2: Window expands (1200x800)" << std::endl;
    content.m_resizeLog.clear();
    QRect expandedWindow(0, 0, 1200, 800);
    shell.arrange(expandedWindow);
    
    std::cout << "   Page viewport: " << page.bounds().width() << "x" << page.bounds().height() << std::endl;
    std::cout << "   Content viewport: " << content.m_viewport.width() << "x" << content.m_viewport.height() << std::endl;
    std::cout << "   Resize events: " << content.m_resizeLog.size() << std::endl;
    
    QRect expandedContentViewport = content.m_viewport;
    
    std::cout << "\n📏 Test 3: Window shrinks (600x400) - THE KEY TEST!" << std::endl;
    content.m_resizeLog.clear();
    QRect shrunkWindow(0, 0, 600, 400);
    shell.arrange(shrunkWindow);
    
    std::cout << "   Page viewport: " << page.bounds().width() << "x" << page.bounds().height() << std::endl;
    std::cout << "   Content viewport: " << content.m_viewport.width() << "x" << content.m_viewport.height() << std::endl;
    std::cout << "   Resize events: " << content.m_resizeLog.size() << std::endl;
    
    // Verify shrinking works
    assert(content.m_viewport.width() < expandedContentViewport.width());
    assert(content.m_viewport.height() < expandedContentViewport.height());
    assert(content.m_resizeLog.size() >= 1);
    
    std::cout << "\n📏 Test 4: Very small window (300x200) - stress test" << std::endl;
    content.m_resizeLog.clear();
    QRect tinyWindow(0, 0, 300, 200);
    shell.arrange(tinyWindow);
    
    std::cout << "   Page viewport: " << page.bounds().width() << "x" << page.bounds().height() << std::endl;
    std::cout << "   Content viewport: " << content.m_viewport.width() << "x" << content.m_viewport.height() << std::endl;
    std::cout << "   Resize events: " << content.m_resizeLog.size() << std::endl;
    
    // Content might be very small but should still receive events
    assert(content.m_resizeLog.size() >= 1);
    
    std::cout << "\n🎉 All resize tests PASSED!" << std::endl;
    std::cout << "✅ UiPage now participates in layout contract (implements ILayoutable)" << std::endl;
    std::cout << "✅ UiPage forwards arrange() and setViewportRect() to child content" << std::endl;
    std::cout << "✅ AppShell content resizes correctly in BOTH directions (expand AND shrink)" << std::endl;
    std::cout << "✅ Content receives resize events on every size change" << std::endl;
    
    std::cout << "\n📋 Full resize log:" << std::endl;
    shell.arrange(QRect(0, 0, 800, 600)); // Reset for final log
    for (const auto& event : content.m_resizeLog) {
        std::cout << "   " << event << std::endl;
    }
    
    return 0;
}