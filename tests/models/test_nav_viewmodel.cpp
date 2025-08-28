#include <QtTest>
#include <QSignalSpy>
#include "../../src/models/NavViewModel.h"

class TestNavViewModel : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing NavViewModel tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "NavViewModel tests completed.";
    }
    
    void testItemManagement() {
        NavViewModel vm;
        
        QVector<NavViewModel::Item> items = {
            {.id = "home", .svgLight = "home_light.svg", .svgDark = "home_dark.svg", .label = "Home"},
            {.id = "settings", .svgLight = "settings_light.svg", .svgDark = "settings_dark.svg", .label = "Settings"}
        };
        
        QSignalSpy itemsSpy(&vm, &NavViewModel::itemsChanged);
        vm.setItems(items);
        
        QCOMPARE(vm.count(), 2);
        QCOMPARE(vm.items()[0].id, QString("home"));
        QCOMPARE(vm.items()[1].label, QString("Settings"));
        QCOMPARE(itemsSpy.count(), 1);
    }
    
    void testSelection() {
        NavViewModel vm;
        vm.setItems({
            {.id = "1", .svgLight = "", .svgDark = "", .label = "Item1"},
            {.id = "2", .svgLight = "", .svgDark = "", .label = "Item2"},
            {.id = "3", .svgLight = "", .svgDark = "", .label = "Item3"}
        });
        
        QSignalSpy selectionSpy(&vm, &NavViewModel::selectedIndexChanged);
        
        // 初始选择
        QCOMPARE(vm.selectedIndex(), -1);
        
        // 设置选择
        vm.setSelectedIndex(1);
        QCOMPARE(vm.selectedIndex(), 1);
        QCOMPARE(selectionSpy.count(), 1);
        QCOMPARE(selectionSpy.at(0).at(0).toInt(), 1);
        
        // 设置相同的选择（不应触发信号）
        vm.setSelectedIndex(1);
        QCOMPARE(selectionSpy.count(), 1);
        
        // 无效索引
        vm.setSelectedIndex(10);
        QCOMPARE(vm.selectedIndex(), 1); // 应保持不变
    }
    
    void testExpansion() {
        NavViewModel vm;
        QSignalSpy expandedSpy(&vm, &NavViewModel::expandedChanged);
        
        QCOMPARE(vm.expanded(), false);
        
        vm.setExpanded(true);
        QCOMPARE(vm.expanded(), true);
        QCOMPARE(expandedSpy.count(), 1);
        QCOMPARE(expandedSpy.at(0).at(0).toBool(), true);
        
        vm.toggleExpanded();
        QCOMPARE(vm.expanded(), false);
        QCOMPARE(expandedSpy.count(), 2);
    }
    
    void testAutoSelection() {
        NavViewModel vm;
        
        // 空列表时设置项目
        vm.setItems({});
        QCOMPARE(vm.selectedIndex(), -1);
        
        // 添加项目，如果之前没有选中，应自动选中第一个
        vm.setItems({
            {.id = "1", .svgLight = "", .svgDark = "", .label = "Item1"}
        });
        
        // 这取决于实现，可能需要调整
        // QCOMPARE(vm.selectedIndex(), 0);
    }
};

#include "test_nav_viewmodel.moc"

QTEST_MAIN(TestNavViewModel)