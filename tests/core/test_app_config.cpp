#include <QtTest>
#include <QTemporaryFile>
#include <QSignalSpy>
#include "../../src/core/config/AppConfig.h"

class TestAppConfig : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "Initializing AppConfig tests...";
        // 使用临时设置文件
        QCoreApplication::setOrganizationName("FangjiaTest");
        QCoreApplication::setApplicationName("ConfigTest");
    }
    
    void cleanupTestCase() {
        Config::AppConfig::instance()->clear();
        qDebug() << "AppConfig tests completed.";
    }
    
    void init() {
        Config::AppConfig::instance()->clear();
    }
    
    void testBasicReadWrite() {
        auto config = Config::AppConfig::instance();
        
        // 写入值
        config->setValue("test/key1", 42);
        config->setValue("test/key2", "hello");
        config->setValue("test/key3", true);
        
        // 读取值
        QCOMPARE(config->value("test/key1").toInt(), 42);
        QCOMPARE(config->value("test/key2").toString(), QString("hello"));
        QCOMPARE(config->value("test/key3").toBool(), true);
    }
    
    void testDefaultValues() {
        auto config = Config::AppConfig::instance();
        
        // 测试默认值
        QCOMPARE(config->value("nonexistent", 100).toInt(), 100);
        QCOMPARE(config->value("nonexistent", "default").toString(), QString("default"));
    }
    
    void testTypedAccessors() {
        auto config = Config::AppConfig::instance();
        
        // 测试类型安全的访问器
        config->setWindowWidth(1920);
        config->setWindowHeight(1080);
        config->setNavExpanded(true);
        config->setAnimationSpeed(2.0f);
        
        QCOMPARE(config->windowWidth(), 1920);
        QCOMPARE(config->windowHeight(), 1080);
        QCOMPARE(config->navExpanded(), true);
        QVERIFY(qFuzzyCompare(config->animationSpeed(), 2.0f));
    }
    
    void testSignals() {
        auto config = Config::AppConfig::instance();
        QSignalSpy spy(config, &Config::AppConfig::valueChanged);
        
        config->setValue("test/signal", 123);
        
        QCOMPARE(spy.count(), 1);
        QList<QVariant> arguments = spy.takeFirst();
        QCOMPARE(arguments.at(0).toString(), QString("test/signal"));
        QCOMPARE(arguments.at(1).toInt(), 123);
    }
    
    void testGroups() {
        auto config = Config::AppConfig::instance();
        
        config->beginGroup("window");
        config->setValue("width", 800);
        config->setValue("height", 600);
        config->endGroup();
        
        QCOMPARE(config->value("window/width").toInt(), 800);
        QCOMPARE(config->value("window/height").toInt(), 600);
    }
    
    void testExportImport() {
        auto config = Config::AppConfig::instance();
        
        // 设置一些值
        config->setValue("export/test1", 111);
        config->setValue("export/test2", "exported");
        
        // 导出到临时文件
        QTemporaryFile tempFile;
        QVERIFY(tempFile.open());
        QString filePath = tempFile.fileName();
        tempFile.close();
        
        QVERIFY(config->exportToFile(filePath));
        
        // 清空配置
        config->clear();
        QVERIFY(!config->value("export/test1").isValid());
        
        // 导入配置
        QVERIFY(config->importFromFile(filePath));
        
        // 验证导入的值
        QCOMPARE(config->value("export/test1").toInt(), 111);
        QCOMPARE(config->value("export/test2").toString(), QString("exported"));
    }
    
    void testConfigBinding() {
        Config::ConfigBinding<int> binding("test/binding", 50);
        QSignalSpy spy(&binding, &Config::ConfigBinding<int>::changed);
        
        QCOMPARE(binding.value(), 50); // 默认值
        
        binding.setValue(75);
        QCOMPARE(binding.value(), 75);
        
        QCOMPARE(spy.count(), 1);
        QCOMPARE(spy.at(0).at(0).toInt(), 75);
    }
};