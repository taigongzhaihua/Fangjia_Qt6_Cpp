#include <QtTest>
#include <QDebug>
#include "presentation/ui/containers/UiRoot.h"
#include "presentation/ui/base/UiComponent.hpp"
#include "presentation/ui/base/UiContent.hpp" 
#include "presentation/ui/base/ILayoutable.hpp"

/**
 * This test validates the fix for the UiRoot layout ordering issue.
 * 
 * Problem: UiRoot::updateLayout previously called child->updateLayout() BEFORE 
 * setting viewport, causing declarative containers to compute layout with invalid viewport.
 * 
 * Fix: Reordered to set viewport and arrange BEFORE calling updateLayout(), ensuring
 * containers have valid viewport during layout computation.
 */
class TestUiRootViewportOrdering : public QObject
{
    Q_OBJECT

private slots:
    void testViewportOrderingFix()
    {
        qDebug() << "Testing UiRoot viewport ordering fix...";
        
        // Mock declarative container that depends on viewport during updateLayout
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
        
        UiRoot root;
        MockDeclarativeContainer container;
        
        // Add container to root
        root.add(&container);
        
        // Call updateLayout - this should now call viewport/arrange BEFORE updateLayout
        QSize windowSize(800, 600);
        root.updateLayout(windowSize);
        
        // Verify the fix worked
        QVERIFY(container.m_updateLayoutCalled);
        QVERIFY(container.m_viewportSetBeforeUpdate); // Viewport should be set before updateLayout
        QVERIFY(container.m_arrangeCalledBeforeUpdate); // Arrange should be called before updateLayout
        
        // Verify correct viewport was set
        QCOMPARE(container.m_viewport, QRect(0, 0, 800, 600));
        QCOMPARE(container.m_arrangeRect, QRect(0, 0, 800, 600));
        
        // Verify container computed content rect correctly with valid viewport
        QCOMPARE(container.m_computedContentRect, QRect(10, 10, 780, 580));
        
        qDebug() << "UiRoot viewport ordering fix test PASSED ✅";
    }
    
    void testOrderingWithNonLayoutableComponent()
    {
        qDebug() << "Testing UiRoot with non-layoutable component...";
        
        // Simple component that doesn't implement ILayoutable or IUiContent
        class MockSimpleComponent : public IUiComponent {
        public:
            bool m_updateLayoutCalled = false;
            
            void updateLayout(const QSize&) override { m_updateLayoutCalled = true; }
            void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
            void append(Render::FrameData&) const override {}
            bool onMousePress(const QPoint&) override { return false; }
            bool onMouseMove(const QPoint&) override { return false; }
            bool onMouseRelease(const QPoint&) override { return false; }
            bool onWheel(const QPoint&, const QPoint&) override { return false; }
            bool tick() override { return false; }
            QRect bounds() const override { return QRect(0, 0, 100, 50); }
            void onThemeChanged(bool) override {}
        };
        
        UiRoot root;
        MockSimpleComponent simpleComponent;
        
        root.add(&simpleComponent);
        root.updateLayout(QSize(800, 600));
        
        // Simple components should still have updateLayout called
        QVERIFY(simpleComponent.m_updateLayoutCalled);
        
        qDebug() << "Non-layoutable component test PASSED ✅";
    }
};

// Run the test
QTEST_MAIN(TestUiRootViewportOrdering)
#include "test_ui_root_viewport_ordering.moc"