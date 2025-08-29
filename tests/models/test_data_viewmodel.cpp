#include <QtTest>
#include <QSignalSpy>
#include <QTemporaryFile>
#include <QSettings>
#include "../../src/models/DataViewModel.h"
#include "AppConfig.h"

class TestDataViewModel : public QObject
{
    Q_OBJECT
    
private slots:
    void initTestCase() {
        qDebug() << "Initializing DataViewModel tests...";
    }
    
    void cleanupTestCase() {
        qDebug() << "DataViewModel tests completed.";
    }
    
    void testInitialization() {
        // Create a temporary config for testing
        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        tempFile.close();
        
        QSettings testSettings(tempFile.fileName(), QSettings::IniFormat);
        AppConfig config;
        
        // Create DataViewModel
        DataViewModel dataVm(config);
        
        // Verify that TabViewModel is properly initialized
        QVERIFY(dataVm.tabs() != nullptr);
        QCOMPARE(dataVm.tabs()->count(), 6);  // Should have 6 tabs as per the implementation
        
        // Verify the first tab is "formula"
        QCOMPARE(dataVm.tabs()->items()[0].id, QString("formula"));
        QCOMPARE(dataVm.tabs()->items()[0].label, QString("方剂"));
    }
    
    void testConfigIntegration() {
        // Create a temporary config for testing
        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        tempFile.close();
        
        AppConfig config;
        
        // Set a recent tab in config
        config.setRecentTab("herb");
        
        // Create DataViewModel - it should restore the recent tab
        DataViewModel dataVm(config);
        
        // Find the herb tab index
        int herbIndex = dataVm.tabs()->findById("herb");
        QVERIFY(herbIndex >= 0);
        
        // Verify that the DataViewModel restored the recent tab
        QCOMPARE(dataVm.selectedTab(), herbIndex);
        QCOMPARE(dataVm.tabs()->selectedId(), QString("herb"));
    }
    
    void testTabChangeNotifications() {
        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        tempFile.close();
        
        AppConfig config;
        DataViewModel dataVm(config);
        
        // Set up signal spies
        QSignalSpy selectedTabSpy(&dataVm, &DataViewModel::selectedTabChanged);
        
        // Change tab selection
        int classicIndex = dataVm.tabs()->findById("classic");
        QVERIFY(classicIndex >= 0);
        
        dataVm.tabs()->setSelectedIndex(classicIndex);
        
        // Verify notifications were emitted
        QCOMPARE(selectedTabSpy.count(), 1);
        QCOMPARE(selectedTabSpy.at(0).at(0).toInt(), classicIndex);
        
        // Verify config was updated
        QCOMPARE(config.recentTab(), QString("classic"));
    }
    
    void testPropertyAccess() {
        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        tempFile.close();
        
        AppConfig config;
        DataViewModel dataVm(config);
        
        // Test selectedTab property
        QCOMPARE(dataVm.selectedTab(), dataVm.tabs()->selectedIndex());
        
        // Change selection and verify property consistency
        dataVm.tabs()->setSelectedIndex(2);
        QCOMPARE(dataVm.selectedTab(), 2);
        QCOMPARE(dataVm.selectedTab(), dataVm.tabs()->selectedIndex());
    }
};

#include "test_data_viewmodel.moc"

QTEST_MAIN(TestDataViewModel)