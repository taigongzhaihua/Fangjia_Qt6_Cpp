**English** | [简体中文](../../doc.zh-cn/architecture/system-integration.md)

# System Integration Design

## Overview

This document describes the system integration design of the Fangjia Qt6 C++ project, showing interaction patterns between layers, data flow, and integration strategies. The system adopts a layered architecture, achieving loose coupling component integration through dependency injection and interface abstraction.

## Overall Architecture

### Architecture Layers
```
┌─────────────────────────────────────────┐
│           Application Layer (Apps)      │
│  MainOpenGlWindow, CompositionRoot      │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│        Presentation Layer               │
│  ViewModels, UI Components, Binding    │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│            Domain Layer                 │
│  Entities, UseCases, Services           │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│             Data Layer                  │
│        Repositories, Sources            │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│       Infrastructure Layer             │
│     Graphics, Platform, Resources      │
└─────────────────────────────────────────┘
```

## Core Integration Patterns

### Dependency Injection Integration

#### Composition Root
```cpp
// apps/fangjia/main.cpp
int main(int argc, char* argv[]) {
    // 1. Create infrastructure services
    auto settingsRepository = std::make_shared<SettingsRepository>();
    
    // 2. Create domain use cases
    auto getSettingsUseCase = std::make_shared<GetSettingsUseCase>(settingsRepository);
    auto updateSettingsUseCase = std::make_shared<UpdateSettingsUseCase>(settingsRepository);
    
    // 3. Configure dependency provider
    auto& deps = DependencyProvider::instance();
    deps.setGetSettingsUseCase(getSettingsUseCase);
    deps.setUpdateSettingsUseCase(updateSettingsUseCase);
    
    // 4. Create presentation layer services
    auto themeManager = std::make_shared<ThemeManager>(getThemeModeUseCase, setThemeModeUseCase);
    
    // 5. Create main window
    MainOpenGlWindow window(themeManager);
}
```

#### Dual DI Strategy
The project currently uses a dual dependency injection strategy:

**Boost.DI (CompositionRoot)**
- Used for complete DI configuration of Formula domain
- Automatic dependency resolution
- Type-safe compile-time injection

**Temporary Service Locator (DependencyProvider)**
- Used for Settings and Theme-related use cases
- Runtime dependency resolution
- Compatibility solution during transition

### Data Flow Integration

#### Query Data Flow
```
UI Component → ViewModel → UseCase → Service → Repository → Data Source
           ←           ←        ←       ←          ←
```

**Example: Theme Query**
```cpp
// 1. UI component requests theme information
UiTopBar::updateTheme() {
    // 2. ViewModel calls UseCase
    auto themeMode = deps.getGetThemeModeUseCase()->execute();
    
    // 3. UseCase calls Repository
    // GetThemeModeUseCase::execute() → SettingsRepository::getSettings()
    
    // 4. Repository reads from data source
    // SettingsRepository::getSettings() → configuration file read
    
    // 5. Data returns layer by layer and gets applied
    applyTheme(themeMode);
}
```

#### Command Data Flow
```
UI Event → ViewModel → UseCase → Repository → Data Source
```

**Example: Theme Toggle**
```cpp
// 1. User clicks theme toggle button
onThemeToggle() {
    // 2. ViewModel calls toggle use case
    deps.getToggleThemeUseCase()->execute();
    
    // 3. UseCase updates settings
    // ToggleThemeUseCase::execute() → calculate new theme → UpdateSettingsUseCase
    
    // 4. Repository saves to data source
    // SettingsRepository::updateSettings() → configuration file write
    
    // 5. Notify system to update
    themeManager->notifyThemeChanged();
}
```

### UI Integration Patterns

#### Declarative Component Integration
```cpp
// Presentation layer components integrated through declarative API
auto buildTopBar() {
    return UiTopBar()
        .height(32)
        .backgroundColor(theme.topBarBackground)
        .onThemeToggle([this]() { handleThemeToggle(); })
        .children({
            UiButton().text("Toggle Theme").onClick([this]() { toggleTheme(); }),
            UiSpacer(),
            UiWindowControls()
        });
}
```

#### Data Binding Integration
```cpp
class MainViewModel {
public:
    // Reactive property binding
    void setupBindings() {
        // Theme changes automatically update UI
        observe(themeManager->currentTheme(), [this](const Theme& theme) {
            requestRebuild(); // Trigger UI rebuild
        });
        
        // Settings changes automatically save
        observe(navExpanded, [this](bool expanded) {
            auto useCase = deps.getUpdateSettingsUseCase();
            useCase->updateNavExpanded(expanded);
        });
    }
};
```

## Key Integration Points

### Main Window Integration (MainOpenGlWindow)

The main window serves as the integration center of the system, coordinating various subsystems:

