**English** | [简体中文](../../doc.zh-cn/domain/design.md)

# Domain Layer Design

## Overview

The Domain Layer is the core of the Fangjia Qt6 C++ project, implementing the business logic of Clean Architecture. This layer is completely independent of external frameworks and infrastructure, implemented in pure C++, ensuring testability and maintainability of business logic.

## Design Principles

### Dependency Inversion
- Domain layer does not depend on any external frameworks (including Qt)
- Defines contracts with outer layers through interfaces
- Outer layers provide concrete implementations via dependency injection

### Single Responsibility
- Each component is responsible for a single business function
- Entities handle business data, services handle business logic, use cases handle application scenarios

### Open/Closed Principle
- Supports feature extension through interface abstraction
- Adding new business features doesn't require modifying existing code

## Architecture Components

### Entity Layer

Entities represent core business objects, containing business data and basic validation logic.

#### Core Entities

**Formula-related Entities**
- `FormulaDetail`: Traditional Chinese medicine formula detailed information
- `FormulaNode`: Formula tree hierarchy node
- `Drug`: Basic drug information
- `Formulation`: Formula combination information
- `Category`: Category information

**System Configuration Entities**
- `Settings`: Application settings
- `Theme`: Theme configuration

#### Design Features
```cpp
// Pure C++ design, no Qt dependencies
struct FormulaDetail {
    std::string id;
    std::string name;
    std::string composition;
    // ... other fields
};
```

### Repository Layer

Repository interfaces define data access contracts, with concrete implementations provided in the data layer.

#### Repository Interfaces
- `IFormulaRepository`: Formula data access
- `ISettingsRepository`: Settings data access

#### Design Pattern
```cpp
// Interface defined in domain layer
class IFormulaRepository {
public:
    virtual std::vector<FormulaNode> getFormulaTree() = 0;
    virtual FormulaDetail getFormulaDetail(const std::string& id) = 0;
};
```

### Service Layer

Domain services implement complex business logic, coordinating multiple entities and repositories.

#### Core Services
- `FormulaService`: Formula business logic
  - Formula hierarchy management
  - Formula detail queries
  - Data availability checking

#### Service Design
```cpp
class FormulaService : public IFormulaService {
public:
    explicit FormulaService(std::shared_ptr<IFormulaRepository> repository);
    
    std::vector<FormulaNode> getFormulaTree() override;
    FormulaDetail getFormulaDetail(const std::string& formulaId) override;
    bool isDataAvailable() const override;
};
```

### Use Case Layer

Use cases implement specific application scenarios, serving as the interaction interface between presentation and domain layers.

#### Settings Management Use Cases
- `GetSettingsUseCase`: Get application settings
- `UpdateSettingsUseCase`: Update application settings
- `ToggleThemeUseCase`: Toggle theme
- `GetThemeModeUseCase` / `SetThemeModeUseCase`: Theme mode management
- `GetRecentTabUseCase` / `SetRecentTabUseCase`: Recent tab management

#### Use Case Design Pattern
```cpp
class GetSettingsUseCase {
public:
    explicit GetSettingsUseCase(std::shared_ptr<ISettingsRepository> repository);
    Settings execute();
private:
    std::shared_ptr<ISettingsRepository> m_repository;
};
```

## Data Flow Design

### Query Flow
```
UI Layer → UseCase → Service → Repository → Data Source
Settings ← Settings ← Settings ← Settings ← Config File
```

### Command Flow
```
UI Layer → UseCase → Repository → Data Source
UpdateTheme → UpdateSettingsUseCase → SettingsRepository → Config File
```

## Dependency Injection Integration

### Service Registry
```cpp
class ServiceRegistry {
public:
    static void registerFormulaService(std::shared_ptr<IFormulaService> service);
    static std::shared_ptr<IFormulaService> getFormulaService();
};
```

### Boost.DI Integration (CompositionRoot)
```cpp
class CompositionRoot {
public:
    static auto createInjector();
    static std::shared_ptr<IFormulaService> getFormulaService();
};
```

## Testing Strategy

### Unit Testing
- Entity testing: Verify business rules and data validation
- Service testing: Test business logic using mock repositories
- Use case testing: Verify correctness of application scenarios

### Testing Example
```cpp
TEST(FormulaServiceTest, GetFormulaTree) {
    auto mockRepository = std::make_shared<MockFormulaRepository>();
    FormulaService service(mockRepository);
    
    EXPECT_CALL(*mockRepository, getFormulaTree())
        .WillOnce(Return(expectedNodes));
    
    auto result = service.getFormulaTree();
    ASSERT_EQ(result.size(), expectedSize);
}
```

## Design Trade-offs

### Advantages
- **Business Logic Isolation**: Core business unaffected by technical frameworks
- **High Testability**: Pure C++ implementation, easy to unit test
- **Low Coupling**: Decoupled from other layers through interfaces
- **High Cohesion**: Related business functions organized together

### Considerations
- **Interface Complexity**: Need to define clear repository interfaces
- **Dependency Injection**: Requires additional DI configuration and management
- **Data Conversion**: May need data conversion between layers

## Future Evolution

### Planned Improvements
1. **Complete DI Migration**: Migrate from temporary DependencyProvider to full Boost.DI
2. **Event-Driven**: Introduce domain events to support business process decoupling
3. **Domain Service Extension**: Add more complex business logic services
4. **Specification Pattern**: Use specification pattern for complex query logic

### Extension Guidelines
1. Follow pure C++ principles when adding new entities
2. Define interfaces first when adding new repositories
3. Implement complex business logic through domain services
4. Encapsulate application scenarios through use cases

## Related Documentation

- **[Architecture Overview](../architecture/overview.md)** - Understand overall system architecture
- **[Dependency Injection Design](../architecture/dependency-injection.md)** - DI container and dependency management strategy
- **[Data Management Overview](../data/overview.md)** - Data layer implementation and persistence strategy
- **[Presentation Architecture](../presentation/architecture.md)** - UI layer integration with domain layer