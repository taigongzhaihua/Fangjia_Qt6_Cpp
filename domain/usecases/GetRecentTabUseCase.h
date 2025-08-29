#pragma once
#include "repositories/ISettingsRepository.h"
#include <memory>
#include <string>

namespace domain {
namespace usecases {

/// Use case for retrieving the recent tab setting
class GetRecentTabUseCase {
public:
    explicit GetRecentTabUseCase(std::shared_ptr<repositories::ISettingsRepository> repository);
    
    /// Get the recent tab ID from settings
    /// @return Recent tab ID as string (empty if not set)
    std::string execute() const;

private:
    std::shared_ptr<repositories::ISettingsRepository> m_repository;
};

} // namespace usecases
} // namespace domain