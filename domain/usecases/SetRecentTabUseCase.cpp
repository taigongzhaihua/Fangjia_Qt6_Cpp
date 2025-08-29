#include "SetRecentTabUseCase.h"

namespace domain {
namespace usecases {

SetRecentTabUseCase::SetRecentTabUseCase(std::shared_ptr<repositories::ISettingsRepository> repository)
    : m_repository(std::move(repository))
{
}

void SetRecentTabUseCase::execute(const std::string& tabId)
{
    auto settings = m_repository->getSettings();
    settings.recentTab = tabId;
    m_repository->updateSettings(settings);
    m_repository->save();
}

} // namespace usecases
} // namespace domain