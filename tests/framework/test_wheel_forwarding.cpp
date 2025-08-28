#include <QtTest>
#include <QCoreApplication>
#include <QDebug>
#include "UiTabView.h"
#include "UiPanel.h"
#include "UiContainer.h"
#include "UiGrid.h"
#include "RebuildHost.h"
#include "UiComponent.hpp"
#include "TabViewModel.h"

// Mock component for testing wheel event forwarding
class MockWheelComponent : public IUiComponent {
public:
    bool m_wheelCalled{false};
    QPoint m_lastWheelPos;
    QPoint m_lastWheelAngle;
    
    void reset() { m_wheelCalled = false; m_lastWheelPos = QPoint(); m_lastWheelAngle = QPoint(); }
    
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
    QRect bounds() const override { return QRect(0, 0, 100, 100); }
    void applyTheme(bool) override {}
};

// Mock TabViewModel for testing
class MockTabViewModel : public TabViewModel {
public:
    MockTabViewModel() {
        setTabs({"Tab1", "Tab2"});
        setSelectedIndex(0);
    }
    void setTabs(const QStringList& tabs) { m_tabs = tabs; }
    void setSelectedIndex(int idx) { m_selectedIndex = std::max(0, std::min(idx, m_tabs.size() - 1)); }
    
    int tabCount() const override { return m_tabs.size(); }
    QString tabLabel(int i) const override { return i >= 0 && i < m_tabs.size() ? m_tabs[i] : QString(); }
    int selectedIndex() const override { return m_selectedIndex; }
    
private:
    QStringList m_tabs;
    int m_selectedIndex{0};
};

class WheelForwardingTestRunner : public QObject
{
    Q_OBJECT

public slots:
    void runWheelForwardingTests()
    {
        qDebug() << "=== Testing Wheel Event Forwarding ===";
        
        testUiContainerWheelForwarding();
        testUiPanelWheelForwarding();
        testUiGridWheelForwarding();
        testUiTabViewWheelForwarding();
        testRebuildHostWheelForwarding();
        
        qDebug() << "All wheel forwarding tests PASSED ✅";
    }

private:
    void testUiContainerWheelForwarding()
    {
        qDebug() << "Testing UiContainer wheel forwarding...";
        
        UiContainer container;
        MockWheelComponent mockChild;
        
        container.setViewportRect(QRect(0, 0, 200, 200));
        container.setChild(&mockChild);
        
        // Test wheel event forwarding
        QPoint testPos(50, 50);
        QPoint testAngle(0, 120);
        
        bool consumed = container.onWheel(testPos, testAngle);
        
        QVERIFY(consumed);
        QVERIFY(mockChild.m_wheelCalled);
        QCOMPARE(mockChild.m_lastWheelPos, testPos);
        QCOMPARE(mockChild.m_lastWheelAngle, testAngle);
        
        // Test with no child
        container.setChild(nullptr);
        mockChild.reset();
        
        consumed = container.onWheel(testPos, testAngle);
        QVERIFY(!consumed);
        QVERIFY(!mockChild.m_wheelCalled);
    }
    
    void testUiPanelWheelForwarding()
    {
        qDebug() << "Testing UiPanel wheel forwarding...";
        
        UiPanel panel;
        MockWheelComponent mockChild1, mockChild2;
        
        panel.setViewportRect(QRect(0, 0, 200, 200));
        panel.addChild(&mockChild1);
        panel.addChild(&mockChild2);
        
        // Test wheel event forwarding (should go to last child first - reverse Z-order)
        QPoint testPos(50, 50);
        QPoint testAngle(0, 120);
        
        bool consumed = panel.onWheel(testPos, testAngle);
        
        QVERIFY(consumed);
        // mockChild2 was added last, so it should receive the event first
        QVERIFY(mockChild2.m_wheelCalled);
        QVERIFY(!mockChild1.m_wheelCalled); // Should not reach first child since second consumes
        
        // Test outside viewport
        mockChild1.reset();
        mockChild2.reset();
        
        consumed = panel.onWheel(QPoint(300, 300), testAngle);
        QVERIFY(!consumed);
        QVERIFY(!mockChild1.m_wheelCalled);
        QVERIFY(!mockChild2.m_wheelCalled);
    }
    
    void testUiGridWheelForwarding()
    {
        qDebug() << "Testing UiGrid wheel forwarding...";
        
        UiGrid grid;
        MockWheelComponent mockChild1, mockChild2;
        
        grid.setViewportRect(QRect(0, 0, 200, 200));
        grid.addChild(&mockChild1, 0, 0);
        grid.addChild(&mockChild2, 0, 1);
        
        // Test wheel event forwarding
        QPoint testPos(50, 50);
        QPoint testAngle(0, 120);
        
        bool consumed = grid.onWheel(testPos, testAngle);
        
        QVERIFY(consumed);
        // Should follow reverse Z-order (last added first)
        QVERIFY(mockChild2.m_wheelCalled);
        
        // Test outside viewport
        mockChild1.reset();
        mockChild2.reset();
        
        consumed = grid.onWheel(QPoint(300, 300), testAngle);
        QVERIFY(!consumed);
        QVERIFY(!mockChild1.m_wheelCalled);
        QVERIFY(!mockChild2.m_wheelCalled);
    }
    
    void testUiTabViewWheelForwarding()
    {
        qDebug() << "Testing UiTabView wheel forwarding...";
        
        UiTabView tabView;
        MockTabViewModel vm;
        MockWheelComponent mockContent1, mockContent2;
        
        tabView.setViewportRect(QRect(0, 0, 300, 200));
        tabView.setViewModel(&vm);
        tabView.setContent(0, &mockContent1);
        tabView.setContent(1, &mockContent2);
        
        // Test wheel event in content area (should forward to selected tab)
        // Content area is below the tab bar, so we use a point that would be in content
        QPoint contentPos(150, 150); // Well within content area
        QPoint testAngle(0, 120);
        
        // First tab is selected, so mockContent1 should receive the event
        bool consumed = tabView.onWheel(contentPos, testAngle);
        QVERIFY(consumed);
        QVERIFY(mockContent1.m_wheelCalled);
        QVERIFY(!mockContent2.m_wheelCalled);
        
        // Test outside viewport
        mockContent1.reset();
        mockContent2.reset();
        
        consumed = tabView.onWheel(QPoint(400, 400), testAngle);
        QVERIFY(!consumed);
        QVERIFY(!mockContent1.m_wheelCalled);
        QVERIFY(!mockContent2.m_wheelCalled);
    }
    
    void testRebuildHostWheelForwarding()
    {
        qDebug() << "Testing RebuildHost wheel forwarding...";
        
        UI::RebuildHost host;
        MockWheelComponent mockChild;
        
        // Manually set the child (simulating the rebuild process)
        host.setBuilder([&mockChild]() -> std::unique_ptr<IUiComponent> {
            // Return a mock component that forwards to our test mock
            return std::make_unique<MockWheelComponent>();
        });
        host.requestRebuild();
        
        QPoint testPos(50, 50);
        QPoint testAngle(0, 120);
        
        bool consumed = host.onWheel(testPos, testAngle);
        
        // The RebuildHost should forward to its child
        QVERIFY(consumed);
        
        // Test with no child
        host.setBuilder(nullptr);
        consumed = host.onWheel(testPos, testAngle);
        QVERIFY(!consumed);
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    WheelForwardingTestRunner runner;
    QMetaObject::invokeMethod(&runner, "runWheelForwardingTests", Qt::QueuedConnection);
    
    return app.exec();
}

#include "test_wheel_forwarding.moc"