// UnifiedDependencyProvider.ixx - C++20 module interface for UnifiedDependencyProvider
module;

#include <memory>

export module fangjia.apps.unified_dependency_provider;

// Forward declarations for domain types
export namespace domain::usecases {
	class GetSettingsUseCase;
	class UpdateSettingsUseCase;
	class ToggleThemeUseCase;
	class GetThemeModeUseCase;
	class SetThemeModeUseCase;
	class GetRecentTabUseCase;
	class SetRecentTabUseCase;
}

export namespace domain::services {
	class IFormulaService;
}

export /// Unified dependency provider using pure Boost.DI container
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