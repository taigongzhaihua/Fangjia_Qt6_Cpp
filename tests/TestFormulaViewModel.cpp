#include <QtTest>
#include <QSignalSpy>
#include "models/FormulaViewModel.h"

class TestFormulaViewModel : public QObject
{
    Q_OBJECT

private slots:
    void testInitialState()
    {
        FormulaViewModel formulaVm;
        
        // Test initial state
        QCOMPARE(formulaVm.nodeCount(), 0);
        QCOMPARE(formulaVm.selectedIndex(), -1);
        QVERIFY(formulaVm.nodes().isEmpty());
        QCOMPARE(formulaVm.selectedFormula(), nullptr);
    }

    void testLoadSampleData()
    {
        FormulaViewModel formulaVm;
        QSignalSpy spy(&formulaVm, &FormulaViewModel::dataChanged);
        
        // Load sample data
        formulaVm.loadSampleData();
        
        // Verify data was loaded
        QVERIFY(formulaVm.nodeCount() > 0);
        QVERIFY(!formulaVm.nodes().isEmpty());
        QCOMPARE(spy.count(), 1);
        
        // Verify structure has categories, subcategories, and formulas
        const auto& nodes = formulaVm.nodes();
        bool hasCategory = false;
        bool hasSubCategory = false;
        bool hasFormula = false;
        
        for (const auto& node : nodes) {
            if (node.level == 0) hasCategory = true;
            else if (node.level == 1) hasSubCategory = true;
            else if (node.level == 2) hasFormula = true;
        }
        
        QVERIFY(hasCategory);
        QVERIFY(hasSubCategory);
        QVERIFY(hasFormula);
    }

    void testClearData()
    {
        FormulaViewModel formulaVm;
        
        // Load data first
        formulaVm.loadSampleData();
        QVERIFY(formulaVm.nodeCount() > 0);
        
        QSignalSpy spy(&formulaVm, &FormulaViewModel::dataChanged);
        
        // Clear data
        formulaVm.clearData();
        
        // Verify data was cleared
        QCOMPARE(formulaVm.nodeCount(), 0);
        QVERIFY(formulaVm.nodes().isEmpty());
        QCOMPARE(formulaVm.selectedIndex(), -1);
        QCOMPARE(spy.count(), 1);
    }

    void testSelectedIndexChange()
    {
        FormulaViewModel formulaVm;
        formulaVm.loadSampleData();
        QVERIFY(formulaVm.nodeCount() > 0);
        
        QSignalSpy spy(&formulaVm, &FormulaViewModel::selectedChanged);
        
        // Test setting valid selection
        formulaVm.setSelectedIndex(0);
        QCOMPARE(formulaVm.selectedIndex(), 0);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toInt(), 0);
        
        // Test setting another valid selection
        if (formulaVm.nodeCount() > 1) {
            formulaVm.setSelectedIndex(1);
            QCOMPARE(formulaVm.selectedIndex(), 1);
            QCOMPARE(spy.count(), 1);
        }
        
