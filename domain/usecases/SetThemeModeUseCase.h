#pragma once
#include "entities/Theme.h"
#include "repositories/ISettingsRepository.h"
#include <memory>

namespace domain {
namespace usecases {

/// Use case for setting the theme mode setting
class SetThemeModeUseCase {
public:
    explicit SetThemeModeUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);
    
    /// Set the theme mode in settings and save
    /// @param mode New theme mode to set
    void execute(entities::ThemeMode mode);

private:
    std::shared_ptr<repositories::ISettingsRepository> m_repository;
};

} // namespace usecases
} // namespace domain