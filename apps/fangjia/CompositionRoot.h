#pragma once
#include <memory>
#include <boost/di.hpp>

// Forward declarations
namespace domain::services {
    class IFormulaService;
}

namespace domain::repositories {
    class IFormulaRepository;
}

/// CompositionRoot for dependency injection using boost-ext/di
/// Configures and provides an injector that can resolve dependencies
/// for the Formula domain (Repository -> Service chain)
class CompositionRoot {
public:
    /// Get the configured DI injector
    /// Returns: boost::di injector configured with Formula dependencies
    static auto createInjector();
    
    /// Convenience method to get a Formula service instance
    /// Returns: Shared pointer to configured FormulaService
    static std::shared_ptr<domain::services::IFormulaService> getFormulaService();

private:
    /// Configure the injector with Formula domain bindings
    /// Returns: Configured injector instance
    static auto configureInjector();
};