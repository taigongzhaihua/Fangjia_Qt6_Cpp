#include <QtTest>
#include <memory>
#include "PageRouter.h"
#include "UiPage.h"

// 简单的测试页面类
class TestPage : public UiPage
{
public:
    TestPage(const QString& name) : m_name(name) {
        setTitle(name);
    }
    
    bool appeared{false};
    bool disappeared{false};
    
    void onAppear() override {
        appeared = true;
        qDebug() << m_name << "onAppear called";
    }
    
    void onDisappear() override {
        disappeared = true;
        qDebug() << m_name << "onDisappear called";
    }

private:
    QString m_name;
};

class TestPageRouter : public QObject
{
    Q_OBJECT

private slots:
    void testFactoryRegistration()
    {
        PageRouter router;
        
        // 注册页面工厂
        router.registerPage("test1", []() { return std::make_unique<TestPage>("TestPage1"); });
        router.registerPage("test2", []() { return std::make_unique<TestPage>("TestPage2"); });
        
        // 懒加载：第一次获取时创建
        UiPage* page1 = router.getPage("test1");
        QVERIFY(page1 != nullptr);
        QCOMPARE(page1->title(), QString("TestPage1"));
        
        // 第二次获取应该返回缓存的实例
        UiPage* page1_again = router.getPage("test1");
        QCOMPARE(page1, page1_again);
    }
    
    void testLifecycleHooks()
    {
        PageRouter router;
        
        router.registerPage("page1", []() { return std::make_unique<TestPage>("Page1"); });
        router.registerPage("page2", []() { return std::make_unique<TestPage>("Page2"); });
        
        // 切换到第一个页面
        QVERIFY(router.switchToPage("page1"));
        TestPage* testPage1 = static_cast<TestPage*>(router.currentPage());
        QVERIFY(testPage1->appeared);
        QVERIFY(!testPage1->disappeared);
        
        // 切换到第二个页面
        QVERIFY(router.switchToPage("page2"));
        TestPage* testPage2 = static_cast<TestPage*>(router.currentPage());
        
        // 第一个页面应该调用了 onDisappear
        QVERIFY(testPage1->disappeared);
        
        // 第二个页面应该调用了 onAppear
        QVERIFY(testPage2->appeared);
        QVERIFY(!testPage2->disappeared);
    }
    
    void testNonExistentPage()
    {
        PageRouter router;
        
        // 获取不存在的页面应该返回 nullptr
        UiPage* page = router.getPage("nonexistent");
        QVERIFY(page == nullptr);
        
        // 切换到不存在的页面应该失败
        QVERIFY(!router.switchToPage("nonexistent"));
    }
};

QTEST_MAIN(TestPageRouter)
#include "test_page_router.moc"