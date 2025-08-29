#include "GetRecentTabUseCase.h"

namespace domain {
namespace usecases {

GetRecentTabUseCase::GetRecentTabUseCase(std::shared_ptr<repositories::ISettingsRepository> repository)
    : m_repository(std::move(repository))
{
}

std::string GetRecentTabUseCase::execute() const
{
    const auto settings = m_repository->getSettings();
    return settings.recentTab;
}

} // namespace usecases
} // namespace domain