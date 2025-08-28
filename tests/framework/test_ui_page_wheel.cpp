#include <QtTest>
#include <QCoreApplication>
#include <QDebug>
#include "UiPage.h"
#include "UiComponent.hpp"

// Mock component for testing wheel event forwarding
class MockWheelComponent : public IUiComponent {
public:
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
    QRect bounds() const override { return QRect(0, 0, 100, 100); }
    void applyTheme(bool) override {}
};

class UiPageWheelTestRunner : public QObject
{
    Q_OBJECT

public slots:
    void runPageWheelTests()
    {
        qDebug() << "=== Testing UiPage Wheel Event Forwarding ===";
        
        UiPage page;
        MockWheelComponent mockContent;
        
        // Set up page layout
        page.setViewportRect(QRect(0, 0, 200, 300));
        page.setContent(&mockContent);
        page.updateLayout(QSize(200, 300));
        
        // Get content area bounds
        QRectF contentRect = page.contentRectF();
        qDebug() << "Content rect:" << contentRect;
        
        // Test wheel event inside content area
        QPoint insidePoint(static_cast<int>(contentRect.center().x()), 
                          static_cast<int>(contentRect.center().y()));
        bool consumed = page.onWheel(insidePoint, QPoint(0, 120));
        
        QVERIFY(consumed); // Page should consume and forward the event
        QVERIFY(mockContent.m_wheelCalled); // Content should have received the event
        QCOMPARE(mockContent.m_lastWheelPos, insidePoint);
        QCOMPARE(mockContent.m_lastWheelAngle, QPoint(0, 120));
        
        // Reset mock
        mockContent.m_wheelCalled = false;
        mockContent.m_lastWheelPos = QPoint();
        mockContent.m_lastWheelAngle = QPoint();
        
        // Test wheel event outside content area (in title area)
        QPoint outsidePoint(10, 10); // Should be in title area
        consumed = page.onWheel(outsidePoint, QPoint(0, 120));
        
        QVERIFY(!consumed); // Page should not consume events outside content area
        QVERIFY(!mockContent.m_wheelCalled); // Content should not have received the event
        
        // Test with no content
        page.setContent(nullptr);
        consumed = page.onWheel(insidePoint, QPoint(0, 120));
        QVERIFY(!consumed); // Should not consume when no content
        
        qDebug() << "UiPage wheel event tests PASSED ✅";
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_LOGGING_RULES", "qt.qpa.gl=false");
    
    qDebug() << "===========================================";
    qDebug() << "UiPage Wheel Event Tests";
    qDebug() << "===========================================";
    
    UiPageWheelTestRunner runner;
    
    try {
        runner.runPageWheelTests();
        
        qDebug() << "===========================================";
        qDebug() << "ALL PAGE WHEEL TESTS PASSED ✅";
        qDebug() << "===========================================";
        
        return 0;
    } catch (...) {
        qDebug() << "===========================================";
        qDebug() << "TEST FAILURE ❌";
        qDebug() << "===========================================";
        return 1;
    }
}

#include "test_ui_page_wheel.moc"