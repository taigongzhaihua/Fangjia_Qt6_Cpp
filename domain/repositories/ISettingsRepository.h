#pragma once
#include "entities/Settings.h"
#include <memory>

namespace domain {
namespace repositories {

/// Abstract repository interface for application settings
/// Pure C++ interface - no Qt dependencies
class ISettingsRepository {
public:
    virtual ~ISettingsRepository() = default;
    
    /// Load settings from persistent storage
    virtual entities::Settings getSettings() const = 0;
    
    /// Save settings to persistent storage  
    virtual void updateSettings(const entities::Settings& settings) = 0;
    
    /// Save current settings immediately (for critical changes)
    virtual void save() = 0;
    
    /// Reset settings to defaults
    virtual void reset() = 0;
};

} // namespace repositories  
} // namespace domain