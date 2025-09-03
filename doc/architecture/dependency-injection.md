**English** | [简体中文](../../doc.zh-cn/architecture/dependency-injection.md)

# Dependency Injection Design

## Overview

This document describes the Dependency Injection (DI) design strategy of the Fangjia Qt6 C++ project. The project currently adopts a hybrid DI approach, combining Boost.DI container and temporary service locator to provide flexible dependency management for different domains.

## Design Goals

### Core Principles
- **Dependency Inversion**: High-level modules do not depend on low-level modules; both depend on abstractions
- **Inversion of Control**: Object creation and dependency injection managed by container
- **Single Responsibility**: Each component focuses on business logic, not dependency management
- **Testability**: Support unit testing and mocking through interface injection

### Technical Goals
- **Compile-time Safety**: Detect dependency errors at compile time when possible
- **Runtime Performance**: Minimize runtime overhead of dependency resolution
- **Configuration Simplicity**: Provide clear dependency configuration API
- **Progressive Migration**: Support gradual migration from legacy code to DI

## Current DI Architecture

### Dual DI Strategy

The project currently uses two parallel DI systems:

#### 1. Boost.DI Container (CompositionRoot)
- **Scope**: Formula domain-related services
- **Features**: Compile-time type safety, automatic dependency resolution
- **Implementation**: Based on boost-ext/di library

#### 2. Temporary Service Locator (DependencyProvider)
- **Scope**: Settings, Theme-related use cases
- **Features**: Runtime dependency resolution, simple to use
- **Implementation**: Custom singleton service locator

### Architecture Diagram
```
┌─────────────────────────────────────────────────────────┐
│                Application Layer (main.cpp)             │
│  ┌─────────────────┐      ┌─────────────────────────┐   │
│  │  CompositionRoot │      │  DependencyProvider     │   │
│  │  (Boost.DI)     │      │  (Service Locator)      │   │
│  └─────────────────┘      └─────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
                    ↓                      ↓
┌─────────────────────────────────────────────────────────┐
│                     Service Layer                       │
│  ┌─────────────────┐      ┌─────────────────────────┐   │
│  │  FormulaService │      │  Settings UseCases      │   │
│  │  (DI Managed)   │      │  (Locator Managed)      │   │
│  └─────────────────┘      └─────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
                    ↓                      ↓
┌─────────────────────────────────────────────────────────┐
│                     Data Layer                          │
│  ┌─────────────────┐      ┌─────────────────────────┐   │
│  │ FormulaRepository│      │ SettingsRepository      │   │
│  │  (Interface)    │      │ (Concrete)              │   │
│  └─────────────────┘      └─────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## Boost.DI Implementation

### CompositionRoot Design
```cpp
class CompositionRoot {
public:
    /// Create fully configured DI injector
    static auto createInjector() {
        return boost::di::make_injector(
            // Bind interface to implementation
            boost::di::bind<domain::repositories::IFormulaRepository>()
                .to<data::repositories::FormulaRepository>(),
            
            boost::di::bind<domain::services::IFormulaService>()
                .to<domain::services::FormulaService>()
        );
    }
    
    /// Convenience method: Get FormulaService instance
    static std::shared_ptr<domain::services::IFormulaService> getFormulaService() {
        auto injector = createInjector();
        return injector.create<std::shared_ptr<domain::services::IFormulaService>>();
    }
};
```

### Dependency Configuration Example
```cpp
// Configure Formula domain dependencies in main.cpp
auto formulaService = CompositionRoot::getFormulaService();
deps.setFormulaService(formulaService);
```

### Boost.DI Advantages
- **Compile-time Verification**: Dependency cycles and missing dependencies detected at compile time
- **Automatic Resolution**: Constructor parameters automatically injected
- **Type Safety**: Strong type checking reduces runtime errors
- **No Reflection Overhead**: Code generated at compile time, excellent runtime performance

## Service Locator Implementation

### DependencyProvider Design
```cpp
class DependencyProvider {
public:
    static DependencyProvider& instance();

    // Setters - called by composition root
    void setGetSettingsUseCase(std::shared_ptr<GetSettingsUseCase> useCase);
    void setUpdateSettingsUseCase(std::shared_ptr<UpdateSettingsUseCase> useCase);
    void setToggleThemeUseCase(std::shared_ptr<ToggleThemeUseCase> useCase);
    
