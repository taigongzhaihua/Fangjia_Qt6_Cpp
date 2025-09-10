/*
 * 文件名：Application.cpp
 * 职责：基础应用程序类实现，提供应用程序生命周期管理。
 */

#include "Application.hpp"
#include <qcoreapplication.h>
#include <qlogging.h>
#include <qstringliteral.h>
#include <exception>

namespace fj::presentation::ui::base {

Application::Application(int argc, char* argv[],
                        const QString& orgName,
                        const QString& orgDomain,
                        const QString& appName)
    : m_organizationName(orgName)
    , m_organizationDomain(orgDomain)
    , m_applicationName(appName)
    , m_argc(argc)
    , m_argv(argv)
{
    // 配置OpenGL默认格式
    setupDefaultOpenGLFormat();
    
    // 创建QApplication实例
    m_qapp = std::make_unique<QApplication>(argc, argv);
    
    // 配置应用程序信息
    configureApplicationInfo();
}

Application::~Application()
{
    cleanupApplication();
}

int Application::run()
{
    try {
        qDebug() << "Application: Starting application run sequence";
        
        // 配置OpenGL
        configureOpenGL();
        
        // 初始化应用程序
        if (!initializeApplication()) {
            qCritical() << "Application: Failed to initialize application";
            return -1;
        }
        
        // 创建并显示主窗口
        if (!createAndShowMainWindow()) {
            qCritical() << "Application: Failed to create main window";
            return -1;
        }
        
        qDebug() << "Application: Starting Qt event loop";
        
        // 运行Qt事件循环
        const int result = QApplication::exec();
        
        qDebug() << "Application: Qt event loop finished with result:" << result;
        
        return result;
    }
    catch (const std::exception& e) {
        qCritical() << "Application: Exception during run:" << e.what();
        return -1;
    }
    catch (...) {
        qCritical() << "Application: Unknown exception during run";
        return -1;
    }
}

void Application::configureOpenGL()
{
    // 子类可重写以设置特定的OpenGL配置
    // 基类默认实现使用setupDefaultOpenGLFormat设置的格式
    qDebug() << "Application: Using default OpenGL configuration";
}

void Application::configureApplicationInfo()
{
    if (!m_organizationName.isEmpty()) {
        QCoreApplication::setOrganizationName(m_organizationName);
    }
    
    if (!m_organizationDomain.isEmpty()) {
        QCoreApplication::setOrganizationDomain(m_organizationDomain);
    }
    
    if (!m_applicationName.isEmpty()) {
        QCoreApplication::setApplicationName(m_applicationName);
    }
    
    qDebug() << "Application: Configured application info -"
             << "Org:" << QCoreApplication::organizationName()
             << "Domain:" << QCoreApplication::organizationDomain()
             << "App:" << QCoreApplication::applicationName();
}

void Application::cleanupApplication()
{
    // 基类默认清理实现 - 子类可重写以添加特定清理逻辑
    qDebug() << "Application: Base cleanup completed";
}

void Application::setupDefaultOpenGLFormat()
{
    // 设置默认的OpenGL上下文参数
    QSurfaceFormat fmt;
    fmt.setDepthBufferSize(24);
    fmt.setStencilBufferSize(16);
    fmt.setVersion(3, 3);
    fmt.setProfile(QSurfaceFormat::CoreProfile);
    QSurfaceFormat::setDefaultFormat(fmt);
    
    qDebug() << "Application: Default OpenGL format configured (3.3 Core Profile)";
}

} // namespace fj::presentation::ui::base