#pragma once
#include "UnifiedDependencyProvider.h"
#include <iostream>

/// Example class demonstrating how to use the UnifiedDependencyProvider
/// This shows the improved developer experience with unified DI access
class UnifiedDIUsageExample {
public:
    /// Demonstrate unified service access
    void demonstrateUnifiedAccess() {
        auto& provider = UnifiedDependencyProvider::instance();
        
        std::cout << "\n=== Unified Dependency Provider Usage Example ===" << std::endl;
        
        // Access services transparently regardless of underlying DI system
        try {
            // This service is managed by Boost.DI (compile-time determination)
            auto formulaService = provider.get<domain::services::IFormulaService>();
            std::cout << "✅ FormulaService resolved from Boost.DI: " 
                      << (formulaService ? "Success" : "Failed") << std::endl;
            std::cout << "   Status: " << provider.getMigrationStatus<domain::services::IFormulaService>() << std::endl;
            
            // These services are managed by legacy provider (compile-time determination)
            auto settingsUseCase = provider.get<domain::usecases::GetSettingsUseCase>();
            std::cout << "✅ GetSettingsUseCase resolved from Legacy: " 
                      << (settingsUseCase ? "Success" : "Failed") << std::endl;
            std::cout << "   Status: " << provider.getMigrationStatus<domain::usecases::GetSettingsUseCase>() << std::endl;
            
            auto themeUseCase = provider.get<domain::usecases::ToggleThemeUseCase>();
            std::cout << "✅ ToggleThemeUseCase resolved from Legacy: " 
                      << (themeUseCase ? "Success" : "Failed") << std::endl;
            std::cout << "   Status: " << provider.getMigrationStatus<domain::usecases::ToggleThemeUseCase>() << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Error accessing services: " << e.what() << std::endl;
        }
        
        std::cout << "\n=== Compile-time System Detection ===" << std::endl;
        std::cout << "FormulaService uses Boost.DI: " 
                  << (provider.isBoostDIManaged<domain::services::IFormulaService>() ? "Yes" : "No") << std::endl;
        std::cout << "GetSettingsUseCase uses Boost.DI: " 
                  << (provider.isBoostDIManaged<domain::usecases::GetSettingsUseCase>() ? "Yes" : "No") << std::endl;
        
        std::cout << "================================================\n" << std::endl;
    }
    
    /// Show how this simplifies ViewModel/Service development
    void demonstrateViewModelUsage() {
        std::cout << "\n=== ViewModel Usage Pattern ===" << std::endl;
        std::cout << "// Before: Need to know which DI system to use" << std::endl;
        std::cout << "auto formulaService = CompositionRoot::getFormulaService();" << std::endl;
        std::cout << "auto settingsUseCase = DependencyProvider::instance().getGetSettingsUseCase();" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "// After: Unified access pattern" << std::endl;
        std::cout << "auto& deps = UnifiedDependencyProvider::instance();" << std::endl;
        std::cout << "auto formulaService = deps.get<IFormulaService>();" << std::endl;
        std::cout << "auto settingsUseCase = deps.get<GetSettingsUseCase>();" << std::endl;
        std::cout << "===============================\n" << std::endl;
    }
};