    // Getters - called by ViewModels
    std::shared_ptr<GetSettingsUseCase> getGetSettingsUseCase() const;
    std::shared_ptr<UpdateSettingsUseCase> getUpdateSettingsUseCase() const;
    std::shared_ptr<ToggleThemeUseCase> getToggleThemeUseCase() const;

private:
    // Store service instances
    std::shared_ptr<GetSettingsUseCase> m_getSettingsUseCase;
    std::shared_ptr<UpdateSettingsUseCase> m_updateSettingsUseCase;
    std::shared_ptr<ToggleThemeUseCase> m_toggleThemeUseCase;
    // ... other services
};
```

### Dependency Configuration Example
```cpp
// Configure Settings-related dependencies in main.cpp
auto settingsRepository = std::make_shared<SettingsRepository>();
auto getSettingsUseCase = std::make_shared<GetSettingsUseCase>(settingsRepository);
auto updateSettingsUseCase = std::make_shared<UpdateSettingsUseCase>(settingsRepository);

auto& deps = DependencyProvider::instance();
deps.setGetSettingsUseCase(getSettingsUseCase);
deps.setUpdateSettingsUseCase(updateSettingsUseCase);
```

### Service Locator Advantages
- **Simple to Use**: Intuitive getter/setter API
- **Progressive Migration**: Can gradually replace hard-coded dependencies
- **Runtime Flexibility**: Support dynamic dependency replacement
- **Minimal Dependencies**: No additional third-party libraries needed

## Dependency Injection Patterns

### Constructor Injection
```cpp
// Recommended: Inject dependencies through constructor
class FormulaService : public IFormulaService {
public:
    explicit FormulaService(std::shared_ptr<IFormulaRepository> repository)
        : m_repository(repository) {}

private:
    std::shared_ptr<IFormulaRepository> m_repository;
};
```

### Service Locator Injection
```cpp
// Temporary: Get dependencies through service locator
class SettingsViewModel {
public:
    void saveSettings(const Settings& settings) {
        auto useCase = DependencyProvider::instance().getUpdateSettingsUseCase();
        useCase->execute(settings);
    }
};
```

### Interface Design Pattern
```cpp
// Define interface in domain layer
namespace domain::repositories {
    class ISettingsRepository {
    public:
        virtual ~ISettingsRepository() = default;
        virtual Settings getSettings() = 0;
        virtual void updateSettings(const Settings& settings) = 0;
    };
}

// Implement interface in data layer
namespace data::repositories {
    class SettingsRepository : public domain::repositories::ISettingsRepository {
    public:
        Settings getSettings() override;
        void updateSettings(const Settings& settings) override;
    };
}
```

## Lifecycle Management

### Singleton Lifecycle
```cpp
// Application-level singleton: Theme manager
auto themeManager = std::make_shared<ThemeManager>(getThemeModeUseCase, setThemeModeUseCase);
// Maintain single instance throughout application lifecycle
```

### Transient Lifecycle
```cpp
// Create new instance each time: Use case objects
auto getSettingsUseCase = std::make_shared<GetSettingsUseCase>(settingsRepository);
// Can create multiple instances as needed
```

### Scoped Lifecycle
```cpp
// Window scope: UI-related services
class MainOpenGlWindow {
public:
    MainOpenGlWindow() {
        // These services are bound to window lifecycle
        m_bindingHost = std::make_unique<BindingHost>();
        m_rebuildHost = std::make_unique<RebuildHost>();
    }
};
```

## Testing Support

### Mock Injection
```cpp
class MockSettingsRepository : public ISettingsRepository {
public:
    MOCK_METHOD(Settings, getSettings, (), (override));
    MOCK_METHOD(void, updateSettings, (const Settings&), (override));
};

