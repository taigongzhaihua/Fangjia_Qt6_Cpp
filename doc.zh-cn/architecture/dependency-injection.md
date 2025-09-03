[English](../../doc/architecture/dependency-injection.md) | **简体中文**

# 依赖注入设计

## 概述

本文档描述 Fangjia Qt6 C++ 项目的依赖注入（Dependency Injection, DI）设计策略。项目当前采用混合 DI 方案，结合 Boost.DI 容器和临时服务定位器，为不同的领域提供灵活的依赖管理。

## 设计目标

### 核心原则
- **依赖倒置**: 高层模块不依赖低层模块，都依赖抽象
- **控制反转**: 对象创建和依赖注入由容器管理
- **单一职责**: 每个组件专注于业务逻辑，而非依赖管理
- **可测试性**: 通过接口注入支持单元测试和 Mock

### 技术目标
- **编译时安全**: 尽可能在编译时检测依赖错误
- **运行时性能**: 最小化依赖解析的运行时开销
- **配置简洁**: 提供清晰的依赖配置 API
- **渐进式迁移**: 支持从旧代码逐步迁移到 DI

## 当前 DI 架构

### 双重 DI 策略

项目当前使用两套并行的 DI 系统：

#### 1. Boost.DI 容器 (CompositionRoot)
- **适用范围**: Formula 领域相关服务
- **特点**: 编译时类型安全，自动依赖解析
- **实现**: 基于 boost-ext/di 库

#### 2. 临时服务定位器 (DependencyProvider)
- **适用范围**: Settings、Theme 相关用例
- **特点**: 运行时依赖解析，简单易用
- **实现**: 自定义单例服务定位器

### 架构图
```
┌─────────────────────────────────────────────────────────┐
│                   应用层 (main.cpp)                     │
│  ┌─────────────────┐      ┌─────────────────────────┐   │
│  │  CompositionRoot │      │  DependencyProvider     │   │
│  │  (Boost.DI)     │      │  (Service Locator)      │   │
│  └─────────────────┘      └─────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
                    ↓                      ↓
┌─────────────────────────────────────────────────────────┐
│                      服务层                              │
│  ┌─────────────────┐      ┌─────────────────────────┐   │
│  │  FormulaService │      │  Settings UseCases      │   │
│  │  (DI Managed)   │      │  (Locator Managed)      │   │
│  └─────────────────┘      └─────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
                    ↓                      ↓
┌─────────────────────────────────────────────────────────┐
│                    数据层                                │
│  ┌─────────────────┐      ┌─────────────────────────┐   │
│  │ FormulaRepository│      │ SettingsRepository      │   │
│  │  (Interface)    │      │ (Concrete)              │   │
│  └─────────────────┘      └─────────────────────────┘   │
└─────────────────────────────────────────────────────────┘
```

## Boost.DI 实现

### CompositionRoot 设计
```cpp
class CompositionRoot {
public:
    /// 创建配置完整的 DI 注入器
    static auto createInjector() {
        return boost::di::make_injector(
            // 绑定接口到实现
            boost::di::bind<domain::repositories::IFormulaRepository>()
                .to<data::repositories::FormulaRepository>(),
            
            boost::di::bind<domain::services::IFormulaService>()
                .to<domain::services::FormulaService>()
        );
    }
    
    /// 便利方法：获取 FormulaService 实例
    static std::shared_ptr<domain::services::IFormulaService> getFormulaService() {
        auto injector = createInjector();
        return injector.create<std::shared_ptr<domain::services::IFormulaService>>();
    }
};
```

### 依赖配置示例
```cpp
// 在 main.cpp 中配置 Formula 领域依赖
auto formulaService = CompositionRoot::getFormulaService();
deps.setFormulaService(formulaService);
```

### Boost.DI 优势
- **编译时验证**: 依赖循环和缺失在编译时检测
- **自动解析**: 构造函数参数自动注入
- **类型安全**: 强类型检查，减少运行时错误
- **无反射开销**: 编译时生成代码，运行时性能优异

