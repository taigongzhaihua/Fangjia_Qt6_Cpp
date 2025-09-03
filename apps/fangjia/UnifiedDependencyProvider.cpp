#include "CompositionRoot.h"
#include "FormulaService.h"
#include "GetRecentTabUseCase.h"
#include "GetSettingsUseCase.h"
#include "GetThemeModeUseCase.h"
#include "SetRecentTabUseCase.h"
#include "SetThemeModeUseCase.h"
#include "ToggleThemeUseCase.h"
#include "UnifiedDependencyProvider.h"
#include "UpdateSettingsUseCase.h"
#include <memory>
#include <stdexcept>

// Pure Boost.DI template implementation - Phase 4 complete
template<typename T>
std::shared_ptr<T> UnifiedDependencyProvider::getFromBoostDI() const {
	// All services are now managed by Boost.DI through CompositionRoot
	// This should only be called for explicit specializations
	throw std::runtime_error("getFromBoostDI not implemented for this type");
}

// Explicit specializations for all services through CompositionRoot
template<>
std::shared_ptr<domain::services::IFormulaService>
UnifiedDependencyProvider::getFromBoostDI<domain::services::IFormulaService>() const {
	return CompositionRoot::getFormulaService();
}

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

// Explicit template instantiations for all services - pure Boost.DI
template std::shared_ptr<domain::services::IFormulaService>
UnifiedDependencyProvider::getFromBoostDI<domain::services::IFormulaService>() const;

template std::shared_ptr<domain::usecases::GetSettingsUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetSettingsUseCase>() const;

template std::shared_ptr<domain::usecases::UpdateSettingsUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::UpdateSettingsUseCase>() const;

template std::shared_ptr<domain::usecases::GetThemeModeUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetThemeModeUseCase>() const;

template std::shared_ptr<domain::usecases::SetThemeModeUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::SetThemeModeUseCase>() const;

template std::shared_ptr<domain::usecases::ToggleThemeUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::ToggleThemeUseCase>() const;

template std::shared_ptr<domain::usecases::GetRecentTabUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::GetRecentTabUseCase>() const;

template std::shared_ptr<domain::usecases::SetRecentTabUseCase>
UnifiedDependencyProvider::getFromBoostDI<domain::usecases::SetRecentTabUseCase>() const;

UnifiedDependencyProvider& UnifiedDependencyProvider::instance()
{
	static UnifiedDependencyProvider instance;
	return instance;
}