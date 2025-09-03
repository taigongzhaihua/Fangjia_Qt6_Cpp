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

// Explicit template instantiations for services currently being used
template std::shared_ptr<domain::services::IFormulaService> 
UnifiedDependencyProvider::getFromBoostDI<domain::services::IFormulaService>() const;

template std::shared_ptr<domain::usecases::GetSettingsUseCase> 
UnifiedDependencyProvider::getFromLegacyProvider<domain::usecases::GetSettingsUseCase>() const;

template std::shared_ptr<domain::usecases::UpdateSettingsUseCase> 
UnifiedDependencyProvider::getFromLegacyProvider<domain::usecases::UpdateSettingsUseCase>() const;

template std::shared_ptr<domain::usecases::ToggleThemeUseCase> 
UnifiedDependencyProvider::getFromLegacyProvider<domain::usecases::ToggleThemeUseCase>() const;

template std::shared_ptr<domain::usecases::GetThemeModeUseCase> 
UnifiedDependencyProvider::getFromLegacyProvider<domain::usecases::GetThemeModeUseCase>() const;

template std::shared_ptr<domain::usecases::SetThemeModeUseCase> 
UnifiedDependencyProvider::getFromLegacyProvider<domain::usecases::SetThemeModeUseCase>() const;

template std::shared_ptr<domain::usecases::GetRecentTabUseCase> 
UnifiedDependencyProvider::getFromLegacyProvider<domain::usecases::GetRecentTabUseCase>() const;

template std::shared_ptr<domain::usecases::SetRecentTabUseCase> 
UnifiedDependencyProvider::getFromLegacyProvider<domain::usecases::SetRecentTabUseCase>() const;

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
template<>
std::shared_ptr<domain::usecases::GetSettingsUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::GetSettingsUseCase>() const
{
    if (!m_legacyProvider) {
        throw std::runtime_error("Legacy provider not initialized");
    }
    return m_legacyProvider->getGetSettingsUseCase();
}

template<>
std::shared_ptr<domain::usecases::UpdateSettingsUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::UpdateSettingsUseCase>() const
{
    if (!m_legacyProvider) {
        throw std::runtime_error("Legacy provider not initialized");
    }
    return m_legacyProvider->getUpdateSettingsUseCase();
}

template<>
std::shared_ptr<domain::usecases::ToggleThemeUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::ToggleThemeUseCase>() const
{
    if (!m_legacyProvider) {
        throw std::runtime_error("Legacy provider not initialized");
    }
    return m_legacyProvider->getToggleThemeUseCase();
}

template<>
std::shared_ptr<domain::usecases::GetThemeModeUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::GetThemeModeUseCase>() const
{
    if (!m_legacyProvider) {
        throw std::runtime_error("Legacy provider not initialized");
    }
    return m_legacyProvider->getGetThemeModeUseCase();
}

template<>
std::shared_ptr<domain::usecases::SetThemeModeUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::SetThemeModeUseCase>() const
{
    if (!m_legacyProvider) {
        throw std::runtime_error("Legacy provider not initialized");
    }
    return m_legacyProvider->getSetThemeModeUseCase();
}

template<>
std::shared_ptr<domain::usecases::GetRecentTabUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::GetRecentTabUseCase>() const
{
    if (!m_legacyProvider) {
        throw std::runtime_error("Legacy provider not initialized");
    }
    return m_legacyProvider->getGetRecentTabUseCase();
}

template<>
std::shared_ptr<domain::usecases::SetRecentTabUseCase> 
UnifiedDependencyProvider::resolveLegacyService<domain::usecases::SetRecentTabUseCase>() const
{
    if (!m_legacyProvider) {
        throw std::runtime_error("Legacy provider not initialized");
    }
    return m_legacyProvider->getSetRecentTabUseCase();
}

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