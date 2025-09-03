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
        // Create the service chain - FormulaRepository now uses shared DB
        // Note: dbPath parameter is ignored, repository uses SqliteDatabase::openDefault()
        auto repository = std::make_shared<data::repositories::FormulaRepository>();
        auto service = std::make_shared<domain::services::FormulaService>(repository);
        
        // Create ViewModel with service injection
        FormulaViewModel formulaVm(service);
        QSignalSpy spy(&formulaVm, &FormulaViewModel::dataChanged);
        
        // Load data (should use service if data is available, otherwise fallback)
        formulaVm.loadData();
        
        // Verify data was loaded (either from service or fallback)
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
        
        // Test that we can find formulas (from either real data or sample data)
        bool foundFormula = false;
        
        for (const auto& node : nodes) {
            if (node.level == 2 && !node.label.isEmpty()) {
                foundFormula = true;
                break;
            }
        }
        
        QVERIFY(foundFormula);
        
        // Test the new fetchFirstCategories method
        auto categories = repository->fetchFirstCategories();
        QVERIFY(categories.size() > 0);
        
        // Should have at least one category
        bool foundCategory = false;
        for (const auto& category : categories) {
            if (!category.empty()) {
                foundCategory = true;
                break;
            }
        }
        QVERIFY(foundCategory);
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
        // Create a service with a repository that may not have data
        auto repository = std::make_shared<data::repositories::FormulaRepository>();
        auto service = std::make_shared<domain::services::FormulaService>(repository);
        
        // Create ViewModel with service injection
        FormulaViewModel formulaVm(service);
        QSignalSpy spy(&formulaVm, &FormulaViewModel::dataChanged);
        
        // Load data (should use service if available, otherwise fallback to sample data)
        formulaVm.loadData();
        
        // Verify data was loaded (via service or fallback)
        QVERIFY(formulaVm.nodeCount() > 0);
        QVERIFY(!formulaVm.nodes().isEmpty());
        QCOMPARE(spy.count(), 1);
    }
};

// Don't use QTEST_MAIN here - will be included in test_main.cpp