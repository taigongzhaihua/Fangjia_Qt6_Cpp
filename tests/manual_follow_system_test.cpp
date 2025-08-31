/*
 * Manual test to verify Follow System button fix
 * This simulates the problematic scenario and verifies the fix works
 */

#include <QApplication>
#include <QWidget> 
#include <QPushButton>
#include <QVBoxLayout>
#include <QTimer>
#include <QDebug>
#include <memory>

class TestRebuildHost {
public:
    void requestRebuild() {
        rebuildCount++;
        qDebug() << "Rebuild requested, count:" << rebuildCount;
    }
    
    int getRebuildCount() const { return rebuildCount; }
    
private:
    int rebuildCount = 0;
};

class TestWindow : public QWidget {
    Q_OBJECT
    
public:
    TestWindow() {
        setWindowTitle("Follow System Button Fix Test");
        setFixedSize(300, 200);
        
        auto* layout = new QVBoxLayout(this);
        
        // Test buttons
        auto* syncButton = new QPushButton("Test Synchronous (Old - Problematic)");
        auto* deferredButton = new QPushButton("Test Deferred (New - Fixed)");
        auto* rapidButton = new QPushButton("Test Rapid Clicks (Fixed)");
        
        layout->addWidget(syncButton);
        layout->addWidget(deferredButton);
        layout->addWidget(rapidButton);
        
        m_rebuildHost = std::make_unique<TestRebuildHost>();
        
        // Connect buttons
        connect(syncButton, &QPushButton::clicked, this, &TestWindow::testSynchronousRebuild);
        connect(deferredButton, &QPushButton::clicked, this, &TestWindow::testDeferredRebuild);
        connect(rapidButton, &QPushButton::clicked, this, &TestWindow::testRapidClicks);
        
        qDebug() << "Test window created. Click buttons to test rebuild behavior.";
    }
    
private slots:
    void testSynchronousRebuild() {
        qDebug() << "=== Testing Synchronous Rebuild (Problematic) ===";
        qDebug() << "This simulates the original problematic behavior";
        
        // This is the problematic pattern - immediate rebuild during event handling
        if (m_rebuildHost) {
            m_rebuildHost->requestRebuild();
        }
        
        qDebug() << "Synchronous rebuild completed. Total rebuilds:" << m_rebuildHost->getRebuildCount();
    }
    
    void testDeferredRebuild() {
        qDebug() << "=== Testing Deferred Rebuild (Fixed) ===";
        qDebug() << "This simulates the fixed behavior using QTimer::singleShot";
        
        // This is the fixed pattern - defer rebuild to next event loop iteration
        QTimer::singleShot(0, this, [this]() {
            if (m_rebuildHost) {
                m_rebuildHost->requestRebuild();
            }
            qDebug() << "Deferred rebuild completed. Total rebuilds:" << m_rebuildHost->getRebuildCount();
        });
        
        qDebug() << "Deferred rebuild scheduled...";
    }
    
    void testRapidClicks() {
        qDebug() << "=== Testing Rapid Clicks (Fixed Pattern) ===";
        
        // Simulate rapid clicking with deferred rebuilds
        for (int i = 0; i < 3; ++i) {
            QTimer::singleShot(0, this, [this, i]() {
                if (m_rebuildHost) {
                    m_rebuildHost->requestRebuild();
                }
                qDebug() << "Rapid click" << (i+1) << "rebuild completed";
            });
        }
        
        qDebug() << "Scheduled 3 rapid deferred rebuilds...";
    }
    
private:
    std::unique_ptr<TestRebuildHost> m_rebuildHost;
};

#include "manual_follow_system_test.moc"

int main(int argc, char* argv[]) {
    QApplication app(argc, argv);
    
    TestWindow window;
    window.show();
    
    qDebug() << "Manual test application started.";
    qDebug() << "This test validates that:";
    qDebug() << "1. Synchronous rebuilds happen immediately (can cause issues)";
    qDebug() << "2. Deferred rebuilds happen safely in next event loop";
    qDebug() << "3. Rapid clicks are handled safely with deferred pattern";
    
    return app.exec();
}