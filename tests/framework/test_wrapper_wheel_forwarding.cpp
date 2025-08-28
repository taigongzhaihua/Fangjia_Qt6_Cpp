#include <QtTest>
#include <QCoreApplication>
#include <QDebug>
#include "UiComponent.hpp"
#include "ComponentWrapper.h"
#include "Decorators.h"

// Mock component for testing wheel event forwarding
class MockWheelComponent : public IUiComponent {
public:
    bool m_wheelCalled{false};
    QPoint m_lastWheelPos;
    QPoint m_lastWheelAngle;
    
    void reset() { 
        m_wheelCalled = false; 
        m_lastWheelPos = QPoint(); 
        m_lastWheelAngle = QPoint(); 
    }
    
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

class WrapperWheelForwardingTestRunner : public QObject
{
    Q_OBJECT

public slots:
    void runWrapperWheelForwardingTests()
    {
        qDebug() << "=== Testing ComponentWrapper and DecoratedBox Wheel Event Forwarding ===";
        
        testComponentWrapperWheelForwarding();
        testDecoratedBoxWheelForwarding();
        
        qDebug() << "All wrapper wheel forwarding tests PASSED ✅";
    }

private:
    void testComponentWrapperWheelForwarding()
    {
        qDebug() << "Testing ComponentWrapper wheel forwarding...";
        
        MockWheelComponent mockChild;
        UI::ComponentWrapper wrapper(&mockChild);
        
        auto proxyComponent = wrapper.build();
        QVERIFY(proxyComponent != nullptr);
        
        // Set viewport for the proxy component
        if (auto* content = dynamic_cast<IUiContent*>(proxyComponent.get())) {
            content->setViewportRect(QRect(0, 0, 200, 200));
        }
        
        // Test wheel event forwarding
        QPoint testPos(50, 50);
        QPoint testAngle(0, 120);
        
        bool consumed = proxyComponent->onWheel(testPos, testAngle);
        
        QVERIFY(consumed);
        QVERIFY(mockChild.m_wheelCalled);
        QCOMPARE(mockChild.m_lastWheelPos, testPos);
        QCOMPARE(mockChild.m_lastWheelAngle, testAngle);
        
        qDebug() << "ComponentWrapper wheel forwarding test PASSED ✅";
    }
    
    void testDecoratedBoxWheelForwarding()
    {
        qDebug() << "Testing DecoratedBox wheel forwarding...";
        
        auto mockChild = std::make_unique<MockWheelComponent>();
        MockWheelComponent* childPtr = mockChild.get();
        
        UI::DecoratedBox::Props props;
        props.visible = true;
        props.bg = QColor(255, 255, 255, 100);
        props.padding = QMargins(10, 10, 10, 10);
        
        UI::DecoratedBox decoratedBox(std::move(mockChild), props);
        decoratedBox.setViewportRect(QRect(0, 0, 200, 200));
        
        // Test wheel event inside viewport
        QPoint testPos(50, 50);
        QPoint testAngle(0, 120);
        
        bool consumed = decoratedBox.onWheel(testPos, testAngle);
        
        QVERIFY(consumed);
        QVERIFY(childPtr->m_wheelCalled);
        QCOMPARE(childPtr->m_lastWheelPos, testPos);
        QCOMPARE(childPtr->m_lastWheelAngle, testAngle);
        
        // Reset and test outside viewport
        childPtr->reset();
        
        consumed = decoratedBox.onWheel(QPoint(300, 300), testAngle);
        QVERIFY(!consumed);
        QVERIFY(!childPtr->m_wheelCalled);
        
        // Reset and test when not visible
        childPtr->reset();
        props.visible = false;
        UI::DecoratedBox invisibleBox(std::make_unique<MockWheelComponent>(), props);
        invisibleBox.setViewportRect(QRect(0, 0, 200, 200));
        
        consumed = invisibleBox.onWheel(testPos, testAngle);
        QVERIFY(!consumed);
        
        qDebug() << "DecoratedBox wheel forwarding test PASSED ✅";
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set test environment
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_LOGGING_RULES", "qt.qpa.gl=false");
    
    WrapperWheelForwardingTestRunner runner;
    runner.runWrapperWheelForwardingTests();
    
    return 0;
}

#include "test_wrapper_wheel_forwarding.moc"