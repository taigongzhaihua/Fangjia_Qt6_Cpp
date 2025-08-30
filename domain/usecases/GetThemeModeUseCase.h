#pragma once
#include "entities/Theme.h"
#include "repositories/ISettingsRepository.h"
#include <memory>

namespace domain::usecases
{

	/// Use case for retrieving the current theme mode setting
	class GetThemeModeUseCase {
	public:
		explicit GetThemeModeUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);

		/// Get the current theme mode from settings
	/// @return Current theme mode (Light, Dark, or FollowSystem)
		entities::ThemeMode execute() const;

	private:
		std::shared_ptr<repositories::ISettingsRepository> m_repository;
	};

}
