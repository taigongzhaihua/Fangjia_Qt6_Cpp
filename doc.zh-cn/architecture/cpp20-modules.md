# C++20 Module Interface Files (.ixx)

This document describes the C++20 module interface files (`.ixx`) created to complement the existing split C++ source files in the Fangjia Qt6 project.

## Module Organization

The modules follow the project's namespace structure and mirror the organization of the existing `.cpp` and `.h` files:

### Domain Entities
- `fangjia.domain.entities.settings` - Settings entity (`domain/entities/Settings.ixx`)
- `fangjia.domain.entities.formula` - Formula entities (`domain/entities/Formula.ixx`)
- `fangjia.domain.entities.theme` - Theme enumeration (`domain/entities/Theme.ixx`)

### Domain Repositories  
- `fangjia.domain.repositories.settings` - ISettingsRepository interface (`domain/repositories/ISettingsRepository.ixx`)
- `fangjia.domain.repositories.formula` - IFormulaRepository interface (`domain/repositories/IFormulaRepository.ixx`)

### Domain Services
- `fangjia.domain.services.formula` - FormulaService and IFormulaService (`domain/services/FormulaService.ixx`)

### Domain Use Cases
- `fangjia.domain.usecases.get_settings` - GetSettingsUseCase (`domain/usecases/GetSettingsUseCase.ixx`)
- `fangjia.domain.usecases.update_settings` - UpdateSettingsUseCase (`domain/usecases/UpdateSettingsUseCase.ixx`)
- `fangjia.domain.usecases.get_theme_mode` - GetThemeModeUseCase (`domain/usecases/GetThemeModeUseCase.ixx`)
- `fangjia.domain.usecases.set_theme_mode` - SetThemeModeUseCase (`domain/usecases/SetThemeModeUseCase.ixx`)
- `fangjia.domain.usecases.toggle_theme` - ToggleThemeUseCase (`domain/usecases/ToggleThemeUseCase.ixx`)
- `fangjia.domain.usecases.get_recent_tab` - GetRecentTabUseCase (`domain/usecases/GetRecentTabUseCase.ixx`)
- `fangjia.domain.usecases.set_recent_tab` - SetRecentTabUseCase (`domain/usecases/SetRecentTabUseCase.ixx`)

### Application Layer
- `fangjia.apps.composition_root` - CompositionRoot (`apps/fangjia/CompositionRoot.ixx`)
- `fangjia.apps.unified_dependency_provider` - UnifiedDependencyProvider (`apps/fangjia/UnifiedDependencyProvider.ixx`)
- `fangjia.apps.dependency_migration_tool` - DependencyMigrationTool (`apps/fangjia/DependencyMigrationTool.ixx`)

## Module Design Principles

1. **Namespace Alignment**: Module names closely follow the C++ namespace structure
2. **Pure C++ Focus**: Modules primarily cover pure C++ components without Qt dependencies
3. **Interface Export**: All public classes, functions, and types are exported using `export` keyword
4. **Dependency Management**: Modules import other modules as dependencies where needed
5. **Forward Declarations**: Use forward declarations for types that don't need complete definitions

## Build Integration

The CMakeLists.txt has been updated to:
- Enable C++20 modules support for compatible compilers
- Include `.ixx` files in the build process alongside `.cpp` and `.h` files
- Maintain compatibility with the existing build system

## Benefits

- **Faster Compilation**: Modules provide faster compilation compared to traditional headers
- **Better Encapsulation**: Explicit export control improves API design
- **Dependency Tracking**: Better dependency management and reduced include pollution
- **Future-Proof**: Prepares the codebase for modern C++ development practices

## Compatibility

These module interfaces work alongside the existing header files and don't break existing functionality. The traditional header-based includes continue to work while the project gradually adopts modules.