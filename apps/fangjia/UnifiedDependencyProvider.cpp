#include "UnifiedDependencyProvider.h"
#include "CompositionRoot.h"
#include "usecases/GetSettingsUseCase.h"
#include "usecases/UpdateSettingsUseCase.h"
#include "usecases/ToggleThemeUseCase.h"
#include "usecases/GetThemeModeUseCase.h"
#include "usecases/SetThemeModeUseCase.h"
#include "usecases/GetRecentTabUseCase.h"
#include "usecases/SetRecentTabUseCase.h"
#include "services/FormulaService.h"

// Template implementations
template<typename T>
std::shared_ptr<T> UnifiedDependencyProvider::getFromBoostDI() const {
    static_assert(is_boost_di_managed_v<T>, "Service must be managed by Boost.DI");
    // This should not be called for unknown types, only for explicit specializations
    throw std::runtime_error("getFromBoostDI not implemented for this type");
}

template<typename T>
std::shared_ptr<T> UnifiedDependencyProvider::getFromLegacyProvider() const {
    static_assert(!is_boost_di_managed_v<T>, "Service should not be managed by Boost.DI");
    return resolveLegacyService<T>();
}

// Explicit specialization for IFormulaService using CompositionRoot convenience method
template<>
std::shared_ptr<domain::services::IFormulaService>
UnifiedDependencyProvider::getFromBoostDI<domain::services::IFormulaService>() const {
    return CompositionRoot::getFormulaService();
}

// Settings domain specializations - Phase 3 migration
template<>
std::shared_ptr<domain::usecases::GetSettingsUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetSettingsUseCase>() const {
    return CompositionRoot::getGetSettingsUseCase();
}

template<>
std::shared_ptr<domain::usecases::UpdateSettingsUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::UpdateSettingsUseCase>() const {
    return CompositionRoot::getUpdateSettingsUseCase();
}

// Theme domain specializations - Phase 3 migration
template<>
std::shared_ptr<domain::usecases::GetThemeModeUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetThemeModeUseCase>() const {
    return CompositionRoot::getGetThemeModeUseCase();
}

template<>
std::shared_ptr<domain::usecases::SetThemeModeUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::SetThemeModeUseCase>() const {
    return CompositionRoot::getSetThemeModeUseCase();
}

template<>
std::shared_ptr<domain::usecases::ToggleThemeUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::ToggleThemeUseCase>() const {
    return CompositionRoot::getToggleThemeUseCase();
}

// Recent Tab domain specializations - Phase 3 migration
template<>
std::shared_ptr<domain::usecases::GetRecentTabUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetRecentTabUseCase>() const {
    return CompositionRoot::getGetRecentTabUseCase();
}

template<>
std::shared_ptr<domain::usecases::SetRecentTabUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::SetRecentTabUseCase>() const {
    return CompositionRoot::getSetRecentTabUseCase();
}

// Explicit template instantiations for services currently being used
template std::shared_ptr<domain::services::IFormulaService> 
UnifiedDependencyProvider::getFromBoostDI<domain::services::IFormulaService>() const;

// Settings domain - migrated to Boost.DI in Phase 3
template std::shared_ptr<domain::usecases::GetSettingsUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetSettingsUseCase>() const;

template std::shared_ptr<domain::usecases::UpdateSettingsUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::UpdateSettingsUseCase>() const;

// Theme domain - migrated to Boost.DI in Phase 3
template std::shared_ptr<domain::usecases::GetThemeModeUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetThemeModeUseCase>() const;

template std::shared_ptr<domain::usecases::SetThemeModeUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::SetThemeModeUseCase>() const;

template std::shared_ptr<domain::usecases::ToggleThemeUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::ToggleThemeUseCase>() const;

// Recent Tab domain - migrated to Boost.DI in Phase 3
template std::shared_ptr<domain::usecases::GetRecentTabUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetRecentTabUseCase>() const;

template std::shared_ptr<domain::usecases::SetRecentTabUseCase> 
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::SetRecentTabUseCase>() const;

UnifiedDependencyProvider& UnifiedDependencyProvider::instance()
{
    static UnifiedDependencyProvider instance;
    return instance;
}

void UnifiedDependencyProvider::initialize(
    DependencyProvider& legacyProvider, 
    const std::shared_ptr<domain::services::IFormulaService>& formulaService)
{
    m_legacyProvider = &legacyProvider;
    m_formulaService = formulaService;
}

// Template specializations for legacy service resolution
// Note: All use cases migrated to Boost.DI in Phase 3 - only IFormulaService fallback remains

template<>
std::shared_ptr<domain::services::IFormulaService> 
UnifiedDependencyProvider::resolveLegacyService<domain::services::IFormulaService>() const
{
    // This is a fallback for when IFormulaService is requested from legacy provider
    // Should not normally happen since IFormulaService is marked as Boost.DI managed
    if (!m_formulaService) {
        throw std::runtime_error("FormulaService not initialized");
    }
    return m_formulaService;
}