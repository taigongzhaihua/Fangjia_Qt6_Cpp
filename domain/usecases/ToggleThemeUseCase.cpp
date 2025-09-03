#include "ToggleThemeUseCase.h"
#include <ISettingsRepository.h>
#include <memory>
#include <string>
#include <utility>

namespace domain::usecases
{

	ToggleThemeUseCase::ToggleThemeUseCase(std::shared_ptr<repositories::ISettingsRepository> repository)
		: m_repository(std::move(repository))
	{
	}

	std::string ToggleThemeUseCase::execute() const
	{
		auto settings = m_repository->getSettings();
		const std::string newMode = getNextThemeMode(settings.themeMode);

		settings.themeMode = newMode;
		m_repository->updateSettings(settings);
		m_repository->save();

		return newMode;
	}

	std::string ToggleThemeUseCase::getNextThemeMode(const std::string& currentMode) const
	{
		if (currentMode == "system") {
			return "light";
		}
		else if (currentMode == "light") {
			return "dark";
		}
		else {
			return "system"; // dark -> system, or unknown -> system
		}
	}

}
