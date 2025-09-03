[English](../../doc/domain/design.md) | **简体中文**

# 领域层设计

## 概述

领域层（Domain Layer）是 Fangjia Qt6 C++ 项目的核心，实现了清洁架构（Clean Architecture）的领域业务逻辑。该层完全独立于外部框架和基础设施，使用纯 C++ 实现，确保业务逻辑的可测试性和可维护性。

## 设计原则

### 依赖倒置
- 领域层不依赖任何外部框架（包括 Qt）
- 通过接口定义与外层的契约
- 外层通过依赖注入提供具体实现

### 单一职责
- 每个组件只负责单一的业务功能
- 实体负责业务数据，服务负责业务逻辑，用例负责应用场景

### 开闭原则
- 通过接口抽象支持功能扩展
- 新增业务功能不需要修改现有代码

## 架构组件

### 实体层 (Entities)

实体代表核心业务对象，包含业务数据和基本验证逻辑。

#### 核心实体

**Formula 相关实体**
- `FormulaDetail`: 中药配方详细信息
- `FormulaNode`: 配方树层次结构节点
- `Drug`: 药物基本信息
- `Formulation`: 配方组合信息
- `Category`: 分类信息

**系统配置实体**
- `Settings`: 应用程序设置
- `Theme`: 主题配置

#### 设计特点
```cpp
// 纯 C++ 设计，无 Qt 依赖
struct FormulaDetail {
    std::string id;
    std::string name;
    std::string composition;
    // ... 其他字段
};
```

### 仓储层 (Repositories)

仓储接口定义数据访问契约，具体实现在数据层提供。

#### 仓储接口
- `IFormulaRepository`: 配方数据访问
- `ISettingsRepository`: 设置数据访问

#### 设计模式
```cpp
// 接口定义在领域层
class IFormulaRepository {
public:
    virtual std::vector<FormulaNode> getFormulaTree() = 0;
    virtual FormulaDetail getFormulaDetail(const std::string& id) = 0;
};
```

### 服务层 (Services)

领域服务实现复杂的业务逻辑，协调多个实体和仓储。

#### 核心服务
- `FormulaService`: 配方业务逻辑
  - 配方层次结构管理
  - 配方详情查询
  - 数据可用性检查

#### 服务设计
```cpp
class FormulaService : public IFormulaService {
public:
    explicit FormulaService(std::shared_ptr<IFormulaRepository> repository);
    
    std::vector<FormulaNode> getFormulaTree() override;
    FormulaDetail getFormulaDetail(const std::string& formulaId) override;
    bool isDataAvailable() const override;
};
```

### 用例层 (UseCases)

用例实现具体的应用场景，是表现层与领域层的交互接口。

#### 设置管理用例
- `GetSettingsUseCase`: 获取应用设置
- `UpdateSettingsUseCase`: 更新应用设置
- `ToggleThemeUseCase`: 切换主题
- `GetThemeModeUseCase` / `SetThemeModeUseCase`: 主题模式管理
- `GetRecentTabUseCase` / `SetRecentTabUseCase`: 最近使用标签管理

#### 用例设计模式
```cpp
class GetSettingsUseCase {
public:
    explicit GetSettingsUseCase(std::shared_ptr<ISettingsRepository> repository);
    Settings execute();
private:
    std::shared_ptr<ISettingsRepository> m_repository;
};
```

## 数据流设计

### 查询流程
```
UI层 → UseCase → Service → Repository → 数据源
Settings ← Settings ← Settings ← Settings ← 配置文件
```

### 命令流程
```
UI层 → UseCase → Repository → 数据源
UpdateTheme → UpdateSettingsUseCase → SettingsRepository → 配置文件
```

## 依赖注入集成

### 服务注册 (ServiceRegistry)
```cpp
class ServiceRegistry {
public:
    static void registerFormulaService(std::shared_ptr<IFormulaService> service);
    static std::shared_ptr<IFormulaService> getFormulaService();
};
```

### Boost.DI 集成 (CompositionRoot)
```cpp
class CompositionRoot {
public:
    static auto createInjector();
    static std::shared_ptr<IFormulaService> getFormulaService();
};
```

## 测试策略

### 单元测试
- 实体测试：验证业务规则和数据验证
- 服务测试：使用 mock 仓储测试业务逻辑
- 用例测试：验证应用场景的正确性

### 测试示例
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

## 设计权衡

### 优势
- **业务逻辑隔离**: 核心业务不受技术框架影响
- **高可测试性**: 纯 C++ 实现，易于单元测试
- **低耦合**: 通过接口与其他层解耦
- **高内聚**: 相关业务功能组织在一起

### 考虑因素
- **接口复杂度**: 需要定义清晰的仓储接口
- **依赖注入**: 需要额外的 DI 配置和管理
- **数据转换**: 可能需要在层间进行数据转换

## 未来演进

### 计划改进
1. **完整 DI 迁移**: 从临时 DependencyProvider 迁移到完整的 Boost.DI
2. **事件驱动**: 引入领域事件支持业务流程解耦
3. **领域服务扩展**: 添加更多复杂业务逻辑服务
4. **规约模式**: 使用规约模式实现复杂查询逻辑

### 扩展指南
1. 新增实体时遵循纯 C++ 原则
2. 新增仓储时先定义接口
3. 复杂业务逻辑通过领域服务实现
4. 应用场景通过用例封装

## 相关文档

- **[架构概览](../architecture/overview.md)** - 了解整体系统架构
- **[依赖注入设计](../architecture/dependency-injection.md)** - DI 容器和依赖管理策略
- **[数据管理概览](../data/overview.md)** - 数据层实现和持久化策略
- **[表现层架构](../presentation/architecture.md)** - UI 层与领域层的集成