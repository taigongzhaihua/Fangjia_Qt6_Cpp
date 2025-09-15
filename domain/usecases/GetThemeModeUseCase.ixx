// GetThemeModeUseCase.ixx - C++20 module interface for GetThemeModeUseCase
module;

#include <memory>

export module fangjia.domain.usecases.get_theme_mode;

import fangjia.domain.entities.theme;

// Forward declarations for internal dependencies
export namespace domain::repositories {
	class ISettingsRepository;
}

export namespace domain::usecases
{
	/// Use case for retrieving current theme mode
	/// Pure C++ - no Qt dependencies
	class GetThemeModeUseCase {
	public:
		explicit GetThemeModeUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);
		~GetThemeModeUseCase() = default;

		/// Execute the use case - get current theme mode
		entities::ThemeMode execute() const;

	private:
		std::shared_ptr<repositories::ISettingsRepository> m_repository;
	};
}