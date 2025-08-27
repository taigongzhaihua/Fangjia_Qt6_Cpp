#include <QtTest>
#include <QSignalSpy>
#include <QRect>
#include <QPoint>
#include "../../src/framework/declarative/Binding.h"
#include "../../src/framework/declarative/UI.h"
#include "../../src/models/TabViewModel.h"
#include "../../src/core/rendering/RenderData.hpp"

// 测试用的 ViewModel
class TestViewModel : public QObject {
    Q_OBJECT
public:
    explicit TestViewModel(QObject* parent = nullptr) : QObject(parent) {}
    
    void setCount(int count) {
        if (m_count != count) {
            m_count = count;
            emit countChanged();
        }
    }
    
    int count() const { return m_count; }
    
signals:
    void countChanged();
    
private:
    int m_count{0};
};

class TestBindingHost : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing BindingHost tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "BindingHost tests completed.";
    }
    
    void testBasicConstruction() {
        // 测试基本构造和 build
        auto buildCount = 0;
        auto bindingWidget = UI::bindingHost([&buildCount]() -> UI::WidgetPtr {
            buildCount++;
            return UI::text(QString("Build count: %1").arg(buildCount));
        });
        
        QVERIFY(bindingWidget != nullptr);
        
        // 验证可以 build
        auto component = bindingWidget->build();
        QVERIFY(component != nullptr);
        
        // 第一次构建应该触发一次 build
        QCOMPARE(buildCount, 1);
    }
    
    void testSignalBinding() {
        TestViewModel vm;
        int buildCount = 0;
        bool connectionMade = false;
        
        auto bindingWidget = UI::bindingHost([&vm, &buildCount]() -> UI::WidgetPtr {
            buildCount++;
            return UI::text(QString("Count: %1").arg(vm.count()));
        })
        ->connect([&vm, &connectionMade](UI::RebuildHost* host) {
            connectionMade = true;
            // 使用 observe 辅助函数连接信号
            UI::observe(&vm, &TestViewModel::countChanged, [host]() {
                host->requestRebuild();
            });
        });
        
        // 构建组件
        auto component = bindingWidget->build();
        QVERIFY(component != nullptr);
        QVERIFY(connectionMade);
        
        // 初始构建应该发生一次
        QCOMPARE(buildCount, 1);
        
        // 改变 VM 状态应该触发重建
        vm.setCount(42);
        
        // 这里我们验证 buildCount 增加了（说明重建被触发）
        // 注意：信号是异步的，可能需要处理事件循环
        QCoreApplication::processEvents();
        QVERIFY(buildCount > 1);
    }
    
    void testObserveFunction() {
        TestViewModel vm;
        bool signalReceived = false;
        
        // 测试 observe 辅助函数
        auto connection = UI::observe(&vm, &TestViewModel::countChanged, [&signalReceived]() {
            signalReceived = true;
        });
        
        QVERIFY(connection);
        
        // 触发信号
        vm.setCount(123);
        
        QVERIFY(signalReceived);
        
        // 断开连接
        QObject::disconnect(connection);
        signalReceived = false;
        vm.setCount(456);
        QVERIFY(!signalReceived);
    }
    
    void testTabViewModelIntegration() {
        // 测试与 TabViewModel 的集成
        TabViewModel tabVm;
        tabVm.setItems(QVector<TabViewModel::TabItem>{
            {.id = "tab1", .label = "Tab 1", .tooltip = "First tab"},
            {.id = "tab2", .label = "Tab 2", .tooltip = "Second tab"}
        });
        
        int buildCount = 0;
        
        auto bindingWidget = UI::bindingHost([&tabVm, &buildCount]() -> UI::WidgetPtr {
            buildCount++;
            const auto selectedId = tabVm.selectedId();
            return UI::text(QString("Selected: %1").arg(selectedId));
        })
        ->connect([&tabVm](UI::RebuildHost* host) {
            UI::observe(&tabVm, &TabViewModel::selectedIndexChanged, [host](int) {
                host->requestRebuild();
            });
        });
        
        auto component = bindingWidget->build();
        QVERIFY(component != nullptr);
        QCOMPARE(buildCount, 1);
        
        // 改变选中项应该触发重建
        tabVm.setSelectedIndex(1);
        QCoreApplication::processEvents();
        
        QVERIFY(buildCount > 1);
    }
};

#include "test_binding_host.moc"

QTEST_MAIN(TestBindingHost)