## 服务定位器实现

### DependencyProvider 设计
```cpp
class DependencyProvider {
public:
    static DependencyProvider& instance();

    // Setters - 由组合根调用
    void setGetSettingsUseCase(std::shared_ptr<GetSettingsUseCase> useCase);
    void setUpdateSettingsUseCase(std::shared_ptr<UpdateSettingsUseCase> useCase);
    void setToggleThemeUseCase(std::shared_ptr<ToggleThemeUseCase> useCase);
    
    // Getters - 由 ViewModels 调用
    std::shared_ptr<GetSettingsUseCase> getGetSettingsUseCase() const;
    std::shared_ptr<UpdateSettingsUseCase> getUpdateSettingsUseCase() const;
    std::shared_ptr<ToggleThemeUseCase> getToggleThemeUseCase() const;

private:
    // 存储服务实例
    std::shared_ptr<GetSettingsUseCase> m_getSettingsUseCase;
    std::shared_ptr<UpdateSettingsUseCase> m_updateSettingsUseCase;
    std::shared_ptr<ToggleThemeUseCase> m_toggleThemeUseCase;
    // ... 其他服务
};
```

### 依赖配置示例
```cpp
// 在 main.cpp 中配置 Settings 相关依赖
auto settingsRepository = std::make_shared<SettingsRepository>();
auto getSettingsUseCase = std::make_shared<GetSettingsUseCase>(settingsRepository);
auto updateSettingsUseCase = std::make_shared<UpdateSettingsUseCase>(settingsRepository);

auto& deps = DependencyProvider::instance();
deps.setGetSettingsUseCase(getSettingsUseCase);
deps.setUpdateSettingsUseCase(updateSettingsUseCase);
```

### 服务定位器优势
- **简单易用**: 直观的 getter/setter API
- **渐进迁移**: 可以逐步替换硬编码依赖
- **运行时灵活**: 支持动态替换依赖
- **最小依赖**: 不需要额外的第三方库

## 依赖注入模式

### 构造函数注入
```cpp
// 推荐方式：通过构造函数注入依赖
class FormulaService : public IFormulaService {
public:
    explicit FormulaService(std::shared_ptr<IFormulaRepository> repository)
        : m_repository(repository) {}

private:
    std::shared_ptr<IFormulaRepository> m_repository;
};
```

### 服务定位注入
```cpp
// 临时方式：通过服务定位器获取依赖
class SettingsViewModel {
public:
    void saveSettings(const Settings& settings) {
        auto useCase = DependencyProvider::instance().getUpdateSettingsUseCase();
        useCase->execute(settings);
    }
};
```

### 接口设计模式
```cpp
// 在领域层定义接口
namespace domain::repositories {
    class ISettingsRepository {
    public:
        virtual ~ISettingsRepository() = default;
        virtual Settings getSettings() = 0;
        virtual void updateSettings(const Settings& settings) = 0;
    };
}

// 在数据层实现接口
namespace data::repositories {
    class SettingsRepository : public domain::repositories::ISettingsRepository {
    public:
        Settings getSettings() override;
        void updateSettings(const Settings& settings) override;
    };
}
```

## 生命周期管理

### 单例生命周期
```cpp
// 应用级单例：主题管理器
auto themeManager = std::make_shared<ThemeManager>(getThemeModeUseCase, setThemeModeUseCase);
// 在整个应用生命周期内保持单一实例
```

### 瞬态生命周期
```cpp
// 每次创建新实例：用例对象
auto getSettingsUseCase = std::make_shared<GetSettingsUseCase>(settingsRepository);
// 可以根据需要创建多个实例
```

### 作用域生命周期
```cpp
// 窗口作用域：UI 相关服务
class MainOpenGlWindow {
public:
    MainOpenGlWindow() {
        // 这些服务与窗口生命周期绑定
        m_bindingHost = std::make_unique<BindingHost>();
        m_rebuildHost = std::make_unique<RebuildHost>();
    }
};
```

## 测试支持

### Mock 注入
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

