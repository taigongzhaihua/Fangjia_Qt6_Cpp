#include <QtTest>
#include <QSignalSpy>
#include <QObject>
#include <vector>
#include <string>
#include <algorithm>
#include "framework/declarative/RebuildHost.h"
#include "framework/declarative/Binding.h"
#include "framework/base/UiComponent.hpp"
#include "framework/base/ILayoutable.hpp"
#include "core/rendering/RenderData.hpp"

// Test ViewModel for binding tests
class DummyViewModel : public QObject
{
    Q_OBJECT
public:
    explicit DummyViewModel(QObject* parent = nullptr) : QObject(parent) {}
    
    int value() const { return m_value; }
    void setValue(int v) {
        if (m_value != v) {
            m_value = v;
            emit valueChanged();
        }
    }

signals:
    void valueChanged();

private:
    int m_value = 0;
};

// Simple test component that counts rebuilds
class TestComponent : public IUiComponent
{
public:
    TestComponent() = default;
    
    int buildCount() const { return m_buildCount; }
    
    // IUiComponent interface
    void updateLayout(const QSize&) override { ++m_buildCount; }
    void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
    void append(Render::FrameData&) const override {}
    bool onMousePress(const QPoint&) override { return false; }
    bool onMouseMove(const QPoint&) override { return false; }
    bool onMouseRelease(const QPoint&) override { return false; }
    bool tick() override { return false; }
    QRect bounds() const override { return QRect(); }
    void onThemeChanged(bool) override {}

private:
    mutable int m_buildCount = 0;
};

class TestRebuildHost : public QObject
{
    Q_OBJECT

private slots:
    void testBasicConstruction()
    {
        UI::RebuildHost host;
        
        // Test that default construction works
        QVERIFY(true); // Just verify it doesn't crash
    }

    void testBuilderSetup()
    {
        UI::RebuildHost host;
        int buildCallCount = 0;
        
        // Set up builder that creates a test component and counts calls
        host.setBuilder([&buildCallCount]() -> std::unique_ptr<IUiComponent> {
            buildCallCount++;
            return std::make_unique<TestComponent>();
        });
        
        // With new default behavior - builder is called immediately
        QCOMPARE(buildCallCount, 1);
        
        // Request rebuild should call builder again
        host.requestRebuild();
        QCOMPARE(buildCallCount, 2);
        
        // Another rebuild should call builder again
        host.requestRebuild();
        QCOMPARE(buildCallCount, 3);
    }

    void testRebuildRequest()
    {
        UI::RebuildHost host;
        auto testComponent = std::make_unique<TestComponent>();
        TestComponent* componentPtr = testComponent.get();
        
        bool builderCalled = false;
        host.setBuilder([&builderCalled, &testComponent]() -> std::unique_ptr<IUiComponent> {
            builderCalled = true;
            return std::move(testComponent);
        });
        
        // With new default behavior, builder is called immediately
        QVERIFY(builderCalled);
        
        // Test that the component was properly set up
        // Since we moved the component, we can't access it directly anymore
        // But we can verify the builder was called
    }

    void testBuildImmediatelyParameter()
    {
        UI::RebuildHost host;
        int buildCallCount = 0;
        
        // Test buildImmediately = false
        host.setBuilder([&buildCallCount]() -> std::unique_ptr<IUiComponent> {
            buildCallCount++;
            return std::make_unique<TestComponent>();
        }, false); // Don't build immediately
        
        // Builder should not be called yet with buildImmediately = false
        QCOMPARE(buildCallCount, 0);
        
        // Manual requestRebuild should work
        host.requestRebuild();
        QCOMPARE(buildCallCount, 1);
        
        // Test default behavior (buildImmediately = true)
        UI::RebuildHost host2;
        int buildCallCount2 = 0;
        
        host2.setBuilder([&buildCallCount2]() -> std::unique_ptr<IUiComponent> {
            buildCallCount2++;
            return std::make_unique<TestComponent>();
        }); // Default buildImmediately = true
        
        // Builder should be called immediately with default behavior
        QCOMPARE(buildCallCount2, 1);
    }

    void testViewModelSignalBinding()
    {
        UI::RebuildHost host;
        DummyViewModel viewModel;
        int rebuildCount = 0;
        
        // Set up builder
        host.setBuilder([&rebuildCount]() -> std::unique_ptr<IUiComponent> {
            rebuildCount++;
            return std::make_unique<TestComponent>();
        });
        
        // Connect ViewModel signal to requestRebuild using observe
        UI::observe(&viewModel, &DummyViewModel::valueChanged, 
                   [&host]() { host.requestRebuild(); });
        
        // Initial state
        QCOMPARE(rebuildCount, 0);
        
        // Change ViewModel value should trigger rebuild
        viewModel.setValue(42);
        QCOMPARE(rebuildCount, 1);
        
        // Another change should trigger another rebuild
        viewModel.setValue(100);
        QCOMPARE(rebuildCount, 2);
        
        // Setting same value should not trigger rebuild (if ViewModel is implemented correctly)
        viewModel.setValue(100);
        QCOMPARE(rebuildCount, 2); // Should still be 2
    }

