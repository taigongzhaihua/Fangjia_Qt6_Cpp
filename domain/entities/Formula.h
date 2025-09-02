#pragma once
#include <string>
#include <vector>

namespace domain::entities
{

/// Pure C++ formula detail entity for domain layer
/// No Qt dependencies - uses standard C++ types only
struct FormulaDetail {
    std::string id;           // Unique identifier
    std::string name;         // Formula name
    std::string source;       // Source text (e.g., "《伤寒论》")
    std::string composition;  // Drug composition
    std::string usage;        // Usage instructions
    std::string function;     // Effects and functions
    std::string indication;   // Main indications
    std::string note;        // Additional notes
    bool hasDetail;          // Whether this detail is populated
    
    FormulaDetail() : hasDetail(false) {}
};

/// Formula tree node entity for hierarchical structure
struct FormulaNode {
    std::string id;           // Unique identifier
    std::string label;        // Display label
    int level;               // 0=category, 1=subcategory, 2=formula
    std::string parentId;     // Parent node ID (empty for root nodes)
    
    // Optional detail - only populated for leaf nodes (formulas)
    FormulaDetail detail;
    bool hasDetail;          // Whether detail is populated
    
    FormulaNode() : level(0), hasDetail(false) {}
};

} // namespace domain::entities