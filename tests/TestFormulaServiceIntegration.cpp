#include <QtTest>
#include <QSignalSpy>
#include "presentation/viewmodels/FormulaViewModel.h"
#include "FormulaRepository.h"
#include "services/FormulaService.h"
#include <QTemporaryDir>

class TestFormulaServiceIntegration : public QObject
{
    Q_OBJECT

public slots:
    void testServiceIntegration()
    {
        // Create a temporary directory for the test database
        QTemporaryDir tempDir;
        QVERIFY(tempDir.isValid());
        
        QString dbPath = tempDir.filePath("test_formulas.db");
        
        // Create the service chain
        auto repository = std::make_shared<data::repositories::FormulaRepository>(dbPath);
        auto service = std::make_shared<domain::services::FormulaService>(repository);
        
        // Create ViewModel with service injection
        FormulaViewModel formulaVm(service);
        QSignalSpy spy(&formulaVm, &FormulaViewModel::dataChanged);
        
        // Load data (should use service)
        formulaVm.loadData();
        
        // Verify data was loaded
        QVERIFY(formulaVm.nodeCount() > 0);
        QVERIFY(!formulaVm.nodes().isEmpty());
        QCOMPARE(spy.count(), 1);
        
        // Verify structure has categories, subcategories, and formulas
        const auto& nodes = formulaVm.nodes();
        bool hasCategory = false;
        bool hasSubCategory = false;
        bool hasFormula = false;
        
        for (const auto& node : nodes) {
            if (node.level == 0) hasCategory = true;
            else if (node.level == 1) hasSubCategory = true;
            else if (node.level == 2) hasFormula = true;
        }
        
        QVERIFY(hasCategory);
        QVERIFY(hasSubCategory);
        QVERIFY(hasFormula);
        
        // Test that we can find specific formulas from the database
        bool foundMahuang = false;
        bool foundGuizhi = false;
        
        for (const auto& node : nodes) {
            if (node.label.contains("麻黄汤")) {
                foundMahuang = true;
            }
            if (node.label.contains("桂枝汤")) {
                foundGuizhi = true;
            }
        }
        
        QVERIFY(foundMahuang);
        QVERIFY(foundGuizhi);
    }
    
    void testFallbackToSampleData()
    {
        // Create ViewModel without service injection (fallback mode)
        FormulaViewModel formulaVm;
        QSignalSpy spy(&formulaVm, &FormulaViewModel::dataChanged);
        
        // Load data (should fallback to sample data)
        formulaVm.loadData();
        
        // Verify data was loaded
        QVERIFY(formulaVm.nodeCount() > 0);
        QVERIFY(!formulaVm.nodes().isEmpty());
        QCOMPARE(spy.count(), 1);
        
        // This should work exactly like the old loadSampleData()
        const auto& nodes = formulaVm.nodes();
        bool hasCategory = false;
        bool hasSubCategory = false;
        bool hasFormula = false;
        
        for (const auto& node : nodes) {
            if (node.level == 0) hasCategory = true;
            else if (node.level == 1) hasSubCategory = true;
            else if (node.level == 2) hasFormula = true;
        }
        
        QVERIFY(hasCategory);
        QVERIFY(hasSubCategory);
        QVERIFY(hasFormula);
    }
    
    void testServiceUnavailableFallback()
    {
        // Create a service with invalid repository
        auto repository = std::make_shared<data::repositories::FormulaRepository>("/invalid/path/db.sqlite");
        auto service = std::make_shared<domain::services::FormulaService>(repository);
        
        // Create ViewModel with service injection
        FormulaViewModel formulaVm(service);
        QSignalSpy spy(&formulaVm, &FormulaViewModel::dataChanged);
        
        // Load data (should fallback to sample data when service is unavailable)
        formulaVm.loadData();
        
        // Verify data was still loaded (via fallback)
        QVERIFY(formulaVm.nodeCount() > 0);
        QVERIFY(!formulaVm.nodes().isEmpty());
        QCOMPARE(spy.count(), 1);
    }
};

// Don't use QTEST_MAIN here - will be included in test_main.cpp