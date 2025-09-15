// SetThemeModeUseCase.ixx - C++20 module interface for SetThemeModeUseCase
module;

#include <memory>

export module fangjia.domain.usecases.set_theme_mode;

import fangjia.domain.entities.theme;

// Forward declarations for internal dependencies
export namespace domain::repositories {
	class ISettingsRepository;
}

export namespace domain::usecases
{
	/// Use case for setting the theme mode setting
	class SetThemeModeUseCase {
	public:
		explicit SetThemeModeUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);

		/// Set the theme mode in settings and save
		/// @param mode New theme mode to set
		void execute(entities::ThemeMode mode) const;

	private:
		std::shared_ptr<repositories::ISettingsRepository> m_repository;
	};
}