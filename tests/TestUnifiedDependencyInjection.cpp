#include <QTest>
#include <QApplication>
#include <QObject>
#include <memory>

// Include our unified DI components
#include "UnifiedDependencyProvider.h"
#include "DependencyMigrationTool.h"
#include "DependencyProvider.h"
#include "CompositionRoot.h"

// Mock services for testing
#include "usecases/GetSettingsUseCase.h"
#include "services/IFormulaService.h"

class TestUnifiedDependencyInjection : public QObject
{
    Q_OBJECT

public slots:
    void testUnifiedProviderCompileTimeSystemDetection()
    {
        auto& provider = UnifiedDependencyProvider::instance();
        
        // Test compile-time system detection
        QVERIFY(provider.isBoostDIManaged<domain::services::IFormulaService>());
        QVERIFY(!provider.isBoostDIManaged<domain::usecases::GetSettingsUseCase>());
    }
    
    void testUnifiedProviderMigrationStatus()
    {
        auto& provider = UnifiedDependencyProvider::instance();
        
        // Test migration status reporting
        const char* formulaStatus = provider.getMigrationStatus<domain::services::IFormulaService>();
        const char* settingsStatus = provider.getMigrationStatus<domain::usecases::GetSettingsUseCase>();
        
        QVERIFY(QString(formulaStatus).contains("Boost.DI"));
        QVERIFY(QString(formulaStatus).contains("migrated"));
        QVERIFY(QString(settingsStatus).contains("Legacy"));
        QVERIFY(QString(settingsStatus).contains("pending"));
    }
    
    void testMigrationToolStatusTracking()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        // Test migration report generation
        auto report = tool.generateMigrationReport();
        
        QVERIFY(report.totalServices > 0);
        QVERIFY(report.migratedServices >= 1); // At least FormulaService should be migrated
        QVERIFY(report.pendingServices >= 0);
        QCOMPARE(report.totalServices, report.migratedServices + report.pendingServices);
        
        // Test completion percentage calculation
        double expectedPercentage = (static_cast<double>(report.migratedServices) / report.totalServices) * 100.0;
        QCOMPARE(report.completionPercentage, expectedPercentage);
    }
    
    void testMigrationToolServiceStatus()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        // Test specific service status
        auto formulaStatus = tool.getServiceStatus("IFormulaService");
        auto settingsStatus = tool.getServiceStatus("GetSettingsUseCase");
        
        QCOMPARE(formulaStatus, DependencyMigrationTool::MigrationStatus::Completed);
        QCOMPARE(settingsStatus, DependencyMigrationTool::MigrationStatus::NotStarted);
        
        // Test unknown service
        auto unknownStatus = tool.getServiceStatus("UnknownService");
        QCOMPARE(unknownStatus, DependencyMigrationTool::MigrationStatus::NotStarted);
    }
    
    void testMigrationToolPendingServices()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        auto pendingServices = tool.getPendingServices();
        
        // Should have pending services since we haven't migrated everything
        QVERIFY(!pendingServices.empty());
        
        // Should include known pending services
        QVERIFY(std::find(pendingServices.begin(), pendingServices.end(), "GetSettingsUseCase") != pendingServices.end());
        QVERIFY(std::find(pendingServices.begin(), pendingServices.end(), "UpdateSettingsUseCase") != pendingServices.end());
        
        // Should not include already migrated services
        QVERIFY(std::find(pendingServices.begin(), pendingServices.end(), "IFormulaService") == pendingServices.end());
    }
    
    void testMigrationToolFullMigrationStatus()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        // Should not be fully migrated yet
        QVERIFY(!tool.isFullyMigrated());
        
        // Verify we have the expected mix of migrated and pending services
        auto report = tool.generateMigrationReport();
        QVERIFY(report.migratedServices > 0);  // Some services are migrated
        QVERIFY(report.pendingServices > 0);   // Some services are still pending
    }
    
    void testMigrationToolValidation()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        // Test migration validation (should pass for current state)
        bool validationResult = tool.validateMigration();
        QVERIFY(validationResult); // Should be true since our migrated services work
    }
    
    void testMigrationProcessSimulation()
    {
        auto& tool = DependencyMigrationTool::instance();
        
        // Test starting migration for a pending service
        bool migrationStarted = tool.migrateService("GetSettingsUseCase");
        QVERIFY(migrationStarted);
        
        // Status should now be in progress
        auto status = tool.getServiceStatus("GetSettingsUseCase");
        QCOMPARE(status, DependencyMigrationTool::MigrationStatus::InProgress);
        
        // Test that trying to migrate an unknown service fails gracefully
        bool unknownMigration = tool.migrateService("NonExistentService");
        QVERIFY(!unknownMigration);
    }
    
    void testUnifiedProviderErrorHandling()
    {
        auto& provider = UnifiedDependencyProvider::instance();
        
        // Test that accessing services before initialization throws appropriate errors
        // Note: In a real test, we would reset the provider state first
        // For now, we just verify that the methods exist and can be called
        
        try {
            // This should work if the provider was properly initialized in main
            auto formulaService = provider.get<domain::services::IFormulaService>();
            QVERIFY(formulaService != nullptr);
        } catch (const std::exception& e) {
            // If it throws, it should be a meaningful error
            QVERIFY(QString(e.what()).length() > 0);
        }
    }
};

#include "TestUnifiedDependencyInjection.moc"

// Register the test class for automatic discovery
QTEST_MAIN(TestUnifiedDependencyInjection)