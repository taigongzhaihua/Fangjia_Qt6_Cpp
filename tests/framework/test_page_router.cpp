#include <QtTest/QtTest>
#include "PageRouter.h"
#include "UiPage.h"

// 简单的测试页面类
class TestPage : public UiPage
{
public:
    TestPage(const QString& testId = "test") : m_testId(testId) {
        setTitle("Test Page: " + testId);
    }
    
    QString testId() const { return m_testId; }
    
protected:
    void initializeContent() override {
        // 简单的测试内容初始化
    }
    
private:
    QString m_testId;
};

class TestPageRouter : public QObject
{
    Q_OBJECT

private slots:
    void testBasicFunctionality();
    void testLazyCreation();
    void testPageCaching();
    void testFactoryRegistration();
    void testErrorHandling();
};

void TestPageRouter::testBasicFunctionality()
{
    PageRouter router;
    
    // 测试初始状态
    QVERIFY(router.currentPage() == nullptr);
    QVERIFY(router.currentPageId().isEmpty());
    
    // 注册页面工厂
    router.registerPageFactory("test1", []() { return std::make_unique<TestPage>("test1"); });
    
    // 验证工厂注册
    QVERIFY(router.hasPageFactory("test1"));
    QVERIFY(!router.hasPageFactory("nonexistent"));
    
    // 验证页面尚未创建
    QVERIFY(!router.isPageCreated("test1"));
}

void TestPageRouter::testLazyCreation()
{
    PageRouter router;
    
    // 注册页面工厂
    router.registerPageFactory("lazy", []() { return std::make_unique<TestPage>("lazy"); });
    
    // 确认页面尚未创建
    QVERIFY(!router.isPageCreated("lazy"));
    
    // 切换到页面，应该触发创建
    QVERIFY(router.switchToPage("lazy"));
    
    // 确认页面已创建并成为当前页面
    QVERIFY(router.isPageCreated("lazy"));
    QVERIFY(router.currentPage() != nullptr);
    QCOMPARE(router.currentPageId(), QString("lazy"));
    
    // 验证页面内容正确
    TestPage* testPage = dynamic_cast<TestPage*>(router.currentPage());
    QVERIFY(testPage != nullptr);
    QCOMPARE(testPage->testId(), QString("lazy"));
}

void TestPageRouter::testPageCaching()
{
    PageRouter router;
    
    // 注册页面工厂
    router.registerPageFactory("cached", []() { return std::make_unique<TestPage>("cached"); });
    
    // 第一次获取页面
    UiPage* page1 = router.getPage("cached");
    QVERIFY(page1 != nullptr);
    QVERIFY(router.isPageCreated("cached"));
    
    // 第二次获取应该返回相同实例（缓存）
    UiPage* page2 = router.getPage("cached");
    QVERIFY(page2 != nullptr);
    QCOMPARE(page1, page2); // 应该是同一个实例
}

void TestPageRouter::testFactoryRegistration()
{
    PageRouter router;
    
    // 注册多个页面工厂
    router.registerPageFactory("page1", []() { return std::make_unique<TestPage>("page1"); });
    router.registerPageFactory("page2", []() { return std::make_unique<TestPage>("page2"); });
    router.registerPageFactory("page3", []() { return std::make_unique<TestPage>("page3"); });
    
    // 验证所有工厂都已注册
    QVERIFY(router.hasPageFactory("page1"));
    QVERIFY(router.hasPageFactory("page2"));
    QVERIFY(router.hasPageFactory("page3"));
    
    // 验证页面尚未创建
    QVERIFY(!router.isPageCreated("page1"));
    QVERIFY(!router.isPageCreated("page2"));
    QVERIFY(!router.isPageCreated("page3"));
    
    // 分别切换到不同页面
    QVERIFY(router.switchToPage("page2"));
    QCOMPARE(router.currentPageId(), QString("page2"));
    QVERIFY(router.isPageCreated("page2"));
    QVERIFY(!router.isPageCreated("page1")); // page1仍未创建
    
    QVERIFY(router.switchToPage("page1"));
    QCOMPARE(router.currentPageId(), QString("page1"));
    QVERIFY(router.isPageCreated("page1"));
    QVERIFY(router.isPageCreated("page2")); // page2保持缓存
}

void TestPageRouter::testErrorHandling()
{
    PageRouter router;
    
    // 测试切换到不存在的页面
    QVERIFY(!router.switchToPage("nonexistent"));
    QVERIFY(router.currentPage() == nullptr);
    
    // 测试获取不存在的页面
    QVERIFY(router.getPage("nonexistent") == nullptr);
    
    // 测试注册空工厂
    router.registerPageFactory("null_factory", nullptr);
    QVERIFY(!router.hasPageFactory("null_factory")); // 应该拒绝空工厂
    
    // 测试清理功能
    router.registerPageFactory("temp", []() { return std::make_unique<TestPage>("temp"); });
    router.switchToPage("temp");
    QVERIFY(router.currentPage() != nullptr);
    
    router.clear();
    QVERIFY(router.currentPage() == nullptr);
    QVERIFY(router.currentPageId().isEmpty());
    QVERIFY(!router.hasPageFactory("temp"));
    QVERIFY(!router.isPageCreated("temp"));
}

QTEST_MAIN(TestPageRouter)
#include "test_page_router.moc"