    void testMultipleSignalConnections()
    {
        UI::RebuildHost host;
        DummyViewModel viewModel1;
        DummyViewModel viewModel2;
        int rebuildCount = 0;
        
        // Set up builder
        host.setBuilder([&rebuildCount]() -> std::unique_ptr<IUiComponent> {
            rebuildCount++;
            return std::make_unique<TestComponent>();
        });
        
        // Connect multiple ViewModels to the same host
        UI::observe(&viewModel1, &DummyViewModel::valueChanged, 
                   [&host]() { host.requestRebuild(); });
        UI::observe(&viewModel2, &DummyViewModel::valueChanged, 
                   [&host]() { host.requestRebuild(); });
        
        // Changes to either ViewModel should trigger rebuilds
        viewModel1.setValue(1);
        QCOMPARE(rebuildCount, 1);
        
        viewModel2.setValue(2);
        QCOMPARE(rebuildCount, 2);
        
        viewModel1.setValue(3);
        QCOMPARE(rebuildCount, 3);
    }

    void testBoundsAndEvents()
    {
        UI::RebuildHost host;
        
        // Set up simple builder
        host.setBuilder([]() -> std::unique_ptr<IUiComponent> {
            return std::make_unique<TestComponent>();
        });
        
        // Build once
        host.requestRebuild();
        
        // Test bounds (should return viewport or delegate to child)
        QRect bounds = host.bounds();
        // Should not crash and return a valid rect
        
        // Test event handling (should not crash)
        bool mouseResult = host.onMousePress(QPoint(10, 10));
        QVERIFY(mouseResult == false); // TestComponent returns false
        
        mouseResult = host.onMouseMove(QPoint(15, 15));
        QVERIFY(mouseResult == false);
        
        mouseResult = host.onMouseRelease(QPoint(20, 20));
        QVERIFY(mouseResult == false);
        
        // Test tick
        bool tickResult = host.tick();
        QVERIFY(tickResult == false); // TestComponent returns false
    }

    void testEnvironmentContextPassing()
    {
        UI::RebuildHost host;
        bool builderCalled = false;
        
        host.setBuilder([&builderCalled]() -> std::unique_ptr<IUiComponent> {
            builderCalled = true;
            return std::make_unique<TestComponent>();
        });
        
        // Set viewport before building
        host.setViewportRect(QRect(0, 0, 800, 600));
        
        // Set layout size
        host.updateLayout(QSize(1024, 768));
        
        // Set theme
        host.onThemeChanged(true); // dark theme
        
        // Now rebuild - the child should receive all the context
        host.requestRebuild();
        QVERIFY(builderCalled);
        
        // The test verifies that no crashes occur when context is passed to child
        // In a real implementation, we would verify the child received the context
    }

    void testBuilderCalledOnlyWhenSet()
    {
        UI::RebuildHost host;
        
        // Requesting rebuild without builder should not crash
        host.requestRebuild();
        
        // No verification needed - just ensure it doesn't crash
        QVERIFY(true);
    }

    void testILayoutableInterface()
    {
        UI::RebuildHost host;
        
        // Test that RebuildHost implements ILayoutable
        ILayoutable* layoutable = dynamic_cast<ILayoutable*>(&host);
        QVERIFY(layoutable != nullptr);
        
        // Test measure with empty host
        SizeConstraints cs;
        cs.minW = 10; cs.minH = 20;
        cs.maxW = 500; cs.maxH = 400;
        
        QSize measured = host.measure(cs);
        
        // Empty host should return size within constraints
        QVERIFY(measured.width() >= cs.minW && measured.width() <= cs.maxW);
        QVERIFY(measured.height() >= cs.minH && measured.height() <= cs.maxH);
        
        // Test arrange method doesn't crash
        QRect finalRect(0, 0, 200, 100);
        host.arrange(finalRect);
        
        // After arrange, viewport should be set
        QRect bounds = host.bounds();
        QCOMPARE(bounds, finalRect);
        
        // Test with a built child component
        bool builderCalled = false;
        host.setBuilder([&builderCalled]() -> std::unique_ptr<IUiComponent> {
            builderCalled = true;
            return std::make_unique<TestComponent>();
        });
        
        QVERIFY(builderCalled);
        
        // Measure should now delegate to child
        QSize measured2 = host.measure(cs);
        // We can't verify exact size since TestComponent returns empty bounds,
        // but we can verify it doesn't crash and returns valid size
        QVERIFY(measured2.width() >= 0 && measured2.height() >= 0);
    }

