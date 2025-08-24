#include <QtTest>
#include <memory>
#include "../../src/core/di/ServiceLocator.h"

// 测试用的服务类
class ITestService {
public:
    virtual ~ITestService() = default;
    virtual int getValue() const = 0;
};

class TestServiceImpl : public ITestService {
public:
    explicit TestServiceImpl(int v) : m_value(v) {}
    int getValue() const override { return m_value; }
private:
    int m_value;
};

class TestServiceLocator : public QObject
{
    Q_OBJECT

private slots:
    void initTestCase() {
        qDebug() << "Initializing ServiceLocator tests...";
        DI::ServiceLocator::instance().clear();
    }
    
    void cleanupTestCase() {
        DI::ServiceLocator::instance().clear();
        qDebug() << "ServiceLocator tests completed.";
    }
    
    void init() {
        // 每个测试前清理
        DI::ServiceLocator::instance().clear();
    }
    
    void testSingletonRegistration() {
        // 注册单例服务
        auto service = std::make_shared<TestServiceImpl>(42);
        DI::registerService<ITestService>(service);
        
        // 获取服务
        auto retrieved1 = DI::getService<ITestService>();
        auto retrieved2 = DI::getService<ITestService>();
        
        QVERIFY(retrieved1 != nullptr);
        QVERIFY(retrieved2 != nullptr);
        QCOMPARE(retrieved1->getValue(), 42);
        QVERIFY(retrieved1 == retrieved2); // 应该是同一实例
    }
    
    void testFactoryRegistration() {
        int counter = 0;
        
        // 注册工厂
        DI::ServiceLocator::instance().registerFactory<ITestService>(
            [&counter]() -> std::shared_ptr<ITestService> {
                return std::make_shared<TestServiceImpl>(++counter);
            }
        );
        
        // 第一次获取会创建实例
        auto first = DI::getService<ITestService>();
        QVERIFY(first != nullptr);
        QCOMPARE(first->getValue(), 1);
        
        // 第二次获取应该返回缓存的实例
        auto second = DI::getService<ITestService>();
        QVERIFY(second == first);
        QCOMPARE(counter, 1); // 工厂只应被调用一次
    }
    
    void testNonExistentService() {
        auto service = DI::getService<ITestService>();
        QVERIFY(service == nullptr);
    }
    
    void testServiceReplacement() {
        // 注册第一个服务
        auto service1 = std::make_shared<TestServiceImpl>(10);
        DI::registerService<ITestService>(service1);
        QCOMPARE(DI::getService<ITestService>()->getValue(), 10);
        
        // 注册第二个服务（应该替换第一个）
        auto service2 = std::make_shared<TestServiceImpl>(20);
        DI::registerService<ITestService>(service2);
        QCOMPARE(DI::getService<ITestService>()->getValue(), 20);
    }
    
    void testThreadSafety() {
        // 测试并发访问
        const int threadCount = 10;
        const int iterations = 100;
        QVector<std::thread> threads;
        std::atomic<int> successCount{0};
        
        auto service = std::make_shared<TestServiceImpl>(99);
        DI::registerService<ITestService>(service);
        
        for (int i = 0; i < threadCount; ++i) {
            threads.emplace_back([&successCount, iterations]() {
                for (int j = 0; j < iterations; ++j) {
                    auto s = DI::getService<ITestService>();
                    if (s && s->getValue() == 99) {
                        successCount++;
                    }
                }
            });
        }
        
        for (auto& t : threads) {
            t.join();
        }
        
        QCOMPARE(successCount, threadCount * iterations);
    }
};