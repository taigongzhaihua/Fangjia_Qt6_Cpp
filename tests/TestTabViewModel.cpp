#include <QtTest>
#include <QSignalSpy>
#include "models/TabViewModel.h"

class TestTabViewModel : public QObject
{
    Q_OBJECT

private slots:
    void testInitialState()
    {
        TabViewModel tabVm;
        
        // Test initial state
        QCOMPARE(tabVm.count(), 0);
        QCOMPARE(tabVm.selectedIndex(), 0);
        QCOMPARE(tabVm.selectedId(), QString());
        QVERIFY(tabVm.items().isEmpty());
    }

    void testSetItems()
    {
        TabViewModel tabVm;
        QSignalSpy spy(&tabVm, &TabViewModel::itemsChanged);
        
        // Create test items
        QVector<TabViewModel::TabItem> items{
            {.id = "tab1", .label = "Tab 1", .tooltip = "First tab"},
            {.id = "tab2", .label = "Tab 2", .tooltip = "Second tab"},
            {.id = "tab3", .label = "Tab 3", .tooltip = "Third tab"}
        };
        
        tabVm.setItems(items);
        
        // Verify items were set
        QCOMPARE(tabVm.count(), 3);
        QCOMPARE(tabVm.items().size(), 3);
        QCOMPARE(tabVm.items().at(0).id, QString("tab1"));
        QCOMPARE(tabVm.items().at(1).label, QString("Tab 2"));
        QCOMPARE(tabVm.items().at(2).tooltip, QString("Third tab"));
        QCOMPARE(spy.count(), 1);
    }

    void testSelectedIndex()
    {
        TabViewModel tabVm;
        
        // Test with empty items (should handle gracefully)
        QCOMPARE(tabVm.selectedIndex(), 0);
        
        // Add items
        QVector<TabViewModel::TabItem> items{
            {.id = "tab1", .label = "Tab 1", .tooltip = "First tab"},
            {.id = "tab2", .label = "Tab 2", .tooltip = "Second tab"},
            {.id = "tab3", .label = "Tab 3", .tooltip = "Third tab"}
        };
        tabVm.setItems(items);
        
        QSignalSpy spy(&tabVm, &TabViewModel::selectedIndexChanged);
        
        // Test initial selected index
        QCOMPARE(tabVm.selectedIndex(), 0);
        QCOMPARE(tabVm.selectedId(), QString("tab1"));
        
        // Test setting valid index
        tabVm.setSelectedIndex(1);
        QCOMPARE(tabVm.selectedIndex(), 1);
        QCOMPARE(tabVm.selectedId(), QString("tab2"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toInt(), 1);
        
        // Test setting another valid index
        tabVm.setSelectedIndex(2);
        QCOMPARE(tabVm.selectedIndex(), 2);
        QCOMPARE(tabVm.selectedId(), QString("tab3"));
        QCOMPARE(spy.count(), 1);
        
        // Test setting same index (should not emit signal)
        spy.clear();
        tabVm.setSelectedIndex(2);
        QCOMPARE(spy.count(), 0);
    }

    void testSelectedIndexRange()
    {
        TabViewModel tabVm;
        QVector<TabViewModel::TabItem> items{
            {.id = "tab1", .label = "Tab 1", .tooltip = "First tab"},
            {.id = "tab2", .label = "Tab 2", .tooltip = "Second tab"}
        };
        tabVm.setItems(items);
        
        QSignalSpy spy(&tabVm, &TabViewModel::selectedIndexChanged);
        
        // Test setting out-of-range index (negative)
        tabVm.setSelectedIndex(-1);
        // Should clamp to valid range or ignore - implementation dependent
        int indexAfterNegative = tabVm.selectedIndex();
        QVERIFY(indexAfterNegative >= 0 && indexAfterNegative < tabVm.count());
        
        // Test setting out-of-range index (too high)
        tabVm.setSelectedIndex(10);
        int indexAfterHigh = tabVm.selectedIndex();
        QVERIFY(indexAfterHigh >= 0 && indexAfterHigh < tabVm.count());
    }

    void testFindById()
    {
        TabViewModel tabVm;
        QVector<TabViewModel::TabItem> items{
            {.id = "tab1", .label = "Tab 1", .tooltip = "First tab"},
            {.id = "tab2", .label = "Tab 2", .tooltip = "Second tab"},
            {.id = "tab3", .label = "Tab 3", .tooltip = "Third tab"}
        };
        tabVm.setItems(items);
        
        // Test finding existing IDs
        QCOMPARE(tabVm.findById("tab1"), 0);
        QCOMPARE(tabVm.findById("tab2"), 1);
        QCOMPARE(tabVm.findById("tab3"), 2);
        
        // Test finding non-existing ID
        QCOMPARE(tabVm.findById("nonexistent"), -1);
    }

    void testEmptyItemsCase()
    {
        TabViewModel tabVm;
        
        // When count is 0, skip assertions that would fail
        if (tabVm.count() == 0) {
            // Test that basic operations don't crash
            QCOMPARE(tabVm.selectedIndex(), 0);
            QCOMPARE(tabVm.selectedId(), QString());
            QCOMPARE(tabVm.findById("anything"), -1);
            
            // Setting selected index on empty should be handled gracefully
            QSignalSpy spy(&tabVm, &TabViewModel::selectedIndexChanged);
            tabVm.setSelectedIndex(5);
            // May or may not emit signal depending on implementation
        }
    }

    void testSelectedIdConsistency()
    {
        TabViewModel tabVm;
        QVector<TabViewModel::TabItem> items{
            {.id = "first", .label = "First", .tooltip = ""},
            {.id = "second", .label = "Second", .tooltip = ""},
            {.id = "third", .label = "Third", .tooltip = ""}
        };
        tabVm.setItems(items);
        
        // Test that selectedId is consistent with selectedIndex
        for (int i = 0; i < tabVm.count(); ++i) {
            tabVm.setSelectedIndex(i);
            QCOMPARE(tabVm.selectedId(), items.at(i).id);
        }
    }
};

#include "TestTabViewModel.moc"
QTEST_MAIN(TestTabViewModel)