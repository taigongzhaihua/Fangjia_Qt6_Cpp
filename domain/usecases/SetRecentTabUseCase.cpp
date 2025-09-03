#include "SetRecentTabUseCase.h"
#include <ISettingsRepository.h>
#include <memory>
#include <string>
#include <utility>

namespace domain::usecases
{

	SetRecentTabUseCase::SetRecentTabUseCase(std::shared_ptr<repositories::ISettingsRepository> repository)
		: m_repository(std::move(repository))
	{
	}

	void SetRecentTabUseCase::execute(const std::string& tabId) const
	{
		auto settings = m_repository->getSettings();
		settings.recentTab = tabId;
		m_repository->updateSettings(settings);
		m_repository->save();
	}

}
