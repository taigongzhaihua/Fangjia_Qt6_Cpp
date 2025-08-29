#pragma once
#include <memory>

// Forward declarations
namespace domain {
namespace usecases {
class GetSettingsUseCase;
class UpdateSettingsUseCase;
class ToggleThemeUseCase;
}
}

/// Minimal service locator for dependency injection during Stage 3 refactor
/// This is a temporary solution to avoid wide constructor changes
/// TODO: Replace with proper DI container in future phases
class DependencyProvider {
public:
    static DependencyProvider& instance();
    
    // Setters (used by composition root)
    void setGetSettingsUseCase(std::shared_ptr<domain::usecases::GetSettingsUseCase> useCase);
    void setUpdateSettingsUseCase(std::shared_ptr<domain::usecases::UpdateSettingsUseCase> useCase);
    void setToggleThemeUseCase(std::shared_ptr<domain::usecases::ToggleThemeUseCase> useCase);
    
    // Getters (used by ViewModels)
    std::shared_ptr<domain::usecases::GetSettingsUseCase> getGetSettingsUseCase() const;
    std::shared_ptr<domain::usecases::UpdateSettingsUseCase> getUpdateSettingsUseCase() const;
    std::shared_ptr<domain::usecases::ToggleThemeUseCase> getToggleThemeUseCase() const;
    
private:
    std::shared_ptr<domain::usecases::GetSettingsUseCase> m_getSettingsUseCase;
    std::shared_ptr<domain::usecases::UpdateSettingsUseCase> m_updateSettingsUseCase;
    std::shared_ptr<domain::usecases::ToggleThemeUseCase> m_toggleThemeUseCase;
};