```cpp
class MainOpenGlWindow : public QOpenGLWidget {
public:
    explicit MainOpenGlWindow(std::shared_ptr<ThemeManager> themeManager);

private:
    // Core integration components
    std::unique_ptr<UiRoot> m_uiRoot;           // UI root container
    std::unique_ptr<BindingHost> m_bindingHost; // Data binding host
    std::unique_ptr<RebuildHost> m_rebuildHost; // Rebuild coordinator
    std::shared_ptr<ThemeManager> m_themeManager; // Theme manager
    
    // OpenGL rendering integration
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<IconCache> m_iconCache;
    
    // Page navigation integration
    std::unique_ptr<CurrentPageHost> m_currentPageHost;
};
```

### Rendering System Integration

#### OpenGL Context Integration
```cpp
void MainOpenGlWindow::paintGL() {
    // 1. Prepare rendering context
    auto* gl = QOpenGLContext::currentContext()->functions();
    
    // 2. Update resource context
    m_uiRoot->updateResourceContext(m_iconCache.get(), gl, devicePixelRatio());
    
    // 3. Collect rendering data
    FrameData frameData;
    m_uiRoot->append(frameData);
    
    // 4. Execute rendering
    m_renderer->render(frameData, size(), devicePixelRatio());
}
```

#### Resource Management Integration
```cpp
// Icon cache integration with theme system
class IconCache {
public:
    void updateTheme(const Theme& theme) {
        // Clean up old theme-related resources
        clearThemeResources();
        
        // Load resources based on new theme
        loadThemeResources(theme);
    }
};
```

### Event System Integration

#### User Input Event Flow
```
Qt Event → MainOpenGlWindow → UiRoot → Target Component → ViewModel → UseCase
```

```cpp
void MainOpenGlWindow::mousePressEvent(QMouseEvent* event) {
    QPoint pos = event->pos();
    
    // Convert coordinates and dispatch to UI system
    if (m_uiRoot->onMousePress(pos.x(), pos.y())) {
        event->accept();
    }
}
```

#### Application Lifecycle Events
```cpp
// Integration sequence during application startup
void MainOpenGlWindow::initializeGL() {
    // 1. Initialize OpenGL renderer
    m_renderer = std::make_unique<Renderer>();
    
    // 2. Initialize icon cache
    m_iconCache = std::make_unique<IconCache>();
    
    // 3. Set up theme listening
    m_themeManager->onThemeChanged([this](const Theme& theme) {
        m_iconCache->updateTheme(theme);
        scheduleUpdate();
    });
    
    // 4. Build initial UI hierarchy
    buildUIHierarchy();
}
```

## Testing Integration Strategy

### Layered Testing
```cpp
// Domain layer unit testing
TEST(GetSettingsUseCaseTest, ReturnsCorrectSettings) {
    auto mockRepo = std::make_shared<MockSettingsRepository>();
    GetSettingsUseCase useCase(mockRepo);
    
    EXPECT_CALL(*mockRepo, getSettings())
        .WillOnce(Return(expectedSettings));
    
    auto result = useCase.execute();
    ASSERT_EQ(result.themeMode, "dark");
}

// Integration testing
TEST(ThemeIntegrationTest, ThemeChangeUpdatesUI) {
    TestApplication app;
    
    // Toggle theme
    app.toggleTheme();
    
    // Verify UI update
    ASSERT_TRUE(app.isUsingDarkTheme());
}
```

### Mock Integration
```cpp
class MockDependencyProvider : public DependencyProvider {
public:
    void setupMocks() {
        setGetSettingsUseCase(std::make_shared<MockGetSettingsUseCase>());
        setUpdateSettingsUseCase(std::make_shared<MockUpdateSettingsUseCase>());
    }
};
```

## Performance Integration Considerations

### Rendering Performance
- **Batch Updates**: Collect all UI changes and render them uniformly
- **View Culling**: Only render components within visible area
- **Caching Strategy**: Smart cache management for icons and textures

### Memory Management
- **Shared Resources**: Manage lifecycle through std::shared_ptr
- **RAII Pattern**: Automatic resource cleanup
- **Object Pool**: Use object pool for frequently created temporary objects

### Data Binding Performance
- **Lazy Rebuild**: Only rebuild UI when truly needed
- **Dependency Tracking**: Precisely track data dependency relationships
- **Batch Notifications**: Merge notifications for multiple property changes

## Future Integration Evolution

### Planned Improvements
1. **Unified DI System**: Migrate all dependencies to Boost.DI
2. **Event Bus**: Introduce event bus for loose coupling communication
3. **Asynchronous Data Flow**: Support asynchronous data loading and processing
4. **Plugin Architecture**: Support dynamic loading of functional modules

### Extension Guidelines
1. Follow dependency direction principles when adding new layers
2. Define inter-layer contracts through interfaces
3. Use dependency injection to manage component lifecycle
4. Maintain unidirectional data flow

## Related Documentation

- **[Architecture Overview](./overview.md)** - Overall system architecture and design principles
- **[Dependency Injection Design](./dependency-injection.md)** - Detailed design of DI container and dependency management
- **[Domain Layer Design](../domain/design.md)** - Internal design and patterns of domain layer
- **[Presentation Architecture](../presentation/architecture.md)** - Declarative system design of UI layer
- **[Graphics & Rendering System](../infrastructure/gfx.md)** - Low-level rendering and graphics integration