TEST(GetSettingsUseCaseTest, ReturnsSettings) {
    auto mockRepo = std::make_shared<MockSettingsRepository>();
    GetSettingsUseCase useCase(mockRepo);
    
    Settings expectedSettings;
    expectedSettings.themeMode = "dark";
    
    EXPECT_CALL(*mockRepo, getSettings())
        .WillOnce(Return(expectedSettings));
    
    auto result = useCase.execute();
    ASSERT_EQ(result.themeMode, "dark");
}
```

### Test Configuration
```cpp
class TestDependencyProvider {
public:
    static void setupTestDependencies() {
        auto& deps = DependencyProvider::instance();
        
        // Inject mock objects for testing
        deps.setGetSettingsUseCase(std::make_shared<MockGetSettingsUseCase>());
        deps.setUpdateSettingsUseCase(std::make_shared<MockUpdateSettingsUseCase>());
    }
};
```

## Migration Strategy

### Current Status
```
✅ Formula Domain → Boost.DI (fully implemented)
🔄 Settings Domain → DependencyProvider (temporary solution)
🚧 Theme Domain → hybrid approach
⏳ New Domains → planned to use Boost.DI
```

### Migration Path
```cpp
// Phase 1: Maintain current dual system
// Phase 2: Prioritize Boost.DI for new features
// Phase 3: Gradually migrate Settings to Boost.DI
// Phase 4: Remove DependencyProvider

// Target architecture (after Phase 4)
auto createUnifiedInjector() {
    return boost::di::make_injector(
        // Formula domain
        boost::di::bind<IFormulaRepository>().to<FormulaRepository>(),
        boost::di::bind<IFormulaService>().to<FormulaService>(),
        
        // Settings domain
        boost::di::bind<ISettingsRepository>().to<SettingsRepository>(),
        boost::di::bind<GetSettingsUseCase>().in(boost::di::singleton),
        boost::di::bind<UpdateSettingsUseCase>().in(boost::di::singleton),
        
        // Theme domain
        boost::di::bind<ThemeManager>().in(boost::di::singleton)
    );
}
```

### Migration Best Practices
1. **Progressive Migration**: Migrate one domain at a time
2. **Interface First**: Define interfaces before migrating implementations
3. **Test Coverage**: Ensure behavior consistency before and after migration
4. **Backward Compatibility**: Keep legacy code working until complete migration

## Performance Considerations

### Boost.DI Performance
- **Compile-time Overhead**: Increases compilation time but excellent runtime performance
- **Code Generation**: Generates optimal object creation code at compile time
- **Memory Efficiency**: Avoids runtime lookup and reflection overhead

### Service Locator Performance
- **Lookup Overhead**: std::map or similar container lookup cost
- **Memory Usage**: Need to store all service instances
- **Thread Safety**: Singleton access may require synchronization overhead

### Optimization Strategies
```cpp
// Cache frequently used services to reduce lookup overhead
class OptimizedDependencyProvider {
private:
    // Use member variables to cache common services
    mutable std::shared_ptr<GetSettingsUseCase> m_cachedGetSettings;
    
public:
    std::shared_ptr<GetSettingsUseCase> getGetSettingsUseCase() const {
        if (!m_cachedGetSettings) {
            m_cachedGetSettings = /* create or lookup */;
        }
        return m_cachedGetSettings;
    }
};
```

## Best Practices

### Dependency Design Principles
1. **Depend on Interfaces Not Implementations**: All dependencies should be abstract interfaces
2. **Minimal Dependency Principle**: Only inject truly needed dependencies
3. **Single Responsibility**: Each service responsible for one clear function
4. **Clear Lifecycle**: Clearly define service lifecycle requirements

### Code Organization
```cpp
// Recommended file organization structure
domain/
  ├── entities/           # Domain entities
  ├── repositories/       # Repository interfaces (defined in domain layer)
  ├── services/          # Domain service interfaces and implementations
  └── usecases/          # Use case implementations

data/
  ├── repositories/      # Repository implementations (implement domain layer interfaces)
  └── sources/           # Data sources

apps/
  ├── CompositionRoot.*  # DI configuration
  └── DependencyProvider.* # Temporary service locator
```

### Error Handling
```cpp
// DI error handling example
class SafeDependencyProvider {
public:
    std::shared_ptr<GetSettingsUseCase> getGetSettingsUseCase() const {
        if (!m_getSettingsUseCase) {
            throw std::runtime_error("GetSettingsUseCase not configured");
        }
        return m_getSettingsUseCase;
    }
};
```

## Related Documentation

- **[System Integration Design](./system-integration.md)** - Integration patterns and data flow between layers
- **[Domain Layer Design](../domain/design.md)** - Dependencies and interface design of domain layer
- **[Architecture Overview](./overview.md)** - Overall architecture design and layering principles
- **[Data Management Overview](../data/overview.md)** - Dependency implementation in data layer