        // Test setting same selection (should not emit signal)
        spy.clear();
        formulaVm.setSelectedIndex(formulaVm.selectedIndex());
        QCOMPARE(spy.count(), 0);
    }

    void testNodeExpansion()
    {
        FormulaViewModel formulaVm;
        formulaVm.loadSampleData();
        QVERIFY(formulaVm.nodeCount() > 0);
        
        // Find a node with children (level 0 or 1)
        int nodeWithChildren = -1;
        const auto& nodes = formulaVm.nodes();
        for (int i = 0; i < nodes.size(); ++i) {
            if (nodes[i].level < 2) {  // Categories and subcategories have children
                QVector<int> children = formulaVm.childIndices(i);
                if (!children.isEmpty()) {
                    nodeWithChildren = i;
                    break;
                }
            }
        }
        
        if (nodeWithChildren >= 0) {
            QSignalSpy spy(&formulaVm, &FormulaViewModel::nodeExpandChanged);
            
            // Test setting expanded
            formulaVm.setExpanded(nodeWithChildren, true);
            QCOMPARE(spy.count(), 1);
            auto args = spy.takeFirst();
            QCOMPARE(args.at(0).toInt(), nodeWithChildren);
            QCOMPARE(args.at(1).toBool(), true);
            
            // Test setting collapsed
            formulaVm.setExpanded(nodeWithChildren, false);
            QCOMPARE(spy.count(), 1);
            args = spy.takeFirst();
            QCOMPARE(args.at(0).toInt(), nodeWithChildren);
            QCOMPARE(args.at(1).toBool(), false);
            
            // Test toggle
            bool wasExpanded = nodes[nodeWithChildren].expanded;
            formulaVm.toggleExpanded(nodeWithChildren);
            QCOMPARE(spy.count(), 1);
        }
    }

    void testChildIndices()
    {
        FormulaViewModel formulaVm;
        formulaVm.loadSampleData();
        
        const auto& nodes = formulaVm.nodes();
        
        // Test child relationships
        for (int i = 0; i < nodes.size(); ++i) {
            QVector<int> children = formulaVm.childIndices(i);
            
            // Verify children have correct parent
            for (int childIdx : children) {
                QVERIFY(childIdx >= 0 && childIdx < nodes.size());
                QCOMPARE(nodes[childIdx].parentIndex, i);
            }
        }
        
        // Test non-existent parent
        QVector<int> noChildren = formulaVm.childIndices(-1);
        QVERIFY(noChildren.isEmpty());
        
        QVector<int> outOfRange = formulaVm.childIndices(9999);
        QVERIFY(outOfRange.isEmpty());
    }

    void testSelectedFormula()
    {
        FormulaViewModel formulaVm;
        formulaVm.loadSampleData();
        
        // Find a formula node (level 2)
        int formulaIndex = -1;
        const auto& nodes = formulaVm.nodes();
        for (int i = 0; i < nodes.size(); ++i) {
            if (nodes[i].level == 2 && nodes[i].detail != nullptr) {
                formulaIndex = i;
                break;
            }
        }
        
        if (formulaIndex >= 0) {
            // Select the formula
            formulaVm.setSelectedIndex(formulaIndex);
            
            // Get selected formula
            const auto* formula = formulaVm.selectedFormula();
            QVERIFY(formula != nullptr);
            QVERIFY(!formula->name.isEmpty());
            
            // Verify it matches the node's detail
            QCOMPARE(formula, nodes[formulaIndex].detail);
        }
        
        // Test with non-formula selection
        formulaVm.setSelectedIndex(-1);
        QCOMPARE(formulaVm.selectedFormula(), nullptr);
    }

    void testNodeStructure()
    {
        FormulaViewModel formulaVm;
        formulaVm.loadSampleData();
        
        const auto& nodes = formulaVm.nodes();
        
        // Verify node structure consistency
        for (int i = 0; i < nodes.size(); ++i) {
            const auto& node = nodes[i];
            
            // Verify basic properties
            QVERIFY(!node.id.isEmpty());
            QVERIFY(!node.label.isEmpty());
            QVERIFY(node.level >= 0 && node.level <= 2);
            
            // Verify parent-child relationships
            if (node.parentIndex >= 0) {
                QVERIFY(node.parentIndex < nodes.size());
                QVERIFY(nodes[node.parentIndex].level < node.level);
            }
            
            // Verify detail is only on formula nodes
            if (node.level == 2) {
                // Formula nodes may have detail
            } else {
                // Category and subcategory nodes should not have detail
                QCOMPARE(node.detail, nullptr);
            }
        }
    }

    void testLoadSampleDataContent()
    {
        FormulaViewModel formulaVm;
        formulaVm.loadSampleData();
        
        const auto& nodes = formulaVm.nodes();
        QVERIFY(!nodes.isEmpty());
        
        // Verify some expected content from sample data
        bool foundJiebiao = false;
        bool foundMahuang = false;
        
        for (const auto& node : nodes) {
            if (node.label.contains("解表剂")) {
                foundJiebiao = true;
            }
            if (node.label.contains("麻黄汤")) {
                foundMahuang = true;
            }
        }
        
        QVERIFY(foundJiebiao);  // Should have the 解表剂 category
        QVERIFY(foundMahuang);  // Should have the 麻黄汤 formula
    }
};

#include "TestFormulaViewModel.moc"
QTEST_MAIN(TestFormulaViewModel)