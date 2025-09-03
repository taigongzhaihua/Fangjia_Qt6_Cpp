#include "AppConfig.h"
#include "CompositionRoot.h"
#include "FormulaRepository.h"
#include "FormulaService.h"
#include "GetRecentTabUseCase.h"
#include "GetSettingsUseCase.h"
#include "GetThemeModeUseCase.h"
#include "IFormulaRepository.h"
#include "ISettingsRepository.h"
#include "ServiceRegistry.h"
#include "SetRecentTabUseCase.h"
#include "SetThemeModeUseCase.h"
#include "SettingsRepository.h"
#include "ToggleThemeUseCase.h"
#include "UpdateSettingsUseCase.h"
#include <boost/di.hpp>
#include <memory>

namespace di = boost::di;

auto CompositionRoot::configureInjector()
{
	// Create AppConfig instance manually (requires Qt context)
	static auto appConfig = []
		{
			auto config = std::make_shared<AppConfig>();
			config->load();
			return config;
		}();

	return di::make_injector(
		// Formula domain bindings
		di::bind<domain::repositories::IFormulaRepository>().to<data::repositories::FormulaRepository>().in(di::singleton),
		di::bind<domain::services::IFormulaService>().to<domain::services::FormulaService>().in(di::singleton),

		// Settings domain bindings - bind AppConfig instance
		di::bind<AppConfig>().to(appConfig),
		di::bind<domain::repositories::ISettingsRepository>().to<data::repositories::SettingsRepository>().in(di::singleton),
		di::bind<domain::usecases::GetSettingsUseCase>().in(di::singleton),
		di::bind<domain::usecases::UpdateSettingsUseCase>().in(di::singleton),
		di::bind<domain::usecases::GetThemeModeUseCase>().in(di::singleton),
		di::bind<domain::usecases::SetThemeModeUseCase>().in(di::singleton),
		di::bind<domain::usecases::ToggleThemeUseCase>().in(di::singleton),
		di::bind<domain::usecases::GetRecentTabUseCase>().in(di::singleton),
		di::bind<domain::usecases::SetRecentTabUseCase>().in(di::singleton)
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

std::shared_ptr<domain::repositories::ISettingsRepository> CompositionRoot::getSettingsRepository()
{
	auto injector = createInjector();
	return injector.template create<std::shared_ptr<domain::repositories::ISettingsRepository>>();
}

std::shared_ptr<domain::usecases::GetSettingsUseCase> CompositionRoot::getGetSettingsUseCase()
{
	auto injector = createInjector();
	return injector.template create<std::shared_ptr<domain::usecases::GetSettingsUseCase>>();
}

std::shared_ptr<domain::usecases::UpdateSettingsUseCase> CompositionRoot::getUpdateSettingsUseCase()
{
	auto injector = createInjector();
	return injector.template create<std::shared_ptr<domain::usecases::UpdateSettingsUseCase>>();
}

std::shared_ptr<domain::usecases::GetThemeModeUseCase> CompositionRoot::getGetThemeModeUseCase()
{
	auto injector = createInjector();
	return injector.template create<std::shared_ptr<domain::usecases::GetThemeModeUseCase>>();
}

std::shared_ptr<domain::usecases::SetThemeModeUseCase> CompositionRoot::getSetThemeModeUseCase()
{
	auto injector = createInjector();
	return injector.template create<std::shared_ptr<domain::usecases::SetThemeModeUseCase>>();
}

std::shared_ptr<domain::usecases::ToggleThemeUseCase> CompositionRoot::getToggleThemeUseCase()
{
	auto injector = createInjector();
	return injector.template create<std::shared_ptr<domain::usecases::ToggleThemeUseCase>>();
}

std::shared_ptr<domain::usecases::GetRecentTabUseCase> CompositionRoot::getGetRecentTabUseCase()
{
	auto injector = createInjector();
	return injector.template create<std::shared_ptr<domain::usecases::GetRecentTabUseCase>>();
}

std::shared_ptr<domain::usecases::SetRecentTabUseCase> CompositionRoot::getSetRecentTabUseCase()
{
	auto injector = createInjector();
	return injector.template create<std::shared_ptr<domain::usecases::SetRecentTabUseCase>>();
}