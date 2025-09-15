// ToggleThemeUseCase.ixx - C++20 module interface for ToggleThemeUseCase
module;

#include <memory>
#include <string>

export module fangjia.domain.usecases.toggle_theme;

import fangjia.domain.entities.settings;

// Forward declarations for internal dependencies
export namespace domain::repositories {
	class ISettingsRepository;
}

export namespace domain::usecases
{
	/// Use case for toggling theme mode through the cycle: system -> light -> dark -> system
	/// Pure C++ - no Qt dependencies
	class ToggleThemeUseCase {
	public:
		explicit ToggleThemeUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);
		~ToggleThemeUseCase() = default;

		/// Execute the use case - cycle to next theme mode and return the new mode
		std::string execute() const;

	private:
		std::shared_ptr<repositories::ISettingsRepository> m_repository;

		/// Get the next theme mode in the cycle
		std::string getNextThemeMode(const std::string& currentMode) const;
	};
}