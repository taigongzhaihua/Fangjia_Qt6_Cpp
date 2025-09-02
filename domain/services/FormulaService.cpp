#include "FormulaService.h"
#include "repositories/IFormulaRepository.h"

namespace domain::services
{

FormulaService::FormulaService(std::shared_ptr<repositories::IFormulaRepository> repository)
    : m_repository(std::move(repository))
{
}

std::vector<entities::FormulaNode> FormulaService::getFormulaTree()
{
    if (!m_repository || !m_repository->isAvailable()) {
        return {};
    }
    
    return m_repository->loadFormulaTree();
}

entities::FormulaDetail FormulaService::getFormulaDetail(const std::string& formulaId)
{
    if (!m_repository || !m_repository->isAvailable()) {
        entities::FormulaDetail empty;
        empty.hasDetail = false;
        return empty;
    }
    
    return m_repository->loadFormulaDetail(formulaId);
}

bool FormulaService::isDataAvailable() const
{
    return m_repository && m_repository->isAvailable();
}

} // namespace domain::services