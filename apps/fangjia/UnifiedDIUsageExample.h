#pragma once
#include "UnifiedDependencyProvider.h"
#include <iostream>

/// Example class demonstrating how to use the UnifiedDependencyProvider
/// This shows the improved developer experience with unified DI access
class UnifiedDIUsageExample {
public:
    /// Demonstrate pure Boost.DI service access
    void demonstrateUnifiedAccess() {
        auto& provider = UnifiedDependencyProvider::instance();
        
        std::cout << "\n=== Pure Boost.DI Provider Usage (Phase 4 Complete) ===" << std::endl;
        
        // All services are now managed by Boost.DI through CompositionRoot
        try {
            auto formulaService = provider.get<domain::services::IFormulaService>();
            std::cout << "✅ FormulaService resolved from Boost.DI: " 
                      << (formulaService ? "Success" : "Failed") << std::endl;
            
            auto settingsUseCase = provider.get<domain::usecases::GetSettingsUseCase>();
            std::cout << "✅ GetSettingsUseCase resolved from Boost.DI: " 
                      << (settingsUseCase ? "Success" : "Failed") << std::endl;
            
            auto themeUseCase = provider.get<domain::usecases::ToggleThemeUseCase>();
            std::cout << "✅ ToggleThemeUseCase resolved from Boost.DI: " 
                      << (themeUseCase ? "Success" : "Failed") << std::endl;
            
        } catch (const std::exception& e) {
            std::cerr << "❌ Error accessing services: " << e.what() << std::endl;
        }
        
        std::cout << "\n=== All Services Now Use Pure Boost.DI ===" << std::endl;
        std::cout << "Phase 4 Migration Complete - Legacy DependencyProvider removed!" << std::endl;
        
        std::cout << "================================================\n" << std::endl;
    }
    
    /// Show how this simplifies ViewModel/Service development in Phase 4
    void demonstrateViewModelUsage() {
        std::cout << "\n=== ViewModel Usage Pattern (Phase 4) ===" << std::endl;
        std::cout << "// Before Phase 4: Dual DI systems" << std::endl;
        std::cout << "auto formulaService = CompositionRoot::getFormulaService();" << std::endl;
        std::cout << "auto settingsUseCase = DependencyProvider::instance().getGetSettingsUseCase();" << std::endl;
        std::cout << "" << std::endl;
        std::cout << "// After Phase 4: Pure Boost.DI unified access" << std::endl;
        std::cout << "auto& deps = UnifiedDependencyProvider::instance();" << std::endl;
        std::cout << "auto formulaService = deps.get<IFormulaService>();" << std::endl;
        std::cout << "auto settingsUseCase = deps.get<GetSettingsUseCase>();" << std::endl;
        std::cout << "auto themeUseCase = deps.get<ToggleThemeUseCase>();" << std::endl;
        std::cout << "// All services now use the same pure Boost.DI pattern!" << std::endl;
        std::cout << "===============================\n" << std::endl;
    }
};