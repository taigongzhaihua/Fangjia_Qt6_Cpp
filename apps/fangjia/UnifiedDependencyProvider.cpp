#include "UnifiedDependencyProvider.h"
#include "usecases/GetSettingsUseCase.h"
#include "usecases/UpdateSettingsUseCase.h"
#include "usecases/ToggleThemeUseCase.h"
#include "usecases/GetThemeModeUseCase.h"
#include "usecases/SetThemeModeUseCase.h"
#include "usecases/GetRecentTabUseCase.h"
#include "usecases/SetRecentTabUseCase.h"
#include "services/IFormulaService.h"

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