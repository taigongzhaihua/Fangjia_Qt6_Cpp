#include "GetSettingsUseCase.h"

namespace domain::usecases
{

	GetSettingsUseCase::GetSettingsUseCase(std::shared_ptr<repositories::ISettingsRepository> repository)
		: m_repository(std::move(repository))
	{
	}

	entities::Settings GetSettingsUseCase::execute() const
	{
		return m_repository->getSettings();
	}

}
