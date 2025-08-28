#include <QtTest>
#include <QCoreApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QCoreApplication app(argc, argv);
    
    // Set application info
    app.setOrganizationName("Fangjia");
    app.setOrganizationDomain("fangjia.test");
    app.setApplicationName("Fangjia_Tests");
    
    // Set Qt test environment
    qputenv("QT_QPA_PLATFORM", "offscreen");
    qputenv("QT_LOGGING_RULES", "qt.qpa.gl=false");
    
    qDebug() << "===========================================";
    qDebug() << "Fangjia Binding Test";
    qDebug() << "===========================================";
    
    int status = 0;
    
    // Include test implementation first
    #include "framework/test_binding_host.cpp"
    
    // Run binding test
    {
        TestBindingHost test;
        status = QTest::qExec(&test, argc, argv);
    }
    
    if (status == 0) {
        qDebug() << "Binding test PASSED ✅";
    } else {
        qDebug() << "Binding test FAILED ❌";
    }
    
    return status;
}