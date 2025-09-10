#include <iostream>
#include <cassert>
#include <memory>

// 引入核心测试目标
#include "core/interfaces/IComponent.h"
#include "core/interfaces/IDependencyContainer.h"
#include "core/base/BaseComponent.h"
#include "core/di/SimpleDependencyContainer.h"

using namespace Fangjia::Core;

class TestComponent : public BaseComponent {
public:
    bool initialized = false;
    bool activated = false;
    bool themeChanged = false;

protected:
    void onInitialize() override { initialized = true; }
    void onActivate() override { activated = true; }
    void onThemeChanged(bool) override { themeChanged = true; }
};

class ITestService {
public:
    virtual ~ITestService() = default;
    virtual int getValue() = 0;
};

class TestService : public ITestService {
public:
    int getValue() override { return 42; }
};

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "Fangjia Core Architecture Tests" << std::endl;
    std::cout << "============================================" << std::endl;

    int passed = 0;
    int total = 0;

    // 测试1: 组件生命周期
    total++;
    try {
        auto component = std::make_shared<TestComponent>();
        assert(!component->isInitialized());
        assert(!component->isActive());
        
        component->initialize();
        assert(component->isInitialized());
        assert(!component->isActive());
        
        component->activate();
        assert(component->isActive());
        
        component->applyTheme(true);
        assert(component->initialized);
        assert(component->activated); 
        assert(component->themeChanged);
        
        passed++;
        std::cout << "✅ 组件生命周期测试通过" << std::endl;
    } catch (...) {
        std::cout << "❌ 组件生命周期测试失败" << std::endl;
    }

    // 测试2: 依赖注入容器
    total++;
    try {
        auto container = std::make_shared<SimpleDependencyContainer>();
        container->registerSingleton<ITestService, TestService>();
        
        assert(container->isRegistered<ITestService>());
        assert(container->getRegisteredCount() == 1);
        
        auto service1 = container->resolve<ITestService>();
        auto service2 = container->resolve<ITestService>();
        
        assert(service1 != nullptr);
        assert(service2 != nullptr);
        assert(service1.get() == service2.get()); // 单例模式
        assert(service1->getValue() == 42);
        
        passed++;
        std::cout << "✅ 依赖注入容器测试通过" << std::endl;
    } catch (...) {
        std::cout << "❌ 依赖注入容器测试失败" << std::endl;
    }

    // 测试3: 全局服务定位器
    total++;
    try {
        auto container = std::make_shared<SimpleDependencyContainer>();
        GlobalServiceLocator::instance().setContainer(container);
        
        GlobalServiceLocator::instance().registerSingleton<ITestService, TestService>();
        
        assert(GlobalServiceLocator::instance().has<ITestService>());
        
        auto service = GlobalServiceLocator::instance().get<ITestService>();
        assert(service != nullptr);
        assert(service->getValue() == 42);
        
        passed++;
        std::cout << "✅ 全局服务定位器测试通过" << std::endl;
    } catch (...) {
        std::cout << "❌ 全局服务定位器测试失败" << std::endl;
    }

    std::cout << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << "测试结果: " << passed << "/" << total << " 通过" << std::endl;
    
    if (passed == total) {
        std::cout << "🎉 所有核心架构测试通过！" << std::endl;
        std::cout << "✅ 基于PR #21设计原则的核心架构实现成功" << std::endl;
        return 0;
    } else {
        std::cout << "❌ 部分测试失败" << std::endl;
        return 1;
    }
}