    void testBoundsPreferViewport()
    {
        UI::RebuildHost host;
        
        // Create a component with specific bounds
        class ComponentWithBounds : public TestComponent {
        public:
            QRect bounds() const override { return QRect(5, 5, 50, 50); }
        };
        
        // Set builder to create component with specific bounds
        host.setBuilder([]() -> std::unique_ptr<IUiComponent> {
            return std::make_unique<ComponentWithBounds>();
        });
        
        // Before setting viewport, bounds should come from child
        QRect childBounds = host.bounds();
        QCOMPARE(childBounds, QRect(5, 5, 50, 50));
        
        // Set viewport via setViewportRect
        QRect viewport(10, 10, 800, 600);
        host.setViewportRect(viewport);
        
        // Now bounds should prefer the viewport
        QRect boundsAfterViewport = host.bounds();
        QCOMPARE(boundsAfterViewport, viewport);
        
        // Test arrange also sets viewport and is preferred
        QRect arrangeRect(20, 20, 1024, 768);
        host.arrange(arrangeRect);
        
        QRect boundsAfterArrange = host.bounds();
        QCOMPARE(boundsAfterArrange, arrangeRect);
    }

    void testThemeOrderingFix()
    {
        qDebug() << "=== Testing Theme Ordering Fix ===";
        
        // Mock component that tracks call order to verify theme fix
        class ThemeOrderTrackingComponent : public IUiComponent {
        public:
            mutable std::vector<std::string> callOrder;
            bool m_isDark = false; // Default theme state
            bool m_themeCorrectDuringResourceUpdate = false;
            
            void onThemeChanged(bool isDark) override {
                callOrder.push_back("onThemeChanged");
                m_isDark = isDark;
                qDebug() << "ThemeOrderTrackingComponent::onThemeChanged(" << (isDark ? "dark" : "light") << ")";
            }
            
            void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float dpr) override {
                callOrder.push_back("updateResourceContext");
                // Record whether theme was correct during resource update
                // For light theme test, m_isDark should be false here
                m_themeCorrectDuringResourceUpdate = !m_isDark; // Expecting light theme
                qDebug() << "ThemeOrderTrackingComponent::updateResourceContext() - isDark=" << m_isDark 
                         << " (correct=" << (m_themeCorrectDuringResourceUpdate ? "true" : "false") << ")";
            }
            
            void updateLayout(const QSize&) override {
                callOrder.push_back("updateLayout");
                qDebug() << "ThemeOrderTrackingComponent::updateLayout()";
            }
            
            bool onMousePress(const QPoint&) override { return false; }
            bool onMouseMove(const QPoint&) override { return false; }
            bool onMouseRelease(const QPoint&) override { return false; }
            bool tick() override { return false; }
            QRect bounds() const override { return QRect(0, 0, 100, 50); }
            
            bool wasThemeCorrectDuringResourceUpdate() const { 
                return m_themeCorrectDuringResourceUpdate; 
            }
        };
        
        UI::RebuildHost host;
        IconCache cache;
        QOpenGLFunctions gl;
        
        // Set up host context (simulating light theme)
        host.setViewportRect(QRect(0, 0, 800, 600));
        host.updateResourceContext(cache, &gl, 1.0f);
        host.updateLayout(QSize(800, 600));
        host.onThemeChanged(false); // Light theme
        
        // Set builder to create theme-tracking component
        ThemeOrderTrackingComponent* componentPtr = nullptr;
        host.setBuilder([&componentPtr]() -> std::unique_ptr<IUiComponent> {
            auto component = std::make_unique<ThemeOrderTrackingComponent>();
            componentPtr = component.get();
            return std::move(component);
        });
        
        // Clear any initial calls from setBuilder
        if (componentPtr) {
            componentPtr->callOrder.clear();
        }
        
        // Trigger rebuild (simulating nav interaction that causes theme flicker)
        qDebug() << "Triggering requestRebuild()...";
        host.requestRebuild();
        
        // Verify the fix worked
        QVERIFY(componentPtr != nullptr);
        
        // Verify call order: onThemeChanged should come before updateResourceContext
        const auto& callOrder = componentPtr->callOrder;
        QVERIFY(!callOrder.empty());
        
        qDebug() << "Call order:";
        for (size_t i = 0; i < callOrder.size(); ++i) {
            qDebug() << "  " << (i+1) << ". " << callOrder[i].c_str();
        }
        
        // Find positions of key calls
        auto themePos = std::find(callOrder.begin(), callOrder.end(), "onThemeChanged");
        auto resourcePos = std::find(callOrder.begin(), callOrder.end(), "updateResourceContext");
        
        // Both calls should be present
        QVERIFY(themePos != callOrder.end());
        QVERIFY(resourcePos != callOrder.end());
        
        // onThemeChanged should come before updateResourceContext (the fix)
        bool themeBeforeResource = (themePos < resourcePos);
        QVERIFY(themeBeforeResource);
        
        // Verify component had correct theme during resource update
        QVERIFY(componentPtr->wasThemeCorrectDuringResourceUpdate());
        
        qDebug() << "✅ Theme ordering fix verified - onThemeChanged called before updateResourceContext";
        qDebug() << "✅ Component had correct theme state during resource context update";
    }
};

#include "TestRebuildHost.moc"
// Don't use QTEST_MAIN here - will be included in test_main.cpp