#pragma once
#include "entities/Formula.h"
#include <vector>
#include <memory>

namespace domain::repositories {
    class IFormulaRepository;
}

namespace domain::services
{

/// Formula domain service interface
/// Coordinates business logic for formula operations using repository
class IFormulaService {
public:
    virtual ~IFormulaService() = default;

    /// Load complete formula hierarchy
    /// Returns: Vector of formula nodes in display order
    virtual std::vector<entities::FormulaNode> getFormulaTree() = 0;

    /// Get specific formula details
    /// Parameters: formulaId - Unique formula identifier  
    /// Returns: Formula detail if found, empty detail with hasDetail=false otherwise
    virtual entities::FormulaDetail getFormulaDetail(const std::string& formulaId) = 0;

    /// Check if service can provide data
    /// Returns: true if underlying data source is available
    virtual bool isDataAvailable() const = 0;
};

/// Concrete implementation of formula service
class FormulaService : public IFormulaService {
public:
    explicit FormulaService(std::shared_ptr<repositories::IFormulaRepository> repository);
    
    std::vector<entities::FormulaNode> getFormulaTree() override;
    entities::FormulaDetail getFormulaDetail(const std::string& formulaId) override;
    bool isDataAvailable() const override;

private:
    std::shared_ptr<repositories::IFormulaRepository> m_repository;
};

} // namespace domain::services