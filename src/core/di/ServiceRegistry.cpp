#include "ServiceRegistry.h"
#include "AppConfig.h"
#include "ThemeManager.h"
#include "NavViewModel.h"
#include "TabViewModel.h"
#include "FormulaViewModel.h"
#include <qapplication.h>

void ServiceRegistry::registerCoreServices()
{
    // 注册配置管理器（单例）
    auto config = std::make_shared<AppConfig>();
    config->load();  // 加载配置
    DI.registerSingleton<AppConfig>(config);
    
    // 注册主题管理器（单例）
    auto themeManager = std::make_shared<ThemeManager>();
    // 从配置中恢复主题设置
    auto mode = config->themeMode();
    if (mode == "light") {
        themeManager->setMode(ThemeManager::ThemeMode::Light);
    } else if (mode == "dark") {
        themeManager->setMode(ThemeManager::ThemeMode::Dark);
    } else {
        themeManager->setMode(ThemeManager::ThemeMode::FollowSystem);
    }
    DI.registerSingleton<ThemeManager>(themeManager);
    
    // 连接主题变化到配置保存
    QObject::connect(themeManager.get(), &ThemeManager::modeChanged,
        [config](ThemeManager::ThemeMode mode) {
            QString modeStr;
            switch (mode) {
                case ThemeManager::ThemeMode::Light: modeStr = "light"; break;
                case ThemeManager::ThemeMode::Dark: modeStr = "dark"; break;
                default: modeStr = "system"; break;
            }
            config->setThemeMode(modeStr);
        });
}

void ServiceRegistry::registerViewModels()
{
    // 导航视图模型（单例）
    auto navVm = std::make_shared<NavViewModel>();
    
    // 从配置恢复导航展开状态
    auto config = DI.get<AppConfig>();
    if (config) {
        navVm->setExpanded(config->navExpanded());
        
        // 连接导航展开变化到配置保存
        QObject::connect(navVm.get(), &NavViewModel::expandedChanged,
            [config](bool expanded) {
                config->setNavExpanded(expanded);
            });
    }
    
    DI.registerSingleton<NavViewModel>(navVm);
    
    // Tab视图模型（工厂模式，每次创建新实例）
    DI.registerFactory<TabViewModel>([]() {
        return std::make_shared<TabViewModel>();
    });
    
    // 方剂视图模型（工厂模式）
    DI.registerFactory<FormulaViewModel>([]() {
        return std::make_shared<FormulaViewModel>();
    });
}

void ServiceRegistry::registerAll()
{
    registerCoreServices();
    registerViewModels();
}

void ServiceRegistry::cleanup()
{
    // 保存配置
    if (auto config = DI.get<AppConfig>()) {
        config->save();
    }
    
    // 清理所有服务
    DI.clear();
}