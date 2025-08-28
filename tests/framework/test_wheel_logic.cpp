#include <iostream>
#include <cassert>
#include <vector>

// Simplified mock classes to test our logic without Qt dependencies

struct QPoint {
    int x, y;
    QPoint(int x = 0, int y = 0) : x(x), y(y) {}
    bool operator==(const QPoint& other) const { return x == other.x && y == other.y; }
};

struct QRect {
    int x, y, w, h;
    QRect(int x = 0, int y = 0, int w = 0, int h = 0) : x(x), y(y), w(w), h(h) {}
    bool contains(const QPoint& p) const {
        return p.x >= x && p.x < x + w && p.y >= y && p.y < y + h;
    }
    bool isValid() const { return w > 0 && h > 0; }
};

// Mock component interface
class MockComponent {
public:
    bool wheelCalled = false;
    QPoint lastPos;
    QPoint lastAngle;
    
    virtual ~MockComponent() = default;
    
    void reset() { wheelCalled = false; lastPos = QPoint(); lastAngle = QPoint(); }
    
    virtual bool onWheel(const QPoint& pos, const QPoint& angleDelta) {
        wheelCalled = true;
        lastPos = pos;
        lastAngle = angleDelta;
        return true;
    }
};

// Test our container logic patterns
class MockContainer {
private:
    QRect m_viewport;
    MockComponent* m_child = nullptr;
    
public:
    void setViewportRect(const QRect& r) { m_viewport = r; }
    void setChild(MockComponent* c) { m_child = c; }
    
    // UiContainer-style onWheel
    bool onWheel(const QPoint& pos, const QPoint& angleDelta) {
        return m_child ? m_child->onWheel(pos, angleDelta) : false;
    }
};

class MockPanel {
private:
    QRect m_viewport;
    std::vector<MockComponent*> m_children;
    
public:
    void setViewportRect(const QRect& r) { m_viewport = r; }
    void addChild(MockComponent* c) { if (c) m_children.push_back(c); }
    
    // UiPanel-style onWheel (reverse order)
    bool onWheel(const QPoint& pos, const QPoint& angleDelta) {
        if (!m_viewport.contains(pos)) return false;
        // Reverse iteration (last added first)
        for (auto it = m_children.rbegin(); it != m_children.rend(); ++it) {
            if ((*it) && (*it)->onWheel(pos, angleDelta)) {
                return true;
            }
        }
        return false;
    }
};

class MockTabView {
private:
    QRect m_viewport;
    QRect m_contentRect;
    MockComponent* m_currentContent = nullptr;
    
public:
    void setViewportRect(const QRect& r) { 
        m_viewport = r; 
        // Simulate content area below tab bar (tab bar height = 50)
        m_contentRect = QRect(r.x + 10, r.y + 50, r.w - 20, r.h - 60);
    }
    void setCurrentContent(MockComponent* c) { m_currentContent = c; }
    
    // UiTabView-style onWheel (only in content area)
    bool onWheel(const QPoint& pos, const QPoint& angleDelta) {
        if (!m_viewport.contains(pos)) return false;
        if (m_contentRect.contains(pos)) {
            if (m_currentContent) {
                return m_currentContent->onWheel(pos, angleDelta);
            }
        }
        return false;
    }
};

void testContainerLogic() {
    std::cout << "Testing Container Logic..." << std::endl;
    
    MockContainer container;
    MockComponent child;
    
    container.setViewportRect(QRect(0, 0, 100, 100));
    container.setChild(&child);
    
    // Test successful forwarding
    bool result = container.onWheel(QPoint(50, 50), QPoint(0, 120));
    assert(result == true);
    assert(child.wheelCalled == true);
    assert(child.lastPos == QPoint(50, 50));
    assert(child.lastAngle == QPoint(0, 120));
    
    // Test with no child
    container.setChild(nullptr);
    child.reset();
    result = container.onWheel(QPoint(50, 50), QPoint(0, 120));
    assert(result == false);
    assert(child.wheelCalled == false);
    
    std::cout << "Container Logic - PASSED ✅" << std::endl;
}

void testPanelLogic() {
    std::cout << "Testing Panel Logic..." << std::endl;
    
    MockPanel panel;
    MockComponent child1, child2;
    
    panel.setViewportRect(QRect(0, 0, 100, 100));
    panel.addChild(&child1);
    panel.addChild(&child2);
    
    // Test reverse order (child2 added last, should be called first)
    bool result = panel.onWheel(QPoint(50, 50), QPoint(0, 120));
    assert(result == true);
    assert(child2.wheelCalled == true);  // Last added, first called
    assert(child1.wheelCalled == false); // Not called since child2 consumed
    
    // Test outside viewport
    child1.reset();
    child2.reset();
    result = panel.onWheel(QPoint(150, 150), QPoint(0, 120));
    assert(result == false);
    assert(child1.wheelCalled == false);
    assert(child2.wheelCalled == false);
    
    std::cout << "Panel Logic - PASSED ✅" << std::endl;
}

void testTabViewLogic() {
    std::cout << "Testing TabView Logic..." << std::endl;
    
    MockTabView tabView;
    MockComponent content;
    
    tabView.setViewportRect(QRect(0, 0, 200, 200));
    tabView.setCurrentContent(&content);
    
    // Test in content area (should forward)
    bool result = tabView.onWheel(QPoint(100, 100), QPoint(0, 120));
    assert(result == true);
    assert(content.wheelCalled == true);
    
    // Test in tab bar area (should not forward)
    content.reset();
    result = tabView.onWheel(QPoint(100, 25), QPoint(0, 120)); // In tab bar area
    assert(result == false);
    assert(content.wheelCalled == false);
    
    // Test outside viewport
    content.reset();
    result = tabView.onWheel(QPoint(300, 300), QPoint(0, 120));
    assert(result == false);
    assert(content.wheelCalled == false);
    
    std::cout << "TabView Logic - PASSED ✅" << std::endl;
}

int main() {
    std::cout << "=== Testing Wheel Forwarding Logic ===" << std::endl;
    
    testContainerLogic();
    testPanelLogic(); 
    testTabViewLogic();
    
    std::cout << "\nAll tests PASSED! 🎉" << std::endl;
    std::cout << "The onWheel forwarding implementation logic is correct." << std::endl;
    
    return 0;
}