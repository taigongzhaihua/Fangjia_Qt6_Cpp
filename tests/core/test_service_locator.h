#pragma once
#include <QtTest>
#include "../../src/core/di/ServiceLocator.h"

class TestService {
public:
    int value;
    explicit TestService(int v) : value(v) {}
};

class TestServiceLocator : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        DI::ServiceLocator::instance().clear();
    }

    void cleanupTestCase() {
        DI::ServiceLocator::instance().clear();
    }

    void testRegisterAndGetSingleton() {
        auto service = std::make_shared<TestService>(42);
        DI::registerService(service);
        
        auto retrieved = DI::getService<TestService>();
        QVERIFY(retrieved != nullptr);
        QCOMPARE(retrieved->value, 42);
        QVERIFY(retrieved == service); // 同一实例
    }

    void testRegisterFactory() {
        int counter = 0;
        DI::ServiceLocator::instance().registerFactory<TestService>(
            [&counter]() { return std::make_shared<TestService>(++counter); }
        );
        
        auto first = DI::getService<TestService>();
        auto second = DI::getService<TestService>();
        
        QVERIFY(first != nullptr);
        QVERIFY(second != nullptr);
        QVERIFY(first == second); // 工厂结果被缓存
        QCOMPARE(first->value, 1);
    }

    void testGetNonExistentService() {
        auto service = DI::getService<TestService>();
        QVERIFY(service == nullptr);
    }
};