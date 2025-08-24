#include <QtTest>
#include <QSignalSpy>
#include "../../src/models/TabViewModel.h"

class TestTabViewModel : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing TabViewModel tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "TabViewModel tests completed.";
    }
    
    void testTabManagement() {
        TabViewModel vm;
        
        QVector<TabViewModel::TabItem> tabs = {
            {.id = "tab1", .label = "Tab 1", .tooltip = "First tab"},
            {.id = "tab2", .label = "Tab 2", .tooltip = "Second tab"},
            {.id = "tab3", .label = "Tab 3", .tooltip = "Third tab"}
        };
        
        QSignalSpy itemsSpy(&vm, &TabViewModel::itemsChanged);
        vm.setItems(tabs);
        
        QCOMPARE(vm.count(), 3);
        QCOMPARE(vm.items()[0].id, QString("tab1"));
        QCOMPARE(vm.items()[2].tooltip, QString("Third tab"));
        QCOMPARE(itemsSpy.count(), 1);
    }
    
    void testTabSelection() {
        TabViewModel vm;
        vm.setItems({
            {.id = "tab1", .label = "Tab 1"},
            {.id = "tab2", .label = "Tab 2"}
        });
        
        QSignalSpy selectionSpy(&vm, &TabViewModel::selectedIndexChanged);
        
        QCOMPARE(vm.selectedIndex(), 0); // 默认选中第一个
        
        vm.setSelectedIndex(1);
        QCOMPARE(vm.selectedIndex(), 1);
        QCOMPARE(vm.selectedId(), QString("tab2"));
        QCOMPARE(selectionSpy.count(), 1);
    }
    
    void testFindById() {
        TabViewModel vm;
        vm.setItems({
            {.id = "home", .label = "Home"},
            {.id = "profile", .label = "Profile"},
            {.id = "settings", .label = "Settings"}
        });
        
        QCOMPARE(vm.findById("profile"), 1);
        QCOMPARE(vm.findById("settings"), 2);
        QCOMPARE(vm.findById("nonexistent"), -1);
    }
    
    void testSelectedId() {
        TabViewModel vm;
        vm.setItems({
            {.id = "alpha", .label = "Alpha"},
            {.id = "beta", .label = "Beta"}
        });
        
        vm.setSelectedIndex(0);
        QCOMPARE(vm.selectedId(), QString("alpha"));
        
        vm.setSelectedIndex(1);
        QCOMPARE(vm.selectedId(), QString("beta"));
        
        vm.setSelectedIndex(-1);
        QVERIFY(vm.selectedId().isEmpty());
    }
};