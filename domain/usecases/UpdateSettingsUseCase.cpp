#include "UpdateSettingsUseCase.h"

namespace domain::usecases
{

	UpdateSettingsUseCase::UpdateSettingsUseCase(std::shared_ptr<repositories::ISettingsRepository> repository)
		: m_repository(std::move(repository))
	{
	}

	void UpdateSettingsUseCase::execute(const entities::Settings& settings, bool saveImmediately)
	{
		m_repository->updateSettings(settings);
		if (saveImmediately) {
			m_repository->save();
		}
	}

}
