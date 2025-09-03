#pragma once
#include <memory>
#include "CompositionRoot.h"
#include <boost/di.hpp>

// Forward declarations for domain types
namespace domain::usecases {
    class GetSettingsUseCase;
    class UpdateSettingsUseCase;
    class ToggleThemeUseCase;
    class GetThemeModeUseCase;
    class SetThemeModeUseCase;
    class GetRecentTabUseCase;
    class SetRecentTabUseCase;
}

namespace domain::services {
    class IFormulaService;
}

/// Pure Boost.DI dependency provider - Phase 4 implementation
/// All services have been migrated to Boost.DI, providing unified dependency resolution
/// Replaces the previous dual-system architecture with pure Boost.DI
class UnifiedDependencyProvider {
public:
    static UnifiedDependencyProvider& instance();

    /// Pure Boost.DI service resolution - all services migrated in Phase 3
    /// @tparam T The service type to resolve
    /// @return Shared pointer to the requested service
    template<typename T>
    std::shared_ptr<T> get() const {
        return getFromBoostDI<T>();
    }

private:
    UnifiedDependencyProvider() = default;

    /// Boost.DI service resolution through CompositionRoot
    template<typename T>
    std::shared_ptr<T> getFromBoostDI() const;
};

// Template specializations for pure Boost.DI resolution - Phase 4 complete
template<>
std::shared_ptr<domain::services::IFormulaService> 
UnifiedDependencyProvider::getFromBoostDI<domain::services::IFormulaService>() const;

template<>
std::shared_ptr<domain::usecases::GetSettingsUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetSettingsUseCase>() const;

template<>
std::shared_ptr<domain::usecases::UpdateSettingsUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::UpdateSettingsUseCase>() const;

template<>
std::shared_ptr<domain::usecases::GetThemeModeUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetThemeModeUseCase>() const;

template<>
std::shared_ptr<domain::usecases::SetThemeModeUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::SetThemeModeUseCase>() const;

template<>
std::shared_ptr<domain::usecases::ToggleThemeUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::ToggleThemeUseCase>() const;

template<>
std::shared_ptr<domain::usecases::GetRecentTabUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetRecentTabUseCase>() const;

template<>
std::shared_ptr<domain::usecases::SetRecentTabUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::SetRecentTabUseCase>() const;