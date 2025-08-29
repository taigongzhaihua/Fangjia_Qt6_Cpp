#include "DependencyProvider.h"
#include "usecases/GetSettingsUseCase.h"
#include "usecases/UpdateSettingsUseCase.h"
#include "usecases/ToggleThemeUseCase.h"

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