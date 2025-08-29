#pragma once
#include "repositories/ISettingsRepository.h"
#include "entities/Settings.h"
#include <memory>

// Forward declaration to avoid Qt includes in header
class AppConfig;

namespace data {
namespace repositories {

/// Concrete implementation of ISettingsRepository using AppConfig
/// Maps between Qt types (AppConfig) and pure C++ domain types (Settings)
class SettingsRepository : public ::domain::repositories::ISettingsRepository {
public:
    explicit SettingsRepository(std::shared_ptr<AppConfig> appConfig);
    ~SettingsRepository() override = default;
    
    // ISettingsRepository implementation
    ::domain::entities::Settings getSettings() const override;
    void updateSettings(const ::domain::entities::Settings& settings) override;
    void save() override;
    void reset() override;
    
private:
    std::shared_ptr<AppConfig> m_appConfig;
    
    /// Convert AppConfig to domain Settings
    ::domain::entities::Settings mapToDomain() const;
    
    /// Apply domain Settings to AppConfig
    void mapFromDomain(const ::domain::entities::Settings& settings);
};

} // namespace repositories
} // namespace data