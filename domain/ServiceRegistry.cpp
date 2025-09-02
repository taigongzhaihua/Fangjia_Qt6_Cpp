#include "ServiceRegistry.h"
#include "services/FormulaService.h"

namespace domain {

ServiceRegistry& ServiceRegistry::instance()
{
    static ServiceRegistry instance;
    return instance;
}

void ServiceRegistry::setFormulaService(std::shared_ptr<services::IFormulaService> service)
{
    m_formulaService = std::move(service);
}

std::shared_ptr<services::IFormulaService> ServiceRegistry::getFormulaService() const
{
    return m_formulaService;
}

} // namespace domain