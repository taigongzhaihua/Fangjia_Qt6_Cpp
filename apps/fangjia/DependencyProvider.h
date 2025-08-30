#pragma once
#include <memory>

// Forward declarations
namespace domain::usecases
{
	class GetSettingsUseCase;
	class UpdateSettingsUseCase;
	class ToggleThemeUseCase;
	class GetThemeModeUseCase;
	class SetThemeModeUseCase;
	class GetRecentTabUseCase;
	class SetRecentTabUseCase;
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
	void setGetThemeModeUseCase(std::shared_ptr<domain::usecases::GetThemeModeUseCase> useCase);
	void setSetThemeModeUseCase(std::shared_ptr<domain::usecases::SetThemeModeUseCase> useCase);
	void setGetRecentTabUseCase(std::shared_ptr<domain::usecases::GetRecentTabUseCase> useCase);
	void setSetRecentTabUseCase(std::shared_ptr<domain::usecases::SetRecentTabUseCase> useCase);

	// Getters (used by ViewModels)
	std::shared_ptr<domain::usecases::GetSettingsUseCase> getGetSettingsUseCase() const;
	std::shared_ptr<domain::usecases::UpdateSettingsUseCase> getUpdateSettingsUseCase() const;
	std::shared_ptr<domain::usecases::ToggleThemeUseCase> getToggleThemeUseCase() const;
	std::shared_ptr<domain::usecases::GetThemeModeUseCase> getGetThemeModeUseCase() const;
	std::shared_ptr<domain::usecases::SetThemeModeUseCase> getSetThemeModeUseCase() const;
	std::shared_ptr<domain::usecases::GetRecentTabUseCase> getGetRecentTabUseCase() const;
	std::shared_ptr<domain::usecases::SetRecentTabUseCase> getSetRecentTabUseCase() const;

private:
	std::shared_ptr<domain::usecases::GetSettingsUseCase> m_getSettingsUseCase;
	std::shared_ptr<domain::usecases::UpdateSettingsUseCase> m_updateSettingsUseCase;
	std::shared_ptr<domain::usecases::ToggleThemeUseCase> m_toggleThemeUseCase;
	std::shared_ptr<domain::usecases::GetThemeModeUseCase> m_getThemeModeUseCase;
	std::shared_ptr<domain::usecases::SetThemeModeUseCase> m_setThemeModeUseCase;
	std::shared_ptr<domain::usecases::GetRecentTabUseCase> m_getRecentTabUseCase;
	std::shared_ptr<domain::usecases::SetRecentTabUseCase> m_setRecentTabUseCase;
};