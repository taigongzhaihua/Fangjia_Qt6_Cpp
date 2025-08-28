#include <QtTest>
#include <QSignalSpy>
#include <QCoreApplication>
#include "core/config/AppConfig.h"

class TestAppConfig : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase()
    {
        // Use separate organization/app name to avoid polluting real settings
        QCoreApplication::setOrganizationName("TestOrgConfig");
        QCoreApplication::setApplicationName("TestAppConfig");
    }

    void cleanupTestCase()
    {
        // Clean up test settings
        QSettings settings;
        settings.clear();
    }

    void init()
    {
        // Reset before each test
        QSettings settings;
        settings.clear();
    }

    void testThemeMode()
    {
        AppConfig config;
        QSignalSpy spy(&config, &AppConfig::themeModeChanged);
        
        // Test initial value (should be default)
        QString initialMode = config.themeMode();
        QVERIFY(!initialMode.isEmpty());
        
        // Test setting theme mode
        config.setThemeMode("dark");
        QCOMPARE(config.themeMode(), QString("dark"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toString(), QString("dark"));
        
        // Test setting same mode (should still emit signal)
        config.setThemeMode("dark");
        QCOMPARE(spy.count(), 1);
    }

    void testNavExpanded()
    {
        AppConfig config;
        QSignalSpy spy(&config, &AppConfig::navExpandedChanged);
        
        // Test initial value
        bool initialExpanded = config.navExpanded();
        
        // Test setting nav expanded
        config.setNavExpanded(true);
        QCOMPARE(config.navExpanded(), true);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toBool(), true);
        
        // Test setting to false
        config.setNavExpanded(false);
        QCOMPARE(config.navExpanded(), false);
        QCOMPARE(spy.count(), 1);
    }

    void testNavSelectedIndex()
    {
        AppConfig config;
        QSignalSpy spy(&config, &AppConfig::navSelectedIndexChanged);
        
        // Test initial value
        int initialIndex = config.navSelectedIndex();
        QVERIFY(initialIndex >= 0);
        
        // Test setting nav selected index
        config.setNavSelectedIndex(5);
        QCOMPARE(config.navSelectedIndex(), 5);
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toInt(), 5);
        
        // Test setting to 0
        config.setNavSelectedIndex(0);
        QCOMPARE(config.navSelectedIndex(), 0);
        QCOMPARE(spy.count(), 1);
    }

    void testWindowGeometry()
    {
        AppConfig config;
        
        // Test initial value (should be empty or valid)
        QByteArray initialGeometry = config.windowGeometry();
        
        // Test setting window geometry
        QByteArray testGeometry("test_geometry_data");
        config.setWindowGeometry(testGeometry);
        QCOMPARE(config.windowGeometry(), testGeometry);
    }

    void testWindowState()
    {
        AppConfig config;
        
        // Test initial value
        QByteArray initialState = config.windowState();
        
        // Test setting window state
        QByteArray testState("test_state_data");
        config.setWindowState(testState);
        QCOMPARE(config.windowState(), testState);
    }

    void testGenericValueAccess()
    {
        AppConfig config;
        QSignalSpy spy(&config, &AppConfig::configChanged);
        
        // Test setting and getting generic values
        config.setValue("test_key", QVariant("test_value"));
        QCOMPARE(config.value("test_key").toString(), QString("test_value"));
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.takeFirst().at(0).toString(), QString("test_key"));
        
        // Test default value
        QCOMPARE(config.value("nonexistent_key", "default").toString(), QString("default"));
    }

    void testLoadSave()
    {
        // Test save and load persistence
        {
            AppConfig config;
            config.setThemeMode("light");
            config.setNavExpanded(true);
            config.setNavSelectedIndex(3);
            config.save();
        }
        
        // Create new instance and load
        {
            AppConfig config2;
            config2.load();
            QCOMPARE(config2.themeMode(), QString("light"));
            QCOMPARE(config2.navExpanded(), true);
            QCOMPARE(config2.navSelectedIndex(), 3);
        }
    }

    void testReset()
    {
        AppConfig config;
        
        // Set some values
        config.setThemeMode("dark");
        config.setNavExpanded(false);
        config.setNavSelectedIndex(10);
        
        // Verify values are set
        QCOMPARE(config.themeMode(), QString("dark"));
        QCOMPARE(config.navExpanded(), false);
        QCOMPARE(config.navSelectedIndex(), 10);
        
        // Reset should restore defaults
        config.reset();
        
        // Values should be back to defaults (implementation-specific)
        QString resetTheme = config.themeMode();
        bool resetExpanded = config.navExpanded();
        int resetIndex = config.navSelectedIndex();
        
        // Verify reset changed values (at least some should be different)
        QVERIFY(resetTheme != "dark" || resetExpanded != false || resetIndex != 10);
    }

    void testRecentTabAndFormula()
    {
        AppConfig config;
        
        // Test recent tab
        config.setRecentTab("tab_123");
        QCOMPARE(config.recentTab(), QString("tab_123"));
        
        // Test recent formula
        config.setRecentFormula("formula_456");
        QCOMPARE(config.recentFormula(), QString("formula_456"));
    }
};

#include "TestAppConfig.moc"
QTEST_MAIN(TestAppConfig)