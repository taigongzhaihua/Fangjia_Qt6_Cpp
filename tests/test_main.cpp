#include <QtTest>
#include <QCoreApplication>
#include <QDebug>

// 测试运行器宏
#define RUN_TEST(TestClass) \
    { \
        TestClass test; \
        status |= QTest::qExec(&test, argc, argv); \
        qDebug() << "Test" << #TestClass << "completed"; \
    }

// 前向声明所有测试类
class TestServiceLocator;
class TestAppConfig;
class TestRenderer;
class TestNavViewModel;
class TestTabViewModel;
class TestFormulaViewModel;
class TestUiComponents;
class TestBoxLayout;
class TestAnimations;
class TestIntegration;

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // 设置应用信息
    app.setOrganizationName("Fangjia");
    app.setOrganizationDomain("fangjia.test");
    app.setApplicationName("Fangjia_Tests");
    
    // 设置Qt测试环境
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_LOGGING_RULES", "qt.qpa.gl=false");
    
    qDebug() << "===========================================";
    qDebug() << "Fangjia Qt6 Test Suite";
    qDebug() << "Date:" << QDateTime::currentDateTime().toString();
    qDebug() << "User:" << qgetenv("USER");
    qDebug() << "===========================================";
    
    int status = 0;
    
    // 运行核心层测试
    qDebug() << "\n[Core Layer Tests]";
    RUN_TEST(TestServiceLocator);
    RUN_TEST(TestAppConfig);
    RUN_TEST(TestRenderer);
    
    // 运行模型层测试
    qDebug() << "\n[Model Layer Tests]";
    RUN_TEST(TestNavViewModel);
    RUN_TEST(TestTabViewModel);
    RUN_TEST(TestFormulaViewModel);
    
    // 运行框架层测试
    qDebug() << "\n[Framework Layer Tests]";
    RUN_TEST(TestUiComponents);
    RUN_TEST(TestBoxLayout);
    RUN_TEST(TestAnimations);
    
    // 运行集成测试
    qDebug() << "\n[Integration Tests]";
    RUN_TEST(TestIntegration);
    
    // 输出测试结果
    qDebug() << "\n===========================================";
    if (status == 0) {
        qDebug() << "All tests PASSED ✅";
    } else {
        qDebug() << "Some tests FAILED ❌";
    }
    qDebug() << "===========================================";
    
    return status;
}

// 包含所有测试实现文件
#include "core/test_service_locator.cpp"
#include "core/test_app_config.cpp"
#include "core/test_renderer.cpp"
#include "models/test_nav_viewmodel.cpp"
#include "models/test_tab_viewmodel.cpp"
#include "models/test_formula_viewmodel.cpp"
#include "framework/test_ui_components.cpp"
#include "framework/test_box_layout.cpp"
#include "framework/test_animations.cpp"
#include "integration/test_integration.cpp"