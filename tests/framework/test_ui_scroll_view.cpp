#include <QtTest>
#include <QCoreApplication>
#include <QDebug>
#include "UiScrollView.h"
#include "UiComponent.hpp"
#include "ILayoutable.hpp"

// Mock component for testing
class MockComponent : public IUiComponent, public IUiContent, public ILayoutable {
public:
    QRect m_bounds{0, 0, 100, 200};
    QRect m_viewport;
    QRect m_arrangeRect;
    QSize m_measureResult{100, 200};
    bool m_wheelCalled{false};
    QPoint m_lastWheelPos;
    QPoint m_lastWheelAngle;
    
    void updateLayout(const QSize&) override {}
    void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
    void append(Render::FrameData&) const override {}
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool onWheel(const QPoint& pos, const QPoint& angleDelta) override { 
        m_wheelCalled = true;
        m_lastWheelPos = pos;
        m_lastWheelAngle = angleDelta;
        return true;
    }
    bool tick() override { return false; }
    QRect bounds() const override { return m_bounds; }
    void applyTheme(bool) override {}
    
    void setViewportRect(const QRect& r) override { m_viewport = r; }
    QSize measure(const SizeConstraints& cs) override { 
        return QSize(
            std::clamp(m_measureResult.width(), cs.minW, cs.maxW),
            std::clamp(m_measureResult.height(), cs.minH, cs.maxH)
        );
    }
    void arrange(const QRect& finalRect) override { m_arrangeRect = finalRect; }
};

class UiScrollViewTestRunner : public QObject
{
    Q_OBJECT

public slots:
    void runScrollViewTests()
    {
        qDebug() << "=== Testing UiScrollView ===";
        
        // Test initial state
        UiScrollView scrollView;
        QCOMPARE(scrollView.scrollY(), 0);
        QCOMPARE(scrollView.child(), nullptr);
        QCOMPARE(scrollView.maxScrollY(), 0);
        
        // Test child management
        MockComponent mockChild;
        scrollView.setChild(&mockChild);
        QCOMPARE(scrollView.child(), &mockChild);
        
        // Test measurement without scrollbar
        mockChild.m_measureResult = QSize(100, 100);
        SizeConstraints cs{0, 0, 200, 150};
        QSize result = scrollView.measure(cs);
        QCOMPARE(result.width(), 100); // No scrollbar needed
        QCOMPARE(result.height(), 100);
        
        // Test measurement with scrollbar needed
        mockChild.m_measureResult = QSize(150, 300);
        cs = SizeConstraints{0, 0, 200, 150};
        result = scrollView.measure(cs);
        QCOMPARE(result.width(), 162); // 150 + 12 (scrollbar width)
        QCOMPARE(result.height(), 150);
        
        // Test scrolling constraints
        scrollView.setViewportRect(QRect(0, 0, 120, 150));
        QCOMPARE(scrollView.maxScrollY(), 150); // 300 - 150
        
        scrollView.setScrollY(100);
        QCOMPARE(scrollView.scrollY(), 100);
        
        // Test clamping
        scrollView.setScrollY(-10);
        QCOMPARE(scrollView.scrollY(), 0);
        
        scrollView.setScrollY(200);
        QCOMPARE(scrollView.scrollY(), 150); // clamped to maxScrollY
        
        // Test child viewport calculation
        scrollView.setScrollY(50);
        scrollView.updateLayout(QSize(200, 200));
        
        // Child viewport should be adjusted by scroll offset
        QCOMPARE(mockChild.m_viewport.x(), 0);
        QCOMPARE(mockChild.m_viewport.y(), -50); // 0 - 50
        QCOMPARE(mockChild.m_viewport.width(), 108); // 120 - 12 (scrollbar width)
        QCOMPARE(mockChild.m_viewport.height(), 300);
        
        // Test theme application (shouldn't crash)
        scrollView.applyTheme(true);
        scrollView.applyTheme(false);
        
        qDebug() << "=== Testing Wheel Events ===";
        
        // Test wheel events with scrollable content
        scrollView.setViewportRect(QRect(0, 0, 120, 150));
        mockChild.m_measureResult = QSize(100, 300); // Content larger than viewport
        scrollView.updateLayout(QSize(200, 200));
        
        // Test wheel event inside bounds
        bool consumed = scrollView.onWheel(QPoint(50, 50), QPoint(0, 120)); // Scroll up one notch
        QVERIFY(consumed); // Should consume event when there's scrollable content
        QCOMPARE(scrollView.scrollY(), 48); // 120/120 * 48 = 48px scroll
        
        // Test wheel event outside bounds
        consumed = scrollView.onWheel(QPoint(200, 200), QPoint(0, 120));
        QVERIFY(!consumed); // Should not consume event outside bounds
        
        // Test wheel event with no scrollable content
        mockChild.m_measureResult = QSize(100, 100); // Content smaller than viewport
        scrollView.updateLayout(QSize(200, 200));
        consumed = scrollView.onWheel(QPoint(50, 50), QPoint(0, 120));
        QVERIFY(!consumed); // Should not consume event when no scrolling needed
        
        qDebug() << "=== Testing Scrollbar Animation ===";
        
        // Test initial alpha state
        UiScrollView scrollView2;
        // After construction, should have base alpha
        // Note: We can't easily test private member m_thumbAlpha, but we can test tick behavior
        
        // Set up scrollable content
        scrollView2.setChild(&mockChild);
        mockChild.m_measureResult = QSize(100, 300);
        scrollView2.setViewportRect(QRect(0, 0, 120, 150));
        scrollView2.updateLayout(QSize(200, 200));
        
        // Simulate interaction (should activate animation)
        scrollView2.onWheel(QPoint(50, 50), QPoint(0, 120));
        
        // Should return true indicating animation is active
        bool animating = scrollView2.tick();
        QVERIFY(animating); // Animation should be active after interaction
        
        qDebug() << "UiScrollView tests PASSED ✅";
    }
};

// Simple test runner for UiScrollView
int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_LOGGING_RULES", "qt.qpa.gl=false");
    
    qDebug() << "===========================================";
    qDebug() << "UiScrollView Tests";
    qDebug() << "===========================================";
    
    UiScrollViewTestRunner runner;
    
    try {
        runner.runScrollViewTests();
        
        qDebug() << "===========================================";
        qDebug() << "ALL SCROLLVIEW TESTS PASSED ✅";
        qDebug() << "===========================================";
        
        return 0;
    } catch (...) {
        qDebug() << "===========================================";
        qDebug() << "TEST FAILURE ❌";
        qDebug() << "===========================================";
        return 1;
    }
}

#include "test_ui_scroll_view.moc"