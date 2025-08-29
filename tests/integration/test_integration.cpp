#include <QtTest>
#include <QOpenGLContext>
#include <QOffscreenSurface>
#include <memory>
#include "AppConfig.h"
#include "../../src/models/NavViewModel.h"
#include "../../src/models/TabViewModel.h"
#include "../../src/framework/containers/UiRoot.h"
#include "../../src/framework/widgets/UiNav.h"
#include "../../src/framework/widgets/UiTabView.h"

class TestIntegration : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing Integration tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "Integration tests completed.";
    }
    
    void testServiceIntegration() {
        // 测试组件直接交互，无需 DI
        auto config = std::make_shared<AppConfig>();
        auto navVm = std::make_shared<NavViewModel>();
        auto tabVm = std::make_shared<TabViewModel>();
        
        // 验证组件交互
        navVm->setItems({
            {.id = "home", .svgLight = "", .svgDark = "", .label = "Home"}
        });
        
        config->setValue("nav/selected", 0);
        QCOMPARE(config->value("nav/selected").toInt(), 0);
        
        // 验证 TabView 需要 VM 才能工作
        UiTabView tabView;
        tabView.setViewModel(tabVm.get());
        QVERIFY(tabView.viewModel() == tabVm.get());
    }
    
    void testUiComponentIntegration() {
        // 创建组件树
        UiRoot root;
        auto nav = std::make_unique<Ui::NavRail>();
        auto tabView = std::make_unique<UiTabView>();
        
        // 配置组件
        auto navVm = std::make_shared<NavViewModel>();
        navVm->setItems({
            {.id = "1", .svgLight = "", .svgDark = "", .label = "Item1"},
            {.id = "2", .svgLight = "", .svgDark = "", .label = "Item2"}
        });
        nav->setViewModel(navVm.get());
        
        auto tabVm = std::make_shared<TabViewModel>();
        tabVm->setItems({
            {.id = "tab1", .label = "Tab 1"},
            {.id = "tab2", .label = "Tab 2"}
        });
        tabView->setViewModel(tabVm.get());
        
        // 添加到根
        root.add(nav.get());
        root.add(tabView.get());
        
        // 更新布局
        root.updateLayout(QSize(800, 600));
        
        // 验证布局已应用
        QVERIFY(!nav->bounds().isEmpty());
        QVERIFY(!tabView->bounds().isEmpty());
        
        // 测试事件分发
        QPoint clickPos(100, 100);
        bool handled = root.onMousePress(clickPos);
        // 具体结果取决于组件位置
        
        // 测试绘制
        Render::FrameData fd;
        root.append(fd);
        QVERIFY(!fd.empty());
    }
    
    void testViewModelSync() {
        // 测试ViewModel与View的同步
        auto navVm = std::make_shared<NavViewModel>();
        Ui::NavRail nav;
        nav.setViewModel(navVm.get());
        
        // 设置数据
        navVm->setItems({
            {.id = "1", .svgLight = "", .svgDark = "", .label = "Item1"},
            {.id = "2", .svgLight = "", .svgDark = "", .label = "Item2"},
            {.id = "3", .svgLight = "", .svgDark = "", .label = "Item3"}
        });
        
        // 改变选择
        navVm->setSelectedIndex(1);
        
        // View应该反映这个变化（通过tick）
        bool hasAnimation = nav.tick();
        // 可能需要动画
        
        // 改变展开状态
        navVm->setExpanded(true);
        hasAnimation = nav.tick() || hasAnimation;
        
        // 验证宽度变化
        int widthBefore = nav.currentWidth();
        
        // 等待动画完成
        for (int i = 0; i < 100 && nav.hasActiveAnimation(); ++i) {
            nav.tick();
            QTest::qWait(16);
        }
        
        int widthAfter = nav.currentWidth();
        QVERIFY(widthAfter != widthBefore);
    }
    
    void testConfigPersistence() {
        const QString testKey = "integration/test";
        const int testValue = 42;
        
        {
            // 作用域1：写入配置
            auto config = std::make_shared<AppConfig>();
            config->setValue(testKey, testValue);
            config->sync();
        }
        
        {
            // 作用域2：读取配置
            auto config = std::make_shared<AppConfig>();
            QCOMPARE(config->value(testKey).toInt(), testValue);
            
            // 清理测试数据
            config->setValue(testKey, QVariant());
        }
    }
    
    void testMemoryManagement() {
        // 测试组件生命周期管理
        {
            UiRoot root;
            
            // 使用unique_ptr管理组件
            auto comp1 = std::make_unique<Ui::NavRail>();
            auto comp2 = std::make_unique<UiTabView>();
            
            // 添加到root（root不拥有所有权）
            root.add(comp1.get());
            root.add(comp2.get());
            
            // 验证正常工作
            root.updateLayout(QSize(800, 600));
            Render::FrameData fd;
            root.append(fd);
            
            // 移除组件
            root.remove(comp1.get());
            
            // comp1和comp2在作用域结束时自动删除
        }
        
        // 这里不应该有内存泄漏
        // 可以使用valgrind或其他工具验证
    }
    
    void testPerformance() {
        // 性能测试：大量组件
        UiRoot root;
        std::vector<std::unique_ptr<Ui::Button>> buttons;
        
        const int buttonCount = 100;
        
        // 创建大量按钮
        for (int i = 0; i < buttonCount; ++i) {
            auto btn = std::make_unique<Ui::Button>();
            btn->setBaseRect(QRect(i * 10, i * 10, 50, 30));
            root.add(btn.get());
            buttons.push_back(std::move(btn));
        }
        
        // 测量布局更新时间
        QElapsedTimer timer;
        timer.start();
        
        root.updateLayout(QSize(1920, 1080));
        
        qint64 layoutTime = timer.elapsed();
        qDebug() << "Layout" << buttonCount << "components took" << layoutTime << "ms";
        QVERIFY(layoutTime < 100); // 应该在100ms内完成
        
        // 测量绘制命令生成时间
        timer.restart();
        
        Render::FrameData fd;
        root.append(fd);
        
        qint64 renderTime = timer.elapsed();
        qDebug() << "Generate render commands took" << renderTime << "ms";
        QVERIFY(renderTime < 50); // 应该在50ms内完成
        
        // 验证生成的命令数量
        QVERIFY(fd.roundedRects.size() >= buttonCount);
    }
};

#include "test_integration.moc"

QTEST_MAIN(TestIntegration)