#include "SimpleDependencyContainer.h"
#include <stdexcept>

namespace Fangjia::Core {

// ==================== SimpleDependencyContainer ====================

SimpleDependencyContainer::SimpleDependencyContainer() = default;

SimpleDependencyContainer::~SimpleDependencyContainer() {
    clear();
}

void SimpleDependencyContainer::clear() {
    std::lock_guard<std::mutex> lock(m_servicesMutex);
    m_services.clear();
    m_singletonInstances.clear();
}

size_t SimpleDependencyContainer::getRegisteredCount() const {
    std::lock_guard<std::mutex> lock(m_servicesMutex);
    return m_services.size();
}

size_t SimpleDependencyContainer::getSingletonCount() const {
    std::lock_guard<std::mutex> lock(m_servicesMutex);
    return m_singletonInstances.size();
}

void SimpleDependencyContainer::registerSingletonImpl(std::type_index type, FactoryFunction factory) {
    std::lock_guard<std::mutex> lock(m_servicesMutex);
    
    ServiceInfo info;
    info.type = ServiceType::Singleton;
    info.factory = factory;
    info.instance = nullptr; // lazy initialization
    
    m_services[type] = info;
}

void SimpleDependencyContainer::registerTransientImpl(std::type_index type, FactoryFunction factory) {
    std::lock_guard<std::mutex> lock(m_servicesMutex);
    
    ServiceInfo info;
    info.type = ServiceType::Transient;
    info.factory = factory;
    info.instance = nullptr;
    
    m_services[type] = info;
}

void SimpleDependencyContainer::registerInstanceImpl(std::type_index type, std::shared_ptr<void> instance) {
    std::lock_guard<std::mutex> lock(m_servicesMutex);
    
    ServiceInfo info;
    info.type = ServiceType::Instance;
    info.factory = nullptr;
    info.instance = instance;
    
    m_services[type] = info;
}

std::shared_ptr<void> SimpleDependencyContainer::resolveImpl(std::type_index type) {
    std::lock_guard<std::mutex> lock(m_servicesMutex);
    
    auto serviceIt = m_services.find(type);
    if (serviceIt == m_services.end()) {
        throw std::runtime_error("Service not registered");
    }
    
    const ServiceInfo& info = serviceIt->second;
    
    switch (info.type) {
        case ServiceType::Instance:
            return info.instance;
            
        case ServiceType::Singleton: {
            // Check if singleton instance already created
            auto singletonIt = m_singletonInstances.find(type);
            if (singletonIt != m_singletonInstances.end()) {
                return singletonIt->second;
            }
            
            // Create new singleton instance
            if (!info.factory) {
                throw std::runtime_error("Singleton factory is null");
            }
            
            auto instance = info.factory();
            m_singletonInstances[type] = instance;
            return instance;
        }
        
        case ServiceType::Transient: {
            if (!info.factory) {
                throw std::runtime_error("Transient factory is null");
            }
            return info.factory();
        }
        
        default:
            throw std::runtime_error("Unknown service type");
    }
}

bool SimpleDependencyContainer::isRegisteredImpl(std::type_index type) const {
    std::lock_guard<std::mutex> lock(m_servicesMutex);
    return m_services.find(type) != m_services.end();
}

// ==================== GlobalServiceLocator ====================

GlobalServiceLocator& GlobalServiceLocator::instance() {
    static GlobalServiceLocator instance;
    return instance;
}

void GlobalServiceLocator::setContainer(std::shared_ptr<IDependencyContainer> container) {
    m_container = container;
}

} // namespace Fangjia::Core