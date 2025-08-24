#pragma once
#include <any>
#include <functional>
#include <memory>
#include <qobject.h>
#include <typeindex>
#include <unordered_map>

// 服务定位器（简单的依赖注入容器）
class ServiceLocator final
{
public:
	// 获取单例
	static ServiceLocator& instance() {
		static ServiceLocator instance;
		return instance;
	}

	// 注册服务（单例）
	template<typename T>
	void registerSingleton(std::shared_ptr<T> instance) {
		m_services[std::type_index(typeid(T))] = instance;
	}

	// 注册服务（工厂函数）
	template<typename T>
	void registerFactory(std::function<std::shared_ptr<T>()> factory) {
		m_factories[std::type_index(typeid(T))] = [factory]() -> std::any {
			return factory();
			};
	}

	// 获取服务
	template<typename T>
	std::shared_ptr<T> get() {
		const std::type_index typeIdx(typeid(T));

		// 先查找已创建的实例
		if (auto it = m_services.find(typeIdx); it != m_services.end()) {
			return std::any_cast<std::shared_ptr<T>>(it->second);
		}

		// 尝试使用工厂创建
		if (auto it = m_factories.find(typeIdx); it != m_factories.end()) {
			auto instance = std::any_cast<std::shared_ptr<T>>(it->second());
			m_services[typeIdx] = instance;  // 缓存实例
			return instance;
		}

		return nullptr;
	}

	// 清理所有服务
	void clear() {
		m_services.clear();
		m_factories.clear();
	}

private:
	ServiceLocator() = default;
	~ServiceLocator() = default;

	// 禁止拷贝
	ServiceLocator(const ServiceLocator&) = delete;
	ServiceLocator& operator=(const ServiceLocator&) = delete;

	std::unordered_map<std::type_index, std::any> m_services;
	std::unordered_map<std::type_index, std::function<std::any()>> m_factories;
};

// 便捷宏
#define DI ServiceLocator::instance()