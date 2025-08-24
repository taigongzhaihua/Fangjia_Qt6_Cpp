#include <QtTest>
#include <QSignalSpy>
#include "../../src/models/FormulaViewModel.h"

class TestFormulaViewModel : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing FormulaViewModel tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "FormulaViewModel tests completed.";
    }
    
    void testDataLoading() {
        FormulaViewModel vm;
        QSignalSpy dataSpy(&vm, &FormulaViewModel::dataChanged);
        
        vm.loadSampleData();
        
        QVERIFY(vm.nodeCount() > 0);
        QCOMPARE(dataSpy.count(), 1);
    }
    
    void testTreeStructure() {
        FormulaViewModel vm;
        vm.loadSampleData();
        
        // 查找根节点的子节点
        QVector<int> rootChildren = vm.childIndices(-1);
        QVERIFY(rootChildren.size() > 0);
        
        // 验证第一个分类有子分类
        if (!rootChildren.isEmpty()) {
            int firstCategory = rootChildren[0];
            QVector<int> subCategories = vm.childIndices(firstCategory);
            QVERIFY(subCategories.size() > 0);
        }
    }
    
    void testSelection() {
        FormulaViewModel vm;
        vm.loadSampleData();
        
        QSignalSpy selectionSpy(&vm, &FormulaViewModel::selectedChanged);
        
        QCOMPARE(vm.selectedIndex(), -1);
        
        // 选择一个节点
        vm.setSelectedIndex(0);
        QCOMPARE(vm.selectedIndex(), 0);
        QCOMPARE(selectionSpy.count(), 1);
        
        // 获取选中的方剂详情（如果是方剂节点）
        const auto* detail = vm.selectedFormula();
        // detail 可能为 nullptr（如果选中的是分类节点）
    }
    
    void testExpansion() {
        FormulaViewModel vm;
        vm.loadSampleData();
        
        if (vm.nodeCount() > 0) {
            QSignalSpy expandSpy(&vm, &FormulaViewModel::nodeExpandChanged);
            
            // 测试展开/折叠
            bool initialExpanded = vm.nodes()[0].expanded;
            vm.toggleExpanded(0);
            
            QCOMPARE(vm.nodes()[0].expanded, !initialExpanded);
            QCOMPARE(expandSpy.count(), 1);
            QCOMPARE(expandSpy.at(0).at(0).toInt(), 0);
            QCOMPARE(expandSpy.at(0).at(1).toBool(), !initialExpanded);
        }
    }
    
    void testFormulaDetail() {
        FormulaViewModel vm;
        vm.loadSampleData();
        
        // 查找一个方剂节点（level == 2）
        int formulaIndex = -1;
        for (int i = 0; i < vm.nodeCount(); ++i) {
            if (vm.nodes()[i].level == 2) {
                formulaIndex = i;
                break;
            }
        }
        
        if (formulaIndex >= 0) {
            vm.setSelectedIndex(formulaIndex);
            const auto* detail = vm.selectedFormula();
            
            QVERIFY(detail != nullptr);
            QVERIFY(!detail->name.isEmpty());
            QVERIFY(!detail->composition.isEmpty());
        }
    }
    
    void testClearData() {
        FormulaViewModel vm;
        vm.loadSampleData();
        
        QVERIFY(vm.nodeCount() > 0);
        
        QSignalSpy dataSpy(&vm, &FormulaViewModel::dataChanged);
        vm.clearData();
        
        QCOMPARE(vm.nodeCount(), 0);
        QCOMPARE(vm.selectedIndex(), -1);
        QVERIFY(vm.selectedFormula() == nullptr);
        QVERIFY(dataSpy.count() > 0);
    }
};