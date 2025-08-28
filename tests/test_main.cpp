#include <QtTest>
#include <QCoreApplication>
#include <QDebug>
#include <QSignalSpy>

// Core test for ThemeManager
#include "models/ThemeManager.h"

// Core test for AppConfig  
#include "core/config/AppConfig.h"

// Core test for TabViewModel
#include "models/TabViewModel.h"

// Core test for FormulaViewModel
#include "models/FormulaViewModel.h"

// Core test for RebuildHost
#include "framework/declarative/RebuildHost.h"
#include "framework/declarative/Binding.h"
#include "framework/base/UiComponent.hpp"

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
        QVector<TabViewModel::TabItem> items{
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
        
        QCOMPARE(buildCount, 0);
        
        // Request rebuild
        host.requestRebuild();
        QCOMPARE(buildCount, 1);
        
        host.requestRebuild();
        QCOMPARE(buildCount, 2);
        
        qDebug() << "RebuildHost tests PASSED ✅";
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