/*
 * 文件名：FangjiaApp.cpp
 * 职责：房价应用程序类实现。
 */

#include "FangjiaApp.hpp"
#include "MainOpenGlWindow.hpp"
#include "data/sources/local/AppConfig.h"
#include "presentation/viewmodels/ThemeManager.h"
#include "apps/fangjia/CompositionRoot.h"
#include "apps/fangjia/UnifiedDependencyProvider.h"
#include "apps/fangjia/DependencyMigrationTool.h"
#include "apps/fangjia/UnifiedDIUsageExample.h"
#include <qstringliteral.h>
#include <qlogging.h>
#include <qobject.h>
#include <qbytearray.h>
#include <exception>

FangjiaApp::FangjiaApp(int argc, char* argv[])
    : fj::presentation::ui::base::Application(argc, argv,
        QStringLiteral("TaiGongZhaiHua"),
        QStringLiteral("Fangjia.com"), 
        QStringLiteral("Fangjia"))
{
}

FangjiaApp::~FangjiaApp() = default;

void FangjiaApp::configureApplicationInfo()
{
    // 调用基类实现来设置组织和应用信息
    fj::presentation::ui::base::Application::configureApplicationInfo();
    
    qDebug() << "FangjiaApp: Application info configured for Fangjia";
}

bool FangjiaApp::initializeApplication()
{
    try {
        qDebug() << "FangjiaApp: Initializing business application";
        
        // 初始化依赖注入
        if (!initializeDependencyInjection()) {
            return false;
        }
        
        // 初始化配置
        if (!initializeConfiguration()) {
            return false;
        }
        
        // 初始化主题管理
        if (!initializeThemeManagement()) {
            return false;
        }
        
        qDebug() << "FangjiaApp: Business application initialization completed";
        return true;
    }
    catch (const std::exception& e) {
        qCritical() << "FangjiaApp: Exception during initialization:" << e.what();
        return false;
    }
}

bool FangjiaApp::createAndShowMainWindow()
{
    try {
        qDebug() << "FangjiaApp: Creating main window";
        
        // 创建主窗口并注入依赖
        m_mainWindow = std::make_unique<MainOpenGlWindow>(m_config, m_themeManager);
        
        // 从配置恢复窗口几何
        QByteArray geo = m_config->windowGeometry();
        if (!geo.isEmpty() && geo.size() == sizeof(int) * 4) {
            const auto data = reinterpret_cast<const int*>(geo.data());
            m_mainWindow->setPosition(data[0], data[1]);
            m_mainWindow->resize(data[2], data[3]);
        } else {
            m_mainWindow->resize(1200, 760);
        }
        
        qDebug() << "FangjiaApp: Showing main window";
        m_mainWindow->show();
        
        return true;
    }
    catch (const std::exception& e) {
        qCritical() << "FangjiaApp: Exception creating main window:" << e.what();
        return false;
    }
}

void FangjiaApp::cleanupApplication()
{
    qDebug() << "FangjiaApp: Cleaning up business application";
    
    // 保存配置
    if (m_config) {
        m_config->save();
    }
    
    // 清理主窗口
    m_mainWindow.reset();
    
    // 调用基类清理
    fj::presentation::ui::base::Application::cleanupApplication();
}

bool FangjiaApp::initializeDependencyInjection()
{
    try {
        qDebug() << "FangjiaApp: Initializing dependency injection";
        
        // === Pure Boost.DI Configuration (Phase 4 Complete) ===
        // All services are now managed through CompositionRoot and Boost.DI
        auto& unifiedDeps = UnifiedDependencyProvider::instance();
        
        qDebug() << "FangjiaApp: Pure Boost.DI Dependency Provider initialized successfully";
        
        // === Migration Status Report ===
        // Phase 4 Complete: All services migrated to pure Boost.DI
        auto& migrationTool = DependencyMigrationTool::instance();
        auto migrationReport = migrationTool.generateMigrationReport();
        
        qDebug() << "FangjiaApp: DI Migration Status: Phase 4 Complete!" 
                 << migrationReport.migratedServices << "/" << migrationReport.totalServices 
                 << "services migrated (" << migrationReport.completionPercentage << "%)";

        // === Demonstrate Pure Boost.DI Usage ===
        // 展示纯Boost.DI依赖注入的使用模式
        UnifiedDIUsageExample example;
        example.demonstrateUnifiedAccess();
        example.demonstrateViewModelUsage();
        
        return true;
    }
    catch (const std::exception& e) {
        qCritical() << "FangjiaApp: Exception initializing DI:" << e.what();
        return false;
    }
}

bool FangjiaApp::initializeConfiguration()
{
    try {
        qDebug() << "FangjiaApp: Creating and loading configuration";
        
        // 创建配置管理器并加载持久化设置
        m_config = std::make_shared<AppConfig>();
        m_config->load();
        
        return true;
    }
    catch (const std::exception& e) {
        qCritical() << "FangjiaApp: Exception initializing configuration:" << e.what();
        return false;
    }
}

bool FangjiaApp::initializeThemeManagement()
{
    try {
        qDebug() << "FangjiaApp: Initializing theme management";
        
        // === Get services from Boost.DI ===
        auto& unifiedDeps = UnifiedDependencyProvider::instance();
        auto getThemeModeUseCase = unifiedDeps.get<domain::usecases::GetThemeModeUseCase>();
        auto setThemeModeUseCase = unifiedDeps.get<domain::usecases::SetThemeModeUseCase>();
        
        // 创建主题管理器（使用纯Boost.DI获取的服务）
        m_themeManager = std::make_shared<ThemeManager>(getThemeModeUseCase, setThemeModeUseCase);
        
        // 从设置中加载主题状态
        m_themeManager->load();

        // 连接主题变化信号到配置持久化（通过ThemeManager内置的save方法）
        QObject::connect(m_themeManager.get(), &ThemeManager::modeChanged,
            [this](const ThemeManager::ThemeMode) {
                m_themeManager->save();
            });
        
        return true;
    }
    catch (const std::exception& e) {
        qCritical() << "FangjiaApp: Exception initializing theme management:" << e.what();
        return false;
    }
}