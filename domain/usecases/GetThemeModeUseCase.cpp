#include "GetThemeModeUseCase.h"

namespace domain {
namespace usecases {

GetThemeModeUseCase::GetThemeModeUseCase(std::shared_ptr<repositories::ISettingsRepository> repository)
    : m_repository(std::move(repository))
{
}

entities::ThemeMode GetThemeModeUseCase::execute() const
{
    const auto settings = m_repository->getSettings();
    return entities::stringToThemeMode(settings.themeMode.c_str());
}

} // namespace usecases
} // namespace domain