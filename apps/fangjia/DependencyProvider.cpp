#include "DependencyProvider.h"
#include "usecases/GetSettingsUseCase.h"
#include "usecases/UpdateSettingsUseCase.h"
#include "usecases/ToggleThemeUseCase.h"
#include "usecases/GetThemeModeUseCase.h"
#include "usecases/SetThemeModeUseCase.h"
#include "usecases/GetRecentTabUseCase.h"
#include "usecases/SetRecentTabUseCase.h"

DependencyProvider& DependencyProvider::instance()
{
    static DependencyProvider instance;
    return instance;
}

void DependencyProvider::setGetSettingsUseCase(std::shared_ptr<domain::usecases::GetSettingsUseCase> useCase)
{
    m_getSettingsUseCase = std::move(useCase);
}

void DependencyProvider::setUpdateSettingsUseCase(std::shared_ptr<domain::usecases::UpdateSettingsUseCase> useCase)
{
    m_updateSettingsUseCase = std::move(useCase);
}

void DependencyProvider::setToggleThemeUseCase(std::shared_ptr<domain::usecases::ToggleThemeUseCase> useCase)
{
    m_toggleThemeUseCase = std::move(useCase);
}

void DependencyProvider::setGetThemeModeUseCase(std::shared_ptr<domain::usecases::GetThemeModeUseCase> useCase)
{
    m_getThemeModeUseCase = std::move(useCase);
}

void DependencyProvider::setSetThemeModeUseCase(std::shared_ptr<domain::usecases::SetThemeModeUseCase> useCase)
{
    m_setThemeModeUseCase = std::move(useCase);
}

void DependencyProvider::setGetRecentTabUseCase(std::shared_ptr<domain::usecases::GetRecentTabUseCase> useCase)
{
    m_getRecentTabUseCase = std::move(useCase);
}

void DependencyProvider::setSetRecentTabUseCase(std::shared_ptr<domain::usecases::SetRecentTabUseCase> useCase)
{
    m_setRecentTabUseCase = std::move(useCase);
}

void DependencyProvider::setFormulaService(std::shared_ptr<domain::services::IFormulaService> service)
{
    m_formulaService = std::move(service);
}

std::shared_ptr<domain::usecases::GetSettingsUseCase> DependencyProvider::getGetSettingsUseCase() const
{
    return m_getSettingsUseCase;
}

std::shared_ptr<domain::usecases::UpdateSettingsUseCase> DependencyProvider::getUpdateSettingsUseCase() const
{
    return m_updateSettingsUseCase;
}

std::shared_ptr<domain::usecases::ToggleThemeUseCase> DependencyProvider::getToggleThemeUseCase() const
{
    return m_toggleThemeUseCase;
}

std::shared_ptr<domain::usecases::GetThemeModeUseCase> DependencyProvider::getGetThemeModeUseCase() const
{
    return m_getThemeModeUseCase;
}

std::shared_ptr<domain::usecases::SetThemeModeUseCase> DependencyProvider::getSetThemeModeUseCase() const
{
    return m_setThemeModeUseCase;
}

std::shared_ptr<domain::usecases::GetRecentTabUseCase> DependencyProvider::getGetRecentTabUseCase() const
{
    return m_getRecentTabUseCase;
}

std::shared_ptr<domain::usecases::SetRecentTabUseCase> DependencyProvider::getSetRecentTabUseCase() const
{
    return m_setRecentTabUseCase;
}

std::shared_ptr<domain::services::IFormulaService> DependencyProvider::getFormulaService() const
{
    return m_formulaService;
}