#include "CompositionRoot.h"
#include "services/FormulaService.h"
#include "repositories/IFormulaRepository.h"
#include "FormulaRepository.h"
#include "ServiceRegistry.h"
#include <boost/di.hpp>

namespace di = boost::di;

auto CompositionRoot::configureInjector()
{
    return di::make_injector(
        // Bind IFormulaRepository to FormulaRepository implementation
        di::bind<domain::repositories::IFormulaRepository>().to<data::repositories::FormulaRepository>().in(di::singleton),
        
        // Bind IFormulaService to FormulaService implementation
        di::bind<domain::services::IFormulaService>().to<domain::services::FormulaService>().in(di::singleton)
    );
}

auto CompositionRoot::createInjector()
{
    return configureInjector();
}

std::shared_ptr<domain::services::IFormulaService> CompositionRoot::getFormulaService()
{
    auto injector = createInjector();
    auto service = injector.template create<std::shared_ptr<domain::services::IFormulaService>>();
    
    // Configure the global ServiceRegistry with the DI-created service
    domain::ServiceRegistry::instance().setFormulaService(service);
    
    return service;
}