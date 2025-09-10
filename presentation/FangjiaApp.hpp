/*
 * 文件名：FangjiaApp.hpp
 * 职责：房价应用程序类，派生自基础Application类，实现业务特定的应用程序逻辑。
 * 依赖：基础Application类、配置管理、主题管理、依赖注入。
 * 线程：仅在主线程使用。
 * 备注：业务层的应用程序实现，负责初始化业务服务和创建主窗口。
 */

#pragma once
#include "ui/base/Application.hpp"
#include <memory>

// 前向声明
class AppConfig;
class ThemeManager;
class MainOpenGlWindow;

namespace domain::usecases {
    class GetThemeModeUseCase;
    class SetThemeModeUseCase;
}

/// 房价应用程序：业务特定的应用程序实现
/// 
/// 功能：
/// - 初始化依赖注入容器和服务
/// - 配置业务特定的应用程序信息
/// - 创建和管理主窗口
/// - 处理应用程序生命周期事件
class FangjiaApp : public fj::presentation::ui::base::Application
{
public:
    /// 构造函数：初始化房价应用程序
    /// 参数：argc, argv — 命令行参数
    explicit FangjiaApp(int argc, char* argv[]);
    ~FangjiaApp() override;

protected:
    /// 应用程序生命周期重写

    /// 功能：配置应用程序元信息
    void configureApplicationInfo() override;
    
    /// 功能：初始化业务服务和依赖注入
    /// 返回：初始化是否成功
    bool initializeApplication() override;
    
    /// 功能：创建并显示主窗口
    /// 返回：是否成功创建窗口
    bool createAndShowMainWindow() override;
    
    /// 功能：清理业务资源
    void cleanupApplication() override;

private:
    // 业务服务
    std::shared_ptr<AppConfig> m_config;
    std::shared_ptr<ThemeManager> m_themeManager;
    
    // 主窗口
    std::unique_ptr<MainOpenGlWindow> m_mainWindow;
    
    // 业务初始化方法
    bool initializeDependencyInjection();
    bool initializeConfiguration();
    bool initializeThemeManagement();
};