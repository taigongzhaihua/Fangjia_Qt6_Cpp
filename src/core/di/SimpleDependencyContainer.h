#pragma once

#include "../interfaces/IDependencyContainer.h"
#include <unordered_map>
#include <mutex>
#include <stdexcept>

namespace Fangjia::Core {

/**
 * @brief 简单依赖注入容器实现
 * 基于PR #21中统一依赖注入的设计原则
 * 提供线程安全的服务注册和解析
 */
class SimpleDependencyContainer : public IDependencyContainer {
public:
    SimpleDependencyContainer();
    ~SimpleDependencyContainer();

    // 清理所有注册的服务
    void clear();

    // 获取统计信息
    size_t getRegisteredCount() const;
    size_t getSingletonCount() const;

protected:
    void registerSingletonImpl(std::type_index type, FactoryFunction factory) override;
    void registerTransientImpl(std::type_index type, FactoryFunction factory) override;
    void registerInstanceImpl(std::type_index type, std::shared_ptr<void> instance) override;
    std::shared_ptr<void> resolveImpl(std::type_index type) override;
    bool isRegisteredImpl(std::type_index type) const override;

private:
    enum class ServiceType {
        Singleton,
        Transient,
        Instance
    };

    struct ServiceInfo {
        ServiceType type;
        FactoryFunction factory;
        std::shared_ptr<void> instance; // for singletons and instances
    };

    mutable std::mutex m_servicesMutex;
    std::unordered_map<std::type_index, ServiceInfo> m_services;
    std::unordered_map<std::type_index, std::shared_ptr<void>> m_singletonInstances;
};

/**
 * @brief 全局服务定位器
 * 基于PR #21中的统一访问模式
 * 提供简化的依赖访问接口
 */
class GlobalServiceLocator {
public:
    static GlobalServiceLocator& instance();

    // 设置依赖容器
    void setContainer(std::shared_ptr<IDependencyContainer> container);
    
    // 便捷的服务访问方法
    template<typename T>
    std::shared_ptr<T> get() {
        if (!m_container) {
            throw std::runtime_error("No dependency container configured");
        }
        return m_container->resolve<T>();
    }

    template<typename T>
    bool has() const {
        if (!m_container) {
            return false;
        }
        return m_container->isRegistered<T>();
    }

    // 直接注册服务的便捷方法
    template<typename Interface, typename Implementation>
    void registerSingleton() {
        if (!m_container) {
            throw std::runtime_error("No dependency container configured");
        }
        m_container->registerSingleton<Interface, Implementation>();
    }

    template<typename Interface, typename Implementation>
    void registerTransient() {
        if (!m_container) {
            throw std::runtime_error("No dependency container configured");
        }
        m_container->registerTransient<Interface, Implementation>();
    }

    template<typename Interface>
    void registerInstance(std::shared_ptr<Interface> instance) {
        if (!m_container) {
            throw std::runtime_error("No dependency container configured");
        }
        m_container->registerInstance<Interface>(instance);
    }

private:
    GlobalServiceLocator() = default;
    ~GlobalServiceLocator() = default;
    
    GlobalServiceLocator(const GlobalServiceLocator&) = delete;
    GlobalServiceLocator& operator=(const GlobalServiceLocator&) = delete;

    std::shared_ptr<IDependencyContainer> m_container;
};

} // namespace Fangjia::Core