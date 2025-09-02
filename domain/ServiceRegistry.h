#pragma once
#include <memory>

// Forward declarations
namespace domain::services {
    class IFormulaService;
}

namespace domain {

/// Global service registry for domain services
/// Allows dependency injection while maintaining proper layering
/// Configured by composition root, accessed by presentation layer
class ServiceRegistry {
public:
    /// Get the singleton instance
    static ServiceRegistry& instance();
    
    /// Configure the Formula service (called by composition root)
    void setFormulaService(std::shared_ptr<services::IFormulaService> service);
    
    /// Get the Formula service (called by ViewModels)
    std::shared_ptr<services::IFormulaService> getFormulaService() const;

private:
    std::shared_ptr<services::IFormulaService> m_formulaService;
};

} // namespace domain