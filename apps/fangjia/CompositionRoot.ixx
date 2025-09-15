// CompositionRoot.ixx - C++20 module interface for CompositionRoot
module;

#include <memory>

export module fangjia.apps.composition_root;

// Forward declarations for domain types
export namespace domain::services {
	class IFormulaService;
}

export namespace domain::repositories {
	class IFormulaRepository;
	class ISettingsRepository;
}

export namespace domain::usecases {
	class GetSettingsUseCase;
	class UpdateSettingsUseCase;
	class GetThemeModeUseCase;
	class SetThemeModeUseCase;
	class ToggleThemeUseCase;
	class GetRecentTabUseCase;
	class SetRecentTabUseCase;
}

export class AppConfig;

export /// CompositionRoot for dependency injection using boost-ext/di
/// Configures and provides an injector that can resolve dependencies
/// for Formula and Settings domains
class CompositionRoot {
public:
	/// Get the configured DI injector
	/// Returns: boost::di injector configured with Formula and Settings dependencies
	static auto createInjector();

	/// Convenience method to get a Formula service instance
	/// Returns: Shared pointer to configured FormulaService
	static std::shared_ptr<domain::services::IFormulaService> getFormulaService();

	/// Convenience method to get a Settings repository instance
	/// Returns: Shared pointer to configured SettingsRepository
	static std::shared_ptr<domain::repositories::ISettingsRepository> getSettingsRepository();

	/// Convenience method to get GetSettingsUseCase instance
	/// Returns: Shared pointer to configured GetSettingsUseCase
	static std::shared_ptr<domain::usecases::GetSettingsUseCase> getGetSettingsUseCase();

	/// Convenience method to get UpdateSettingsUseCase instance
	/// Returns: Shared pointer to configured UpdateSettingsUseCase
	static std::shared_ptr<domain::usecases::UpdateSettingsUseCase> getUpdateSettingsUseCase();

	/// Convenience method to get GetThemeModeUseCase instance
	/// Returns: Shared pointer to configured GetThemeModeUseCase
	static std::shared_ptr<domain::usecases::GetThemeModeUseCase> getGetThemeModeUseCase();

	/// Convenience method to get SetThemeModeUseCase instance
	/// Returns: Shared pointer to configured SetThemeModeUseCase
	static std::shared_ptr<domain::usecases::SetThemeModeUseCase> getSetThemeModeUseCase();

	/// Convenience method to get ToggleThemeUseCase instance
	/// Returns: Shared pointer to configured ToggleThemeUseCase
	static std::shared_ptr<domain::usecases::ToggleThemeUseCase> getToggleThemeUseCase();

	/// Convenience method to get GetRecentTabUseCase instance
	/// Returns: Shared pointer to configured GetRecentTabUseCase
	static std::shared_ptr<domain::usecases::GetRecentTabUseCase> getGetRecentTabUseCase();

	/// Convenience method to get SetRecentTabUseCase instance
	/// Returns: Shared pointer to configured SetRecentTabUseCase
	static std::shared_ptr<domain::usecases::SetRecentTabUseCase> getSetRecentTabUseCase();

private:
	/// Configure the injector with Formula and Settings domain bindings
	/// Returns: Configured injector instance
	static auto configureInjector();
};