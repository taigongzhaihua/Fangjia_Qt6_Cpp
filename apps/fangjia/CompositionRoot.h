#pragma once
#include <memory>
#include <boost/di.hpp>

// Forward declarations
namespace domain::services {
    class IFormulaService;
}

namespace domain::repositories {
    class IFormulaRepository;
    class ISettingsRepository;
}

namespace domain::usecases {
    class GetSettingsUseCase;
    class UpdateSettingsUseCase;
}

class AppConfig;

/// CompositionRoot for dependency injection using boost-ext/di
/// Configures and provides an injector that can resolve dependencies
/// for Formula and Settings domains
class CompositionRoot {
public:
    /// Get the configured DI injector
    /// Returns: boost::di injector configured with Formula and Settings dependencies
    static auto createInjector();
    
    /// Convenience method to get a Formula service instance
    /// Returns: Shared pointer to configured FormulaService
    static std::shared_ptr<domain::services::IFormulaService> getFormulaService();
    
    /// Convenience method to get a Settings repository instance
    /// Returns: Shared pointer to configured SettingsRepository
    static std::shared_ptr<domain::repositories::ISettingsRepository> getSettingsRepository();
    
    /// Convenience method to get GetSettingsUseCase instance
    /// Returns: Shared pointer to configured GetSettingsUseCase
    static std::shared_ptr<domain::usecases::GetSettingsUseCase> getGetSettingsUseCase();
    
    /// Convenience method to get UpdateSettingsUseCase instance
    /// Returns: Shared pointer to configured UpdateSettingsUseCase
    static std::shared_ptr<domain::usecases::UpdateSettingsUseCase> getUpdateSettingsUseCase();

private:
    /// Configure the injector with Formula and Settings domain bindings
    /// Returns: Configured injector instance
    static auto configureInjector();
};