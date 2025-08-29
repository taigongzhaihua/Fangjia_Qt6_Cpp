#include "FakeSettingsRepository.h"
#include "usecases/GetSettingsUseCase.h"
#include "usecases/UpdateSettingsUseCase.h"
#include "usecases/ToggleThemeUseCase.h"
#include "entities/Settings.h"
#include <cassert>
#include <iostream>
#include <memory>

namespace tests {

void testGetSettingsUseCase() {
    std::cout << "=== Testing GetSettingsUseCase ===" << std::endl;
    
    // Arrange
    domain::entities::Settings initialSettings;
    initialSettings.themeMode = "dark";
    initialSettings.recentTab = "herb";
    auto repository = std::make_shared<FakeSettingsRepository>(initialSettings);
    domain::usecases::GetSettingsUseCase useCase(repository);
    
    // Act
    const auto result = useCase.execute();
    
    // Assert
    assert(result.themeMode == "dark");
    assert(result.recentTab == "herb");
    
    std::cout << "GetSettingsUseCase test PASSED ✅" << std::endl;
}

void testUpdateSettingsUseCase() {
    std::cout << "=== Testing UpdateSettingsUseCase ===" << std::endl;
    
    // Arrange
    auto repository = std::make_shared<FakeSettingsRepository>();
    domain::usecases::UpdateSettingsUseCase useCase(repository);
    
    domain::entities::Settings newSettings;
    newSettings.themeMode = "light";
    newSettings.recentTab = "formula";
    newSettings.navExpanded = false;
    
    // Act
    useCase.execute(newSettings);
    
    // Assert
    const auto stored = repository->getSettings();
    assert(stored.themeMode == "light");
    assert(stored.recentTab == "formula");
    assert(stored.navExpanded == false);
    assert(repository->wasSaveCalled());
    
    std::cout << "UpdateSettingsUseCase test PASSED ✅" << std::endl;
}

void testToggleThemeUseCase() {
    std::cout << "=== Testing ToggleThemeUseCase ===" << std::endl;
    
    // Arrange
    domain::entities::Settings initialSettings;
    initialSettings.themeMode = "system";
    auto repository = std::make_shared<FakeSettingsRepository>(initialSettings);
    domain::usecases::ToggleThemeUseCase useCase(repository);
    
    // Act & Assert: system -> light
    std::string result1 = useCase.execute();
    assert(result1 == "light");
    assert(repository->getSettings().themeMode == "light");
    
    // Act & Assert: light -> dark
    std::string result2 = useCase.execute();
    assert(result2 == "dark");
    assert(repository->getSettings().themeMode == "dark");
    
    // Act & Assert: dark -> system
    std::string result3 = useCase.execute();
    assert(result3 == "system");
    assert(repository->getSettings().themeMode == "system");
    
    std::cout << "ToggleThemeUseCase test PASSED ✅" << std::endl;
}

void runDomainTests() {
    std::cout << "==========================================" << std::endl;
    std::cout << "Domain Layer Use Case Tests" << std::endl;
    std::cout << "==========================================" << std::endl;
    
    testGetSettingsUseCase();
    testUpdateSettingsUseCase();
    testToggleThemeUseCase();
    
    std::cout << "==========================================" << std::endl;
    std::cout << "ALL DOMAIN TESTS PASSED ✅" << std::endl;
    std::cout << "==========================================" << std::endl;
}

} // namespace tests