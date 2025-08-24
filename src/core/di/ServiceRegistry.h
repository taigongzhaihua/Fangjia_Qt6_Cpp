#pragma once
#include "ServiceLocator.h"
#include <memory>

// 前向声明
class AppConfig;
class ThemeManager;
class NavViewModel;
class TabViewModel;
class FormulaViewModel;

// 服务注册器
class ServiceRegistry
{
public:
    // 注册所有核心服务
    static void registerCoreServices();
    
    // 注册视图模型
    static void registerViewModels();
    
    // 注册所有服务
    static void registerAll();
    
    // 清理
    static void cleanup();

private:
    ServiceRegistry() = default;
};