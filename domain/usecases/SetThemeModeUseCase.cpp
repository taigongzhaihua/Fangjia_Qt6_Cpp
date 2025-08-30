#include "SetThemeModeUseCase.h"

namespace domain::usecases
{

	SetThemeModeUseCase::SetThemeModeUseCase(std::shared_ptr<repositories::ISettingsRepository> repository)
		: m_repository(std::move(repository))
	{
	}

	void SetThemeModeUseCase::execute(entities::ThemeMode mode)
	{
		auto settings = m_repository->getSettings();
		settings.themeMode = entities::themeModeToString(mode);
		m_repository->updateSettings(settings);
		m_repository->save();
	}

}
