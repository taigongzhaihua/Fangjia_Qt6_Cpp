#include <QTest>
#include <QApplication>
#include <QObject>
#include <memory>

// Include our pure Boost.DI components - Phase 4 complete
#include "UnifiedDependencyProvider.h"
#include "DependencyMigrationTool.h"
#include "CompositionRoot.h"

// Mock services for testing
#include "usecases/GetSettingsUseCase.h"
#include "services/IFormulaService.h"

class TestUnifiedDependencyInjection : public QObject
{
    Q_OBJECT

public slots:
    void testUnifiedProviderPureBoostDI()
    {
        auto& provider = UnifiedDependencyProvider::instance();
        
        // Test that all services can be resolved through pure Boost.DI
        try {
            auto formulaService = provider.get<domain::services::IFormulaService>();
            QVERIFY(formulaService != nullptr);
            
            auto settingsUseCase = provider.get<domain::usecases::GetSettingsUseCase>();
            QVERIFY(settingsUseCase != nullptr);
        } catch (const std::exception& e) {
            QFAIL(QString("Failed to resolve services: %1").arg(e.what()).toLatin1());
        }
    }
    
    void testMigrationToolPhase4Complete()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        // Test migration report generation - Phase 4 should show completion
        auto report = tool.generateMigrationReport();
        
        QVERIFY(report.totalServices > 0);
        // All services should be migrated in Phase 4
        QCOMPARE(report.migratedServices, report.totalServices);
        QCOMPARE(report.pendingServices, 0);
        QCOMPARE(report.completionPercentage, 100.0);
    }
    
    void testMigrationToolAllServicesCompleted()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        // Test that all services are marked as completed
        auto formulaStatus = tool.getServiceStatus("IFormulaService");
        auto settingsStatus = tool.getServiceStatus("GetSettingsUseCase");
        
        QCOMPARE(formulaStatus, DependencyMigrationTool::MigrationStatus::Completed);
        QCOMPARE(settingsStatus, DependencyMigrationTool::MigrationStatus::Completed);
        
        // Test unknown service
        auto unknownStatus = tool.getServiceStatus("UnknownService");
        QCOMPARE(unknownStatus, DependencyMigrationTool::MigrationStatus::NotStarted);
    }
    
    void testMigrationToolNoPendingServices()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        auto pendingServices = tool.getPendingServices();
        
        // Should have no pending services since Phase 4 is complete
        QVERIFY(pendingServices.empty());
    
    void testMigrationToolFullyMigrated()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        // Should be fully migrated in Phase 4
        QVERIFY(tool.isFullyMigrated());
        
        // Verify all services are migrated and no pending services
        auto report = tool.generateMigrationReport();
        QVERIFY(report.migratedServices > 0);
        QCOMPARE(report.pendingServices, 0);
    }
    
    void testMigrationToolValidation()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        // Test migration validation (should pass for Phase 4 complete state)
        bool validationResult = tool.validateMigration();
        QVERIFY(validationResult); // Should be true since all services are migrated
    }
    
    void testUnifiedProviderErrorHandling()
    {
        auto& provider = UnifiedDependencyProvider::instance();
        
        // Test that accessing services works correctly with pure Boost.DI
        try {
            // This should work since all services are now in Boost.DI
            auto formulaService = provider.get<domain::services::IFormulaService>();
            QVERIFY(formulaService != nullptr);
            
            auto settingsUseCase = provider.get<domain::usecases::GetSettingsUseCase>();
            QVERIFY(settingsUseCase != nullptr);
        } catch (const std::exception& e) {
            // If it throws, log the error for debugging
            qDebug() << "Error accessing services:" << e.what();
            // In Phase 4, this should not happen
            QFAIL("Services should be accessible through pure Boost.DI");
        }
    }
};

#include "TestUnifiedDependencyInjection.moc"

// Register the test class for automatic discovery
QTEST_MAIN(TestUnifiedDependencyInjection)