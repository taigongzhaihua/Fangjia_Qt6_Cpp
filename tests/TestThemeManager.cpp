#include <QtTest>
#include <QSignalSpy>
#include <QApplication>
#include "models/ThemeManager.h"

class TestThemeManager : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // Create QApplication to ensure styleHints is available
        if (!qApp) {
            int argc = 0;
            char* argv[] = {nullptr};
            new QApplication(argc, argv);
        }
    }

    void testModeSetAndGet()
    {
        ThemeManager manager;
        
        // Test initial mode
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::FollowSystem);
        
        // Test setting Light mode
        QSignalSpy spy(&manager, &ThemeManager::modeChanged);
        manager.setMode(ThemeManager::ThemeMode::Light);
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::Light);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).value<ThemeManager::ThemeMode>(), ThemeManager::ThemeMode::Light);
        
        // Test setting Dark mode
        manager.setMode(ThemeManager::ThemeMode::Dark);
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::Dark);
        QCOMPARE(spy.count(), 1);
        
        // Test setting same mode (should not emit signal)
        spy.clear();
        manager.setMode(ThemeManager::ThemeMode::Dark);
        QCOMPARE(spy.count(), 0);
    }

    void testEffectiveColorScheme()
    {
        ThemeManager manager;
        QSignalSpy spy(&manager, &ThemeManager::effectiveColorSchemeChanged);
        
        // Test Light mode
        manager.setMode(ThemeManager::ThemeMode::Light);
        QCOMPARE(manager.effectiveColorScheme(), Qt::ColorScheme::Light);
        
        // Test Dark mode  
        manager.setMode(ThemeManager::ThemeMode::Dark);
        QCOMPARE(manager.effectiveColorScheme(), Qt::ColorScheme::Dark);
        
        // Verify signal was emitted when effective scheme changed
        QVERIFY(spy.count() >= 1);
    }

    void testCycleMode()
    {
        ThemeManager manager;
        QSignalSpy spy(&manager, &ThemeManager::modeChanged);
        
        // Start with FollowSystem, cycle should go to Light
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::FollowSystem);
        manager.cycleMode();
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::Light);
        QCOMPARE(spy.count(), 1);
        
        // Light -> Dark
        manager.cycleMode();
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::Dark);
        QCOMPARE(spy.count(), 2);
        
        // Dark -> FollowSystem
        manager.cycleMode();
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::FollowSystem);
        QCOMPARE(spy.count(), 3);
    }

    void testFollowSystemMode()
    {
        ThemeManager manager;
        
        // Set to FollowSystem mode
        manager.setMode(ThemeManager::ThemeMode::FollowSystem);
        QCOMPARE(manager.mode(), ThemeManager::ThemeMode::FollowSystem);
        
        // Effective color scheme should be determined by system
        // Note: We don't test actual system change events to avoid CI/local inconsistencies
        Qt::ColorScheme effective = manager.effectiveColorScheme();
        QVERIFY(effective == Qt::ColorScheme::Light || effective == Qt::ColorScheme::Dark);
    }

    void testLoadSave()
    {
        // Use separate organization/app name to avoid polluting real settings
        QCoreApplication::setOrganizationName("TestOrg");
        QCoreApplication::setApplicationName("TestApp");
        
        {
            ThemeManager manager;
            manager.setMode(ThemeManager::ThemeMode::Dark);
            manager.save();
        }
        
        // Create new instance and load
        {
            ThemeManager manager2;
            manager2.load();
            QCOMPARE(manager2.mode(), ThemeManager::ThemeMode::Dark);
        }
        
        // Clean up
        QSettings settings;
        settings.clear();
    }
};

#include "TestThemeManager.moc"
QTEST_MAIN(TestThemeManager)