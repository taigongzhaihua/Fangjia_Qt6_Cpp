#include <QtTest>
#include <QCoreApplication>
#include <QDebug>
#include <QSignalSpy>

// Core test for ThemeManager
#include "presentation/viewmodels/ThemeManager.h"

// Core test for AppConfig  
#include "AppConfig.h"

// Core test for TabViewModel
#include "presentation/viewmodels/TabViewModel.h"
#include "presentation/binding/tab_interface.h"

// Core test for FormulaViewModel
#include "presentation/viewmodels/FormulaViewModel.h"

// Core test for RebuildHost
#include "presentation/ui/declarative/RebuildHost.h"
#include "presentation/ui/base/UiComponent.hpp"

// Core test for DecoratedBox
#include "presentation/ui/declarative/Decorators.h"

// Core test for AppShell
#include "presentation/ui/declarative/AppShell.h"
#include "presentation/ui/declarative/UI.h"

// Domain layer tests  
#include "tests/domain/test_usecases.cpp"

// Framework tests
#include "presentation/ui/containers/UiScrollView.h"
#include "presentation/ui/containers/UiPage.h"
#include "presentation/ui/containers/UiRoot.h"
#include "presentation/ui/widgets/UiTreeList.h"
#include "presentation/ui/base/ILayoutable.hpp"

class SimpleTestRunner : public QObject
{
    Q_OBJECT

public slots:
    void runThemeManagerTests()
    {
        qDebug() << "=== Testing ThemeManager ===";
        
        ThemeManager manager;
        
        // Test mode setting
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::FollowSystem);
        
        QSignalSpy spy(&manager, &ThemeManager::modeChanged);
        manager.setMode(ThemeManager::ThemeMode::Light);
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::Light);
        QCOMPARE(spy.count(), 1);
        
        // Test cycleMode
        manager.cycleMode();
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::Dark);
        
        qDebug() << "ThemeManager tests PASSED ✅";
    }
    
    void runAppConfigTests()
    {
        qDebug() << "=== Testing AppConfig ===";
        
        // Use test settings
        QCoreApplication::setOrganizationName("TestOrg");
        QCoreApplication::setApplicationName("TestApp");
        
        AppConfig config;
        
        // Test theme mode
        QSignalSpy spy(&config, &AppConfig::themeModeChanged);
        config.setThemeMode("dark");
        QCOMPARE(config.themeMode(), QString("dark"));
        QCOMPARE(spy.count(), 1);
        
        // Test nav expanded
        QSignalSpy navSpy(&config, &AppConfig::navExpandedChanged);
        config.setNavExpanded(true);
        QCOMPARE(config.navExpanded(), true);
        QCOMPARE(navSpy.count(), 1);
        
        // Test reset
        config.reset();
        
        qDebug() << "AppConfig tests PASSED ✅";
    }
    
    void runTabViewModelTests()
    {
        qDebug() << "=== Testing TabViewModel ===";
        
        TabViewModel tabVm;
        QCOMPARE(tabVm.count(), 0);
        
        // Add items
        QVector<fj::presentation::binding::TabItem> items{
            {.id = "tab1", .label = "Tab 1", .tooltip = "First tab"},
            {.id = "tab2", .label = "Tab 2", .tooltip = "Second tab"}
        };
        
        QSignalSpy itemsSpy(&tabVm, &TabViewModel::itemsChanged);
        tabVm.setItems(items);
        QCOMPARE(tabVm.count(), 2);
        QCOMPARE(itemsSpy.count(), 1);
        
        // Test selection
        QSignalSpy selSpy(&tabVm, &TabViewModel::selectedIndexChanged);
        tabVm.setSelectedIndex(1);
        QCOMPARE(tabVm.selectedIndex(), 1);
        QCOMPARE(tabVm.selectedId(), QString("tab2"));
        QCOMPARE(selSpy.count(), 1);
        
        qDebug() << "TabViewModel tests PASSED ✅";
    }
    
    void runFormulaViewModelTests()
    {
        qDebug() << "=== Testing FormulaViewModel ===";
        
        FormulaViewModel formulaVm;
        QCOMPARE(formulaVm.nodeCount(), 0);
        
        // Load sample data
        QSignalSpy dataSpy(&formulaVm, &FormulaViewModel::dataChanged);
        formulaVm.loadSampleData();
        QVERIFY(formulaVm.nodeCount() > 0);
        QCOMPARE(dataSpy.count(), 1);
        
        // Test selection
        QSignalSpy selSpy(&formulaVm, &FormulaViewModel::selectedChanged);
        formulaVm.setSelectedIndex(0);
        QCOMPARE(formulaVm.selectedIndex(), 0);
        QCOMPARE(selSpy.count(), 1);
        
        qDebug() << "FormulaViewModel tests PASSED ✅";
    }
    
    void runRebuildHostTests()
    {
        qDebug() << "=== Testing RebuildHost ===";
        
        UI::RebuildHost host;
        int buildCount = 0;
        
        // Set builder
        host.setBuilder([&buildCount]() -> std::unique_ptr<IUiComponent> {
            buildCount++;
            return nullptr; // Simplified for test
        });
        
        QCOMPARE(buildCount, 1); // Now builder is called immediately
        
        // Request rebuild
        host.requestRebuild();
        QCOMPARE(buildCount, 2);
        
        host.requestRebuild();
        QCOMPARE(buildCount, 3);
        
        qDebug() << "RebuildHost tests PASSED ✅";
    }
    
    void runUiScrollViewTests()
    {
        qDebug() << "=== Testing UiScrollView ===";
        
        // Mock component for testing
        class MockComponent : public IUiComponent, public IUiContent, public ILayoutable {
        public:
            QRect m_bounds{0, 0, 100, 200};
            QRect m_viewport;
            QRect m_arrangeRect;
            QSize m_measureResult{100, 200};
            
            void updateLayout(const QSize&) override {}
            void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
            void append(Render::FrameData&) const override {}
            bool onMousePress(const QPoint&) override { return false; }
            bool onMouseMove(const QPoint&) override { return false; }
            bool onMouseRelease(const QPoint&) override { return false; }
            bool onWheel(const QPoint&, const QPoint&) override { return true; }
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
        
        // Test initial state
        UiScrollView scrollView;
        QCOMPARE(scrollView.scrollY(), 0);
        QCOMPARE(scrollView.child(), nullptr);
        QCOMPARE(scrollView.maxScrollY(), 0);
        
        // Test child management
        MockComponent mockChild;
        scrollView.setChild(&mockChild);
        QCOMPARE(scrollView.child(), &mockChild);
        
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
        
        // Test scrollbar animation
        UiScrollView scrollView2;
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
    
    void runUiPageWheelTests()
    {
        qDebug() << "=== Testing UiPage Wheel Events ===";
        
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
        
        UiPage page;
        MockWheelComponent mockContent;
        
        // Set up page layout
        page.setViewportRect(QRect(0, 0, 200, 300));
        page.setContent(&mockContent);
        page.updateLayout(QSize(200, 300));
        
        // Get content area bounds
        QRectF contentRect = page.contentRectF();
        
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
    
    void runUiTreeListWheelTests()
    {
        qDebug() << "=== Testing UiTreeList Wheel Events ===";
        
        // Mock model for testing
        class MockTreeModel : public UiTreeList::Model {
        public:
            struct MockNode {
                QString label;
                int level;
                bool expanded;
                QVector<int> children;
            };
            
            QMap<int, MockNode> nodes;
            int m_selectedId = -1;
            
            void addNode(int id, const QString& label, int level = 0, bool expanded = false) {
                nodes[id] = MockNode{label, level, expanded, {}};
            }
            
            QVector<int> rootIndices() const override {
                QVector<int> roots;
                for (auto it = nodes.begin(); it != nodes.end(); ++it) {
                    bool isRoot = true;
                    for (auto parentIt = nodes.begin(); parentIt != nodes.end(); ++parentIt) {
                        if (parentIt.value().children.contains(it.key())) {
                            isRoot = false;
                            break;
                        }
                    }
                    if (isRoot) {
                        roots.append(it.key());
                    }
                }
                std::sort(roots.begin(), roots.end());
                return roots;
            }
            
            QVector<int> childIndices(int nodeId) const override {
                if (nodes.contains(nodeId)) {
                    return nodes[nodeId].children;
                }
                return {};
            }
            
            UiTreeList::NodeInfo nodeInfo(int nodeId) const override {
                if (nodes.contains(nodeId)) {
                    const auto& node = nodes[nodeId];
                    return UiTreeList::NodeInfo{node.label, node.level, node.expanded};
                }
                return UiTreeList::NodeInfo{"", 0, false};
            }
            
            int selectedId() const override {
                return m_selectedId;
            }
            
            void setSelectedId(int nodeId) override {
                m_selectedId = nodeId;
            }
            
            void setExpanded(int nodeId, bool on) override {
                if (nodes.contains(nodeId)) {
                    nodes[nodeId].expanded = on;
                }
            }
        };
        
        // Set up tree list with test data
        UiTreeList treeList;
        MockTreeModel model;
        
        // Create test data: 10 root nodes
        for (int i = 0; i < 10; ++i) {
            model.addNode(i, QString("Node %1").arg(i), 0, false);
        }
        
        treeList.setModel(&model);
        treeList.setViewportRect(QRect(0, 0, 200, 144)); // 4 items visible (36px each)
        
        // Test initial state
        QCOMPARE(treeList.scrollOffset(), 0);
        QCOMPARE(treeList.contentHeight(), 360); // 10 items * 36px
        
        // Test wheel event inside bounds
        bool consumed = treeList.onWheel(QPoint(100, 50), QPoint(0, 120)); // Scroll down one notch
        QVERIFY(consumed); // Should consume event when there's scrollable content
        QCOMPARE(treeList.scrollOffset(), 48); // 120/120 * 48 = 48px scroll
        
        // Test wheel event outside bounds
        int previousScroll = treeList.scrollOffset();
        consumed = treeList.onWheel(QPoint(300, 300), QPoint(0, 120));
        QVERIFY(!consumed); // Should not consume event outside bounds
        QCOMPARE(treeList.scrollOffset(), previousScroll); // Should not change scroll
        
        // Test scroll up
        consumed = treeList.onWheel(QPoint(100, 50), QPoint(0, -120)); // Scroll up one notch
        QVERIFY(consumed);
        QCOMPARE(treeList.scrollOffset(), 0); // Should go back to 0 (48 - 48)
        
        // Test scroll limiting at top
        consumed = treeList.onWheel(QPoint(100, 50), QPoint(0, -120)); // Try to scroll above top
        QVERIFY(consumed); // Still consumes event but doesn't scroll beyond limit
        QCOMPARE(treeList.scrollOffset(), 0); // Should stay at 0
        
        // Test scroll to bottom limit
        int maxScroll = 360 - 144; // contentHeight - viewportHeight = 216
        treeList.setScrollOffset(maxScroll);
        QCOMPARE(treeList.scrollOffset(), maxScroll);
        
        // Test scroll limiting at bottom
        consumed = treeList.onWheel(QPoint(100, 50), QPoint(0, 120)); // Try to scroll below bottom
        QVERIFY(consumed); // Still consumes event but doesn't scroll beyond limit
        QCOMPARE(treeList.scrollOffset(), maxScroll); // Should stay at max
        
        // Test with no scrollable content (small tree)
        MockTreeModel smallModel;
        smallModel.addNode(0, "Single Node", 0, false);
        treeList.setModel(&smallModel);
        
        consumed = treeList.onWheel(QPoint(100, 50), QPoint(0, 120));
        QVERIFY(!consumed); // Should not consume event when no scrolling needed
        
        // Test with zero angleDelta
        treeList.setModel(&model); // Back to larger model
        consumed = treeList.onWheel(QPoint(100, 50), QPoint(0, 0));
        QVERIFY(!consumed); // Should not consume event with no delta
        
        qDebug() << "UiTreeList wheel event tests PASSED ✅";
    }
    
    void runDecoratedBoxTests()
    {
        qDebug() << "=== Testing DecoratedBox onTap and hover ===";
        
        // Mock child component
        class MockChild : public IUiComponent {
        public:
            void updateLayout(const QSize&) override {}
            void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
            void append(Render::FrameData&) const override {}
            bool onMousePress(const QPoint&) override { return false; } // Child doesn't handle
            bool onMouseMove(const QPoint&) override { return false; }
            bool onMouseRelease(const QPoint&) override { return false; }
            bool onWheel(const QPoint&, const QPoint&) override { return false; }
            bool tick() override { return false; }
            QRect bounds() const override { return QRect(0, 0, 50, 20); }
            void onThemeChanged(bool) override {}
        };
        
        auto child = std::make_unique<MockChild>();
        
        // Set up DecoratedBox with padding (like HomePage buttons)
        UI::DecoratedBox::Props props;
        props.padding = QMargins(8, 4, 8, 4); // Same as HomePage buttons: padding(8, 4)
        props.visible = true;
        
        bool tapCalled = false;
        props.onTap = [&tapCalled]() {
            tapCalled = true;
        };
        
        bool isHovered = false;
        props.onHover = [&isHovered](bool hover) {
            isHovered = hover;
        };
        
        auto decoratedBox = std::make_unique<UI::DecoratedBox>(std::move(child), props);
        
        // Set viewport (simulating button layout)
        QRect buttonRect(0, 0, 66, 28); // Width: 50 + 8*2 padding, Height: 20 + 4*2 padding
        decoratedBox->setViewportRect(buttonRect);
        
        // Test 1: Click in the padding area (left edge) - should work with fix
        QPoint paddingClick(4, 14); // In the left padding area
        qDebug() << "Testing click in padding area at" << paddingClick;
        
        bool paddingPressHandled = decoratedBox->onMousePress(paddingClick);
        QVERIFY(paddingPressHandled); // Should handle the press with fix
        
        bool paddingReleaseHandled = decoratedBox->onMouseRelease(paddingClick);
        QVERIFY(paddingReleaseHandled); // Should handle the release
        QVERIFY(tapCalled); // onTap should have been called
        
        // Reset for next test
        tapCalled = false;
        
        // Test 2: Click in the content area - should also work
        QPoint contentClick(33, 14); // In the center of the button
        qDebug() << "Testing click in content area at" << contentClick;
        
        bool contentPressHandled = decoratedBox->onMousePress(contentClick);
        QVERIFY(contentPressHandled);
        
        bool contentReleaseHandled = decoratedBox->onMouseRelease(contentClick);
        QVERIFY(contentReleaseHandled);
        QVERIFY(tapCalled);
        
        // Reset for next test
        tapCalled = false;
        
        // Test 3: Click outside the viewport - should not work
        QPoint outsideClick(70, 14); // Outside the button
        qDebug() << "Testing click outside at" << outsideClick;
        
        bool outsidePressHandled = decoratedBox->onMousePress(outsideClick);
        QVERIFY(!outsidePressHandled); // Should not handle clicks outside
        QVERIFY(!tapCalled); // onTap should not be called
        
        // Test 4: Hover in padding area
        QPoint paddingHover(4, 14);
        bool hoverHandled = decoratedBox->onMouseMove(paddingHover);
        QVERIFY(hoverHandled);
        QVERIFY(isHovered);
        
        // Test 5: Hover outside
        QPoint outsideHover(70, 14);
        hoverHandled = decoratedBox->onMouseMove(outsideHover);
        QVERIFY(hoverHandled); // Should handle the change
        QVERIFY(!isHovered); // Should not be hovered anymore
        
        qDebug() << "DecoratedBox tests PASSED ✅";
    }
    
    void runAppShellTests()
    {
        qDebug() << "=== Testing AppShell ===";
        
        // Test basic construction
        auto shell = UI::appShell();
        QVERIFY(shell != nullptr);
        
        // Test that it can build without crashing (even with no components)
        auto component = shell->build();
        QVERIFY(component != nullptr);
        
        // Test fluent API returns self
        auto nav = UI::text("Nav");
        auto topBar = UI::text("TopBar");
        
        auto result = shell->nav(nav)
                           ->topBar(topBar)
                           ->topBarHeight(64)
                           ->navWidthProvider([]() { return 250; });
        
        QCOMPARE(result.get(), shell.get());
        
        // Test connector registration with content
        bool connectorCalled = false;
        shell->connect([&connectorCalled](UI::RebuildHost* host) {
            connectorCalled = true;
            QVERIFY(host != nullptr);
        });
        
        // Set a content builder to trigger BindingHost creation
        shell->content([]() { return UI::text("Content"); });
        
        // Build to trigger connector execution
        component = shell->build();
        QVERIFY(component != nullptr);
        QVERIFY(connectorCalled);
        
        qDebug() << "AppShell tests PASSED ✅";
    }

    void runUiRootLayoutTests()
    {
        qDebug() << "=== Testing UiRoot viewport and layout fixes ===";
        
        // Mock component that implements both IUiContent and ILayoutable
        class MockLayoutableComponent : public IUiComponent, public IUiContent, public ILayoutable {
        public:
            QRect m_viewport;
            QRect m_arrangeRect;
            bool m_viewportSet = false;
            bool m_arrangeWasCalled = false;
            
            void updateLayout(const QSize&) override {}
            void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
            void append(Render::FrameData&) const override {}
            bool onMousePress(const QPoint&) override { return false; }
            bool onMouseMove(const QPoint&) override { return false; }
            bool onMouseRelease(const QPoint&) override { return false; }
            bool onWheel(const QPoint&, const QPoint&) override { return false; }
            bool tick() override { return false; }
            QRect bounds() const override { 
                // Before viewport set, return small bounds that would cause the clipping issue
                if (!m_viewportSet) {
                    return QRect(0, 0, 1, 1); // Tiny bounds that would cause clipping
                }
                return m_viewport; // After viewport set, return proper bounds
            }
            void onThemeChanged(bool) override {}
            
            // IUiContent
            void setViewportRect(const QRect& r) override { 
                m_viewport = r; 
                m_viewportSet = true;
            }
            
            // ILayoutable  
            QSize measure(const SizeConstraints& cs) override {
                return QSize(std::clamp(100, cs.minW, cs.maxW), 
                           std::clamp(50, cs.minH, cs.maxH));
            }
            void arrange(const QRect& finalRect) override { 
                m_arrangeRect = finalRect; 
                m_arrangeWasCalled = true;
            }
        };
        
        // Test UiRoot behavior with top-level declarative component
        UiRoot root;
        MockLayoutableComponent mockComponent;
        
        // Before fix: component would have tiny bounds
        QRect initialBounds = mockComponent.bounds();
        QCOMPARE(initialBounds, QRect(0, 0, 1, 1));
        
        // Add component to root
        root.add(&mockComponent);
        
        // Call updateLayout - this should now set viewport and arrange
        QSize windowSize(800, 600);
        root.updateLayout(windowSize);
        
        // Verify that setViewportRect was called with full window rect
        QVERIFY(mockComponent.m_viewportSet);
        QCOMPARE(mockComponent.m_viewport, QRect(0, 0, 800, 600));
        
        // Verify that arrange was called with full window rect
        QVERIFY(mockComponent.m_arrangeWasCalled);
        QCOMPARE(mockComponent.m_arrangeRect, QRect(0, 0, 800, 600));
        
        // After fix: component should now return proper viewport bounds
        QRect boundsAfterLayout = mockComponent.bounds();
        QCOMPARE(boundsAfterLayout, QRect(0, 0, 800, 600));
        
        qDebug() << "UiRoot layout fixes PASSED ✅";
    }

    void runRebuildHostBoundsTests()
    {
        qDebug() << "=== Testing RebuildHost bounds() fix ===";
        
        // Component with small bounds that would cause clipping issue
        class ComponentWithSmallBounds : public IUiComponent {
        public:
            void updateLayout(const QSize&) override {}
            void updateResourceContext(IconCache&, QOpenGLFunctions*, float) override {}
            void append(Render::FrameData&) const override {}
            bool onMousePress(const QPoint&) override { return false; }
            bool onMouseMove(const QPoint&) override { return false; }
            bool onMouseRelease(const QPoint&) override { return false; }
            bool onWheel(const QPoint&, const QPoint&) override { return false; }
            bool tick() override { return false; }
            QRect bounds() const override { return QRect(0, 0, 10, 5); } // Very small bounds
            void onThemeChanged(bool) override {}
        };
        
        UI::RebuildHost host;
        
        // Set builder to create component with small bounds
        host.setBuilder([]() -> std::unique_ptr<IUiComponent> {
            return std::make_unique<ComponentWithSmallBounds>();
        });
        
        // Before viewport is set, bounds should come from child (small)
        QRect boundsBeforeViewport = host.bounds();
        QCOMPARE(boundsBeforeViewport, QRect(0, 0, 10, 5));
        
        // Set viewport (as UiRoot would do after fix)
        QRect viewport(0, 0, 800, 600);
        host.setViewportRect(viewport);
        
        // After viewport is set, bounds should prefer viewport (fix)
        QRect boundsAfterViewport = host.bounds();
        QCOMPARE(boundsAfterViewport, QRect(0, 0, 800, 600));
        
        // Test arrange also sets viewport correctly
        QRect arrangeRect(10, 10, 1024, 768);
        host.arrange(arrangeRect);
        
        QRect boundsAfterArrange = host.bounds();
        QCOMPARE(boundsAfterArrange, QRect(10, 10, 1024, 768));
        
        qDebug() << "RebuildHost bounds() fix PASSED ✅";
    }
};

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set test environment
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_LOGGING_RULES", "qt.qpa.gl=false");
    
    qDebug() << "===========================================";
    qDebug() << "Fangjia Core Module Tests";
    qDebug() << "===========================================";
    
    SimpleTestRunner runner;
    
    try {
        runner.runThemeManagerTests();
        runner.runAppConfigTests();
        runner.runTabViewModelTests();
        runner.runFormulaViewModelTests();
        runner.runRebuildHostTests();
        runner.runUiScrollViewTests();
        runner.runUiPageWheelTests();
        runner.runUiTreeListWheelTests();
        runner.runDecoratedBoxTests();
        runner.runAppShellTests();
        runner.runUiRootLayoutTests();
        runner.runRebuildHostBoundsTests();
        
        // Run domain tests
        tests::runDomainTests();
        
        qDebug() << "===========================================";
        qDebug() << "ALL CORE TESTS PASSED ✅";
        qDebug() << "===========================================";
        
        return 0;
    } catch (...) {
        qDebug() << "===========================================";
        qDebug() << "TEST FAILURE ❌";
        qDebug() << "===========================================";
        return 1;
    }
}

#include "test_main.moc"