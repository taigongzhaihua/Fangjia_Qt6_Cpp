#pragma once
#include "entities/Formula.h"
#include <vector>

namespace domain::repositories
{

/// Repository interface for Formula data access
/// Pure C++ interface - no Qt dependencies
class IFormulaRepository {
public:
    virtual ~IFormulaRepository() = default;

    /// Load all formula nodes from data source
    /// Returns: Vector of formula nodes in hierarchical order
    virtual std::vector<entities::FormulaNode> loadFormulaTree() = 0;

    /// Load specific formula detail by ID
    /// Parameters: formulaId - Unique formula identifier
    /// Returns: Formula detail if found, empty detail with hasDetail=false if not found
    virtual entities::FormulaDetail loadFormulaDetail(const std::string& formulaId) = 0;

    /// Check if the repository is properly initialized and available
    /// Returns: true if data source is available and operational
    virtual bool isAvailable() const = 0;
};

} // namespace domain::repositories