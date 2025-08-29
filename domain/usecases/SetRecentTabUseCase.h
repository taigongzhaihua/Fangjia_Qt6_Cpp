#pragma once
#include "repositories/ISettingsRepository.h"
#include <memory>
#include <string>

namespace domain {
namespace usecases {

/// Use case for setting the recent tab setting
class SetRecentTabUseCase {
public:
    explicit SetRecentTabUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);
    
    /// Set the recent tab ID in settings and save
    /// @param tabId New recent tab ID to set
    void execute(const std::string& tabId);

private:
    std::shared_ptr<repositories::ISettingsRepository> m_repository;
};

} // namespace usecases
} // namespace domain