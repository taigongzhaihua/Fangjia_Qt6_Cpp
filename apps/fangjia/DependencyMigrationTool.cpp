#include "DependencyMigrationTool.h"
#include <iostream>
#include <iomanip>

DependencyMigrationTool& DependencyMigrationTool::instance()
{
    static DependencyMigrationTool instance;
    return instance;
}

DependencyMigrationTool::DependencyMigrationTool()
{
    initializeKnownServices();
}

void DependencyMigrationTool::initializeKnownServices()
{
    // Initialize tracking for all known services based on current architecture
    trackService("IFormulaService", "Boost.DI", "Boost.DI");
    markServiceMigrated("IFormulaService"); // Already migrated
    
    // Services migrated to Boost.DI in Phase 3
    trackService("GetSettingsUseCase", "Boost.DI", "Boost.DI");
    markServiceMigrated("GetSettingsUseCase");
    
    trackService("UpdateSettingsUseCase", "Boost.DI", "Boost.DI");
    markServiceMigrated("UpdateSettingsUseCase");
    
    trackService("ToggleThemeUseCase", "Boost.DI", "Boost.DI");
    markServiceMigrated("ToggleThemeUseCase");
    
    trackService("GetThemeModeUseCase", "Boost.DI", "Boost.DI");
    markServiceMigrated("GetThemeModeUseCase");
    
    trackService("SetThemeModeUseCase", "Boost.DI", "Boost.DI");
    markServiceMigrated("SetThemeModeUseCase");
    
    trackService("GetRecentTabUseCase", "Boost.DI", "Boost.DI");
    markServiceMigrated("GetRecentTabUseCase");
    
    trackService("SetRecentTabUseCase", "Boost.DI", "Boost.DI");
    markServiceMigrated("SetRecentTabUseCase");
}

bool DependencyMigrationTool::migrateService(const std::string& serviceName)
{
    auto it = m_serviceInfo.find(serviceName);
    if (it == m_serviceInfo.end()) {
        std::cerr << "Unknown service: " << serviceName << std::endl;
        return false;
    }

    auto& info = it->second;
    if (info.status == MigrationStatus::Completed) {
        std::cout << "Service " << serviceName << " already migrated." << std::endl;
        return true;
    }

    if (info.currentSystem == "Boost.DI") {
        std::cout << "Service " << serviceName << " is already using Boost.DI." << std::endl;
        markServiceMigrated(serviceName);
        return true;
    }

    // For now, just mark as in progress - actual migration would involve:
    // 1. Creating Boost.DI bindings
    // 2. Updating service instantiation
    // 3. Removing from legacy provider
    info.status = MigrationStatus::InProgress;
    info.notes = "Migration started but requires manual implementation";
    
    std::cout << "Started migration for service: " << serviceName << std::endl;
    std::cout << "Manual steps required:" << std::endl;
    std::cout << "1. Add Boost.DI binding in CompositionRoot::configureInjector()" << std::endl;
    std::cout << "2. Update service instantiation to use Boost.DI" << std::endl;
    std::cout << "3. Update template specializations in UnifiedDependencyProvider" << std::endl;
    std::cout << "Note: DependencyProvider has been removed in Phase 4" << std::endl;
    
    return true;
}

bool DependencyMigrationTool::validateMigration()
{
    bool allValid = true;
    for (const auto& [serviceName, info] : m_serviceInfo) {
        if (info.status == MigrationStatus::Completed) {
            if (!validateService(serviceName)) {
                std::cerr << "Validation failed for migrated service: " << serviceName << std::endl;
                allValid = false;
            }
        }
    }
    return allValid;
}

DependencyMigrationTool::MigrationReport DependencyMigrationTool::generateMigrationReport()
{
    MigrationReport report;
    
    for (const auto& [serviceName, info] : m_serviceInfo) {
        report.services.push_back(info);
        report.totalServices++;
        
        if (info.status == MigrationStatus::Completed) {
            report.migratedServices++;
        } else {
            report.pendingServices++;
        }
    }
    
    if (report.totalServices > 0) {
        report.completionPercentage = (static_cast<double>(report.migratedServices) / report.totalServices) * 100.0;
    }
    
    // Print report to console
    std::cout << "\n=== Dependency Injection Migration Report ===" << std::endl;
    std::cout << "Total Services: " << report.totalServices << std::endl;
    std::cout << "Migrated: " << report.migratedServices << std::endl;
    std::cout << "Pending: " << report.pendingServices << std::endl;
    std::cout << "Completion: " << std::fixed << std::setprecision(1) << report.completionPercentage << "%" << std::endl;
    std::cout << "\nService Details:" << std::endl;
    
    for (const auto& service : report.services) {
        std::string statusStr;
        switch (service.status) {
            case MigrationStatus::NotStarted: statusStr = "⏳ Not Started"; break;
            case MigrationStatus::InProgress: statusStr = "🔄 In Progress"; break;
            case MigrationStatus::Completed: statusStr = "✅ Completed"; break;
            case MigrationStatus::Failed: statusStr = "❌ Failed"; break;
        }
        
        std::cout << "  " << service.serviceName 
                  << " [" << service.currentSystem << " → " << service.targetSystem << "] "
                  << statusStr << std::endl;
        
        if (!service.notes.empty()) {
            std::cout << "    Notes: " << service.notes << std::endl;
        }
    }
    std::cout << "============================================\n" << std::endl;
    
    return report;
}

void DependencyMigrationTool::trackService(const std::string& serviceName, const std::string& currentSystem, const std::string& targetSystem)
{
    ServiceMigrationInfo info;
    info.serviceName = serviceName;
    info.currentSystem = currentSystem;
    info.targetSystem = targetSystem;
    info.status = (currentSystem == targetSystem) ? MigrationStatus::Completed : MigrationStatus::NotStarted;
    
    m_serviceInfo[serviceName] = info;
}

void DependencyMigrationTool::markServiceMigrated(const std::string& serviceName)
{
    auto it = m_serviceInfo.find(serviceName);
    if (it != m_serviceInfo.end()) {
        it->second.status = MigrationStatus::Completed;
        it->second.currentSystem = it->second.targetSystem;
        it->second.notes = "Migration completed successfully";
    }
}

DependencyMigrationTool::MigrationStatus DependencyMigrationTool::getServiceStatus(const std::string& serviceName) const
{
    auto it = m_serviceInfo.find(serviceName);
    return (it != m_serviceInfo.end()) ? it->second.status : MigrationStatus::NotStarted;
}

bool DependencyMigrationTool::isFullyMigrated() const
{
    for (const auto& [serviceName, info] : m_serviceInfo) {
        if (info.status != MigrationStatus::Completed) {
            return false;
        }
    }
    return true;
}

std::vector<std::string> DependencyMigrationTool::getPendingServices() const
{
    std::vector<std::string> pending;
    for (const auto& [serviceName, info] : m_serviceInfo) {
        if (info.status != MigrationStatus::Completed) {
            pending.push_back(serviceName);
        }
    }
    return pending;
}

bool DependencyMigrationTool::validateService(const std::string& serviceName)
{
    // Basic validation - in a real implementation, this would:
    // 1. Try to resolve the service through the new system
    // 2. Check that it returns a valid instance
    // 3. Verify basic functionality
    
    // For now, just return true as a placeholder
    return true;
}