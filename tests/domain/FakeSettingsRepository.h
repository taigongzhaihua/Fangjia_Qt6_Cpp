#pragma once
#include "repositories/ISettingsRepository.h"
#include "entities/Settings.h"
#include <memory>

namespace tests {

/// Fake/In-memory implementation of ISettingsRepository for testing
class FakeSettingsRepository : public domain::repositories::ISettingsRepository {
public:
    FakeSettingsRepository() = default;
    explicit FakeSettingsRepository(const domain::entities::Settings& initialSettings)
        : m_settings(initialSettings) {}
    
    // ISettingsRepository implementation
    domain::entities::Settings getSettings() const override {
        return m_settings;
    }
    
    void updateSettings(const domain::entities::Settings& settings) override {
        m_settings = settings;
    }
    
    void save() override {
        m_saved = true;
    }
    
    void reset() override {
        m_settings = domain::entities::Settings();
        m_saved = false;
    }
    
    // Test utilities
    bool wasSaveCalled() const { return m_saved; }
    void resetSaveFlag() { m_saved = false; }
    
private:
    domain::entities::Settings m_settings;
    bool m_saved = false;
};

} // namespace tests