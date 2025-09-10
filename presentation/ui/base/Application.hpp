/*
 * 文件名：Application.hpp
 * 职责：基础应用程序类，提供应用程序生命周期管理和公共配置，增加封装性。
 * 依赖：Qt6 Core/Gui/Widgets。
 * 线程：仅在主线程使用。
 * 备注：所有应用程序的基类，封装了Qt应用程序的标准初始化和配置。
 */

#pragma once
#include <memory>
#include <qapplication.h>
#include <qstring.h>
#include <qsurfaceformat.h>

namespace fj::presentation::ui::base {

/// 基础应用程序类：封装Qt应用程序的通用功能
/// 
/// 功能：
/// - Qt应用程序生命周期管理
/// - OpenGL上下文全局配置
/// - 应用程序元信息设置
/// - 标准初始化流程
/// 
/// 设计原则：
/// - 提供通用应用程序功能的基础实现
/// - 子类通过虚函数定制特定配置和行为
/// - 避免业务逻辑耦合，保持框架层纯净性
class Application
{
public:
    /// 构造函数：初始化基础应用程序
    /// 参数：argc, argv — 命令行参数
    /// 参数：orgName — 组织名称
    /// 参数：orgDomain — 组织域名
    /// 参数：appName — 应用程序名称
    explicit Application(int argc, char* argv[],
                        const QString& orgName = QString(),
                        const QString& orgDomain = QString(),
                        const QString& appName = QString());
    
    virtual ~Application();

    /// 功能：运行应用程序主循环
    /// 返回：应用程序退出码
    virtual int run();

    /// 功能：获取QApplication实例
    /// 返回：QApplication指针
    QApplication* qapp() const { return m_qapp.get(); }

protected:
    /// 虚函数接口 - 子类可重写以定制行为
    
    /// 功能：配置OpenGL上下文格式
    /// 说明：子类可重写以设置特定的OpenGL版本和配置
    virtual void configureOpenGL();
    
    /// 功能：设置应用程序元信息
    /// 说明：子类可重写以设置特定的组织信息和应用名称
    virtual void configureApplicationInfo();
    
    /// 功能：初始化应用程序特定的资源和服务
    /// 返回：初始化是否成功
    virtual bool initializeApplication() = 0;
    
    /// 功能：清理应用程序资源
    virtual void cleanupApplication();
    
    /// 功能：创建并显示主窗口
    /// 返回：是否成功创建窗口
    virtual bool createAndShowMainWindow() = 0;

protected:
    // 应用程序基础信息
    QString m_organizationName;
    QString m_organizationDomain;
    QString m_applicationName;
    
    // Qt应用程序实例
    std::unique_ptr<QApplication> m_qapp;
    
    // 命令行参数
    int m_argc;
    char** m_argv;

private:
    void setupDefaultOpenGLFormat();
};

} // namespace fj::presentation::ui::base