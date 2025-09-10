#include <iostream>
#include <memory>

// 引入核心接口和实现
#include "core/interfaces/IComponent.h"
#include "core/interfaces/IDependencyContainer.h"
#include "core/base/BaseComponent.h"
#include "core/di/SimpleDependencyContainer.h"

using namespace Fangjia::Core;

/**
 * @brief 演示组件实现
 * 基于PR #21的设计原则实现的简单组件
 */
class DemoComponent : public BaseComponent {
public:
    DemoComponent(const std::string& name) : m_name(name) {}

protected:
    void onInitialize() override {
        std::cout << "[" << m_name << "] Component initialized" << std::endl;
    }

    void onActivate() override {
        std::cout << "[" << m_name << "] Component activated" << std::endl;
    }

    void onDeactivate() override {
        std::cout << "[" << m_name << "] Component deactivated" << std::endl;
    }

    void onCleanup() override {
        std::cout << "[" << m_name << "] Component cleaned up" << std::endl;
    }

    void onThemeChanged(bool isDark) override {
        std::cout << "[" << m_name << "] Theme changed to " 
                  << (isDark ? "dark" : "light") << std::endl;
    }

    bool onTick() override {
        // 简单的演示动画
        static int counter = 0;
        counter++;
        if (counter % 60 == 0) { // 每60次tick输出一次
            std::cout << "[" << m_name << "] Tick #" << counter << std::endl;
        }
        return counter < 180; // 180次tick后停止
    }

private:
    std::string m_name;
};

/**
 * @brief 服务接口演示
 */
class IGreetingService {
public:
    virtual ~IGreetingService() = default;
    virtual std::string greet(const std::string& name) = 0;
};

class GreetingService : public IGreetingService {
public:
    std::string greet(const std::string& name) override {
        return "Hello, " + name + "! Welcome to Fangjia Qt6 C++.";
    }
};

int main() {
    std::cout << "============================================" << std::endl;
    std::cout << "Fangjia Qt6 C++ - 新架构演示" << std::endl;
    std::cout << "基于PR #21设计原则的重新实现" << std::endl;
    std::cout << "============================================" << std::endl;
    std::cout << std::endl;

    try {
        // 1. 创建依赖注入容器
        auto container = std::make_shared<SimpleDependencyContainer>();
        
        // 2. 注册服务
        container->registerSingleton<IGreetingService, GreetingService>();
        
        // 3. 设置全局服务定位器
        GlobalServiceLocator::instance().setContainer(container);
        
        std::cout << "✅ 依赖注入容器配置完成" << std::endl;
        std::cout << "   注册的服务数量: " << container->getRegisteredCount() << std::endl;
        std::cout << std::endl;

        // 4. 测试服务解析
        auto greetingService = GlobalServiceLocator::instance().get<IGreetingService>();
        std::cout << greetingService->greet("开发者") << std::endl;
        std::cout << std::endl;

        // 5. 创建和测试组件
        auto component = std::make_shared<DemoComponent>("MainComponent");
        
        std::cout << "--- 组件生命周期测试 ---" << std::endl;
        component->initialize();
        component->activate();
        
        std::cout << "组件状态 - 已初始化: " << (component->isInitialized() ? "是" : "否") 
                  << ", 已激活: " << (component->isActive() ? "是" : "否") << std::endl;
        std::cout << std::endl;

        // 6. 测试主题切换
        std::cout << "--- 主题切换测试 ---" << std::endl;
        component->applyTheme(false); // 浅色主题
        component->applyTheme(true);  // 深色主题
        std::cout << std::endl;

        // 7. 测试动画系统 (简化版)
        std::cout << "--- 动画系统测试 ---" << std::endl;
        std::cout << "运行组件动画..." << std::endl;
        
        int tickCount = 0;
        while (component->tick() && tickCount < 10) { // 限制演示次数
            tickCount++;
        }
        std::cout << "动画测试完成" << std::endl;
        std::cout << std::endl;

        // 8. 清理
        std::cout << "--- 清理阶段 ---" << std::endl;
        component->deactivate();
        component->cleanup();

        std::cout << std::endl;
        std::cout << "============================================" << std::endl;
        std::cout << "✅ 核心架构演示完成！" << std::endl;
        std::cout << std::endl;
        std::cout << "🎯 已实现的核心特性（基于PR #21设计）:" << std::endl;
        std::cout << "   • 清晰的组件生命周期管理" << std::endl;
        std::cout << "   • 线程安全的状态管理" << std::endl;
        std::cout << "   • 依赖注入容器" << std::endl;
        std::cout << "   • 主题切换支持" << std::endl;
        std::cout << "   • 动画系统基础" << std::endl;
        std::cout << "   • 统一的接口设计" << std::endl;
        std::cout << std::endl;
        std::cout << "🚀 下一步: 实现Qt依赖的UI框架层" << std::endl;
        std::cout << "============================================" << std::endl;

        return 0;

    } catch (const std::exception& e) {
        std::cerr << "❌ 错误: " << e.what() << std::endl;
        return 1;
    }
}