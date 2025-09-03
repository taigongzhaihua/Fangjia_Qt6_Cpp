#pragma once
#include <memory>
#include <type_traits>
#include "DependencyProvider.h"
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

/// Unified dependency provider that bridges Boost.DI and legacy service locator
/// This implements Phase 1 of the DI unification strategy from architecture-analysis.md
/// Provides a template-based unified interface for dependency resolution
class UnifiedDependencyProvider {
public:
    static UnifiedDependencyProvider& instance();

    /// Template-based service resolution with compile-time system selection
    /// @tparam T The service type to resolve
    /// @return Shared pointer to the requested service
    template<typename T>
    std::shared_ptr<T> get() const {
        if constexpr (is_boost_di_managed_v<T>) {
            // Use Boost.DI for Formula domain services
            return getFromBoostDI<T>();
        } else {
            // Use legacy service locator for Settings/Theme services
            return getFromLegacyProvider<T>();
        }
    }

    /// Initialize the unified provider with existing DI systems
    /// This maintains backward compatibility with current initialization in main.cpp
    void initialize(DependencyProvider& legacyProvider, const std::shared_ptr<domain::services::IFormulaService>& formulaService);

    /// Check if a service is managed by Boost.DI (compile-time)
    template<typename T>
    static constexpr bool isBoostDIManaged() {
        return is_boost_di_managed_v<T>;
    }

    /// Get migration status for a service type
    template<typename T>
    const char* getMigrationStatus() const {
        if constexpr (is_boost_di_managed_v<T>) {
            return "Boost.DI (✅ migrated)";
        } else {
            return "Legacy Service Locator (🔄 pending migration)";
        }
    }

private:
    UnifiedDependencyProvider() = default;

    /// Type trait to detect Boost.DI managed services at compile time
    template<typename T>
    struct is_boost_di_managed : std::false_type {};

    template<typename T>
    static constexpr bool is_boost_di_managed_v = is_boost_di_managed<T>::value;

    /// Boost.DI service resolution
    template<typename T>
    std::shared_ptr<T> getFromBoostDI() const {
        static_assert(is_boost_di_managed_v<T>, "Service must be managed by Boost.DI");
        auto injector = CompositionRoot::createInjector();
        return injector.template create<std::shared_ptr<T>>();
    }

    /// Legacy service locator resolution
    template<typename T>
    std::shared_ptr<T> getFromLegacyProvider() const {
        static_assert(!is_boost_di_managed_v<T>, "Service should not be managed by Boost.DI");
        return resolveLegacyService<T>();
    }

    /// Template specializations for legacy service resolution
    template<typename T>
    std::shared_ptr<T> resolveLegacyService() const;

    DependencyProvider* m_legacyProvider = nullptr;
    std::shared_ptr<domain::services::IFormulaService> m_formulaService;
};

// Specializations to mark Boost.DI managed services
template<>
struct UnifiedDependencyProvider::is_boost_di_managed<domain::services::IFormulaService> : std::true_type {};

// Template specialization declarations for legacy services
template<>
std::shared_ptr<domain::usecases::GetSettingsUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::GetSettingsUseCase>() const;

template<>
std::shared_ptr<domain::usecases::UpdateSettingsUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::UpdateSettingsUseCase>() const;

template<>
std::shared_ptr<domain::usecases::ToggleThemeUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::ToggleThemeUseCase>() const;

template<>
std::shared_ptr<domain::usecases::GetThemeModeUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::GetThemeModeUseCase>() const;

template<>
std::shared_ptr<domain::usecases::SetThemeModeUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::SetThemeModeUseCase>() const;

template<>
std::shared_ptr<domain::usecases::GetRecentTabUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::GetRecentTabUseCase>() const;

template<>
std::shared_ptr<domain::usecases::SetRecentTabUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::SetRecentTabUseCase>() const;

template<>
std::shared_ptr<domain::services::IFormulaService> 
UnifiedDependencyProvider::resolveLegacyService<domain::services::IFormulaService>() const;