### 测试配置
```cpp
class TestDependencyProvider {
public:
    static void setupTestDependencies() {
        auto& deps = DependencyProvider::instance();
        
        // 注入测试用的 Mock 对象
        deps.setGetSettingsUseCase(std::make_shared<MockGetSettingsUseCase>());
        deps.setUpdateSettingsUseCase(std::make_shared<MockUpdateSettingsUseCase>());
    }
};
```

## 迁移策略

### 当前状态
```
✅ Formula 领域 → Boost.DI (完整实现)
🔄 Settings 领域 → DependencyProvider (临时方案)
🚧 Theme 领域 → 混合方案
⏳ 新领域 → 计划使用 Boost.DI
```

### 迁移路径
```cpp
// 阶段1：保持当前双重系统
// 阶段2：新功能优先使用 Boost.DI
// 阶段3：逐步迁移 Settings 到 Boost.DI
// 阶段4：移除 DependencyProvider

// 目标架构（阶段4后）
auto createUnifiedInjector() {
    return boost::di::make_injector(
        // Formula 领域
        boost::di::bind<IFormulaRepository>().to<FormulaRepository>(),
        boost::di::bind<IFormulaService>().to<FormulaService>(),
        
        // Settings 领域
        boost::di::bind<ISettingsRepository>().to<SettingsRepository>(),
        boost::di::bind<GetSettingsUseCase>().in(boost::di::singleton),
        boost::di::bind<UpdateSettingsUseCase>().in(boost::di::singleton),
        
        // Theme 领域
        boost::di::bind<ThemeManager>().in(boost::di::singleton)
    );
}
```

### 迁移最佳实践
1. **渐进式迁移**: 一次迁移一个领域
2. **接口优先**: 先定义接口，再迁移实现
3. **测试覆盖**: 确保迁移前后行为一致
4. **向后兼容**: 保持旧代码可用直到完全迁移

## 性能考虑

### Boost.DI 性能
- **编译时开销**: 增加编译时间，但运行时性能优异
- **代码生成**: 编译时生成最优的对象创建代码
- **内存效率**: 避免运行时查找和反射开销

### 服务定位器性能
- **查找开销**: std::map 或类似容器的查找成本
- **内存占用**: 需要存储所有服务实例
- **线程安全**: 单例访问可能需要同步开销

### 优化策略
```cpp
// 缓存常用服务以减少查找开销
class OptimizedDependencyProvider {
private:
    // 使用成员变量缓存常用服务
    mutable std::shared_ptr<GetSettingsUseCase> m_cachedGetSettings;
    
public:
    std::shared_ptr<GetSettingsUseCase> getGetSettingsUseCase() const {
        if (!m_cachedGetSettings) {
            m_cachedGetSettings = /* 创建或查找 */;
        }
        return m_cachedGetSettings;
    }
};
```

## 最佳实践

### 依赖设计原则
1. **依赖接口不依赖实现**: 所有依赖都应该是抽象接口
2. **最小依赖原则**: 只注入真正需要的依赖
3. **单一职责**: 每个服务只负责一个明确的职责
4. **生命周期明确**: 清晰定义服务的生命周期需求

### 代码组织
```cpp
// 推荐的文件组织结构
domain/
  ├── entities/           # 领域实体
  ├── repositories/       # 仓储接口（在领域层定义）
  ├── services/          # 领域服务接口和实现
  └── usecases/          # 用例实现

data/
  ├── repositories/      # 仓储实现（实现领域层接口）
  └── sources/           # 数据源

apps/
  ├── CompositionRoot.*  # DI 配置
  └── DependencyProvider.* # 临时服务定位器
```

### 错误处理
```cpp
// DI 错误处理示例
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

## 相关文档

- **[系统集成设计](./system-integration.md)** - 各层间的集成模式和数据流
- **[领域层设计](../domain/design.md)** - 领域层的依赖和接口设计
- **[架构概览](./overview.md)** - 整体架构设计和分层原则
- **[数据管理概览](../data/overview.md)** - 数据层的依赖实现