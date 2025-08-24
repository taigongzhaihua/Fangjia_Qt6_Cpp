#pragma once
#include <memory>
#include <typeindex>
#include <unordered_map>
#include <functional>
#include <mutex>

namespace DI {

// 服务定位器：简单的依赖注入容器
class ServiceLocator {
public:
    static ServiceLocator& instance() {
        static ServiceLocator inst;
        return inst;
    }

    // 注册单例服务
    template<typename T>
    void registerSingleton(std::shared_ptr<T> instance) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_services[std::type_index(typeid(T))] = instance;
    }

    // 注册工厂
    template<typename T>
    void registerFactory(std::function<std::shared_ptr<T>()> factory) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_factories[std::type_index(typeid(T))] = [factory]() -> std::any {
            return factory();
        };
    }

    // 获取服务
    template<typename T>
    std::shared_ptr<T> get() {
        std::lock_guard<std::mutex> lock(m_mutex);
        
        // 先查找单例
        auto it = m_services.find(std::type_index(typeid(T)));
        if (it != m_services.end()) {
            return std::any_cast<std::shared_ptr<T>>(it->second);
        }
        
        // 查找工厂
        auto factoryIt = m_factories.find(std::type_index(typeid(T)));
        if (factoryIt != m_factories.end()) {
            auto instance = std::any_cast<std::shared_ptr<T>>(factoryIt->second());
            m_services[std::type_index(typeid(T))] = instance; // 缓存
            return instance;
        }
        
        return nullptr;
    }

    // 清理所有服务
    void clear() {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_services.clear();
        m_factories.clear();
    }

private:
    ServiceLocator() = default;
    ~ServiceLocator() = default;
    ServiceLocator(const ServiceLocator&) = delete;
    ServiceLocator& operator=(const ServiceLocator&) = delete;

    std::unordered_map<std::type_index, std::any> m_services;
    std::unordered_map<std::type_index, std::function<std::any()>> m_factories;
    mutable std::mutex m_mutex;
};

// 便捷函数
template<typename T>
inline void registerService(std::shared_ptr<T> instance) {
    ServiceLocator::instance().registerSingleton<T>(instance);
}

template<typename T>
inline std::shared_ptr<T> getService() {
    return ServiceLocator::instance().get<T>();
}

} // namespace DI