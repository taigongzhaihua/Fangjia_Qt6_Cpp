[English](../../doc/architecture/system-integration.md) | **简体中文**

# 系统集成设计

## 概述

本文档描述 Fangjia Qt6 C++ 项目的系统集成设计，展示各层之间的交互模式、数据流转和集成策略。系统采用分层架构，通过依赖注入和接口抽象实现松耦合的组件集成。

## 整体架构

### 架构层次
```
┌─────────────────────────────────────────┐
│            应用层 (Apps)                 │
│  MainOpenGlWindow, CompositionRoot      │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│          表现层 (Presentation)           │
│  ViewModels, UI Components, Binding    │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│           领域层 (Domain)                │
│  Entities, UseCases, Services           │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│            数据层 (Data)                 │
│        Repositories, Sources            │
└─────────────────────────────────────────┘
                    ↓
┌─────────────────────────────────────────┐
│        基础设施层 (Infrastructure)       │
│     Graphics, Platform, Resources      │
└─────────────────────────────────────────┘
```

## 核心集成模式

### 依赖注入集成

#### 组合根 (Composition Root)
```cpp
// apps/fangjia/main.cpp
int main(int argc, char* argv[]) {
    // 1. 创建基础设施服务
    auto settingsRepository = std::make_shared<SettingsRepository>();
    
    // 2. 创建领域用例
    auto getSettingsUseCase = std::make_shared<GetSettingsUseCase>(settingsRepository);
    auto updateSettingsUseCase = std::make_shared<UpdateSettingsUseCase>(settingsRepository);
    
    // 3. 配置依赖提供者
    auto& deps = DependencyProvider::instance();
    deps.setGetSettingsUseCase(getSettingsUseCase);
    deps.setUpdateSettingsUseCase(updateSettingsUseCase);
    
    // 4. 创建表现层服务
    auto themeManager = std::make_shared<ThemeManager>(getThemeModeUseCase, setThemeModeUseCase);
    
    // 5. 创建主窗口
    MainOpenGlWindow window(themeManager);
}
```

#### 双重 DI 策略
当前项目使用双重依赖注入策略：

**Boost.DI (CompositionRoot)**
- 用于配方领域的完整 DI 配置
- 自动解析依赖关系
- 类型安全的编译时注入

**临时服务定位器 (DependencyProvider)**
- 用于设置和主题相关用例
- 运行时依赖解析
- 过渡期间的兼容方案

### 数据流集成

#### 查询数据流
```
UI 组件 → ViewModel → UseCase → Service → Repository → 数据源
     ←          ←        ←       ←          ←
```

**具体示例：主题查询**
```cpp
// 1. UI 组件请求主题信息
UiTopBar::updateTheme() {
    // 2. ViewModel 调用 UseCase
    auto themeMode = deps.getGetThemeModeUseCase()->execute();
    
    // 3. UseCase 调用 Repository
    // GetThemeModeUseCase::execute() → SettingsRepository::getSettings()
    
    // 4. Repository 从数据源读取
    // SettingsRepository::getSettings() → 配置文件读取
    
    // 5. 数据逐层返回并应用
    applyTheme(themeMode);
}
```

#### 命令数据流
```
UI 事件 → ViewModel → UseCase → Repository → 数据源
```

**具体示例：主题切换**
```cpp
// 1. 用户点击主题切换按钮
onThemeToggle() {
    // 2. ViewModel 调用切换用例
    deps.getToggleThemeUseCase()->execute();
    
    // 3. UseCase 更新设置
    // ToggleThemeUseCase::execute() → 计算新主题 → UpdateSettingsUseCase
    
    // 4. Repository 保存到数据源
    // SettingsRepository::updateSettings() → 配置文件写入
    
    // 5. 通知系统更新
    themeManager->notifyThemeChanged();
}
```

### UI 集成模式

#### 声明式组件集成
```cpp
// 表现层组件通过声明式 API 集成
auto buildTopBar() {
    return UiTopBar()
        .height(32)
        .backgroundColor(theme.topBarBackground)
        .onThemeToggle([this]() { handleThemeToggle(); })
        .children({
            UiButton().text("切换主题").onClick([this]() { toggleTheme(); }),
            UiSpacer(),
            UiWindowControls()
        });
}
```

#### 数据绑定集成
```cpp
class MainViewModel {
public:
    // 响应式属性绑定
    void setupBindings() {
        // 主题变化自动更新 UI
        observe(themeManager->currentTheme(), [this](const Theme& theme) {
            requestRebuild(); // 触发 UI 重建
        });
        
        // 设置变化自动保存
        observe(navExpanded, [this](bool expanded) {
            auto useCase = deps.getUpdateSettingsUseCase();
            useCase->updateNavExpanded(expanded);
        });
    }
};
```

## 关键集成点

### 主窗口集成 (MainOpenGlWindow)

主窗口作为系统的集成中心，协调各个子系统：

```cpp
class MainOpenGlWindow : public QOpenGLWidget {
public:
    explicit MainOpenGlWindow(std::shared_ptr<ThemeManager> themeManager);

private:
    // 核心集成组件
    std::unique_ptr<UiRoot> m_uiRoot;           // UI 根容器
    std::unique_ptr<BindingHost> m_bindingHost; // 数据绑定主机
    std::unique_ptr<RebuildHost> m_rebuildHost; // 重建协调器
    std::shared_ptr<ThemeManager> m_themeManager; // 主题管理器
    
    // OpenGL 渲染集成
    std::unique_ptr<Renderer> m_renderer;
    std::unique_ptr<IconCache> m_iconCache;
    
    // 页面导航集成
    std::unique_ptr<CurrentPageHost> m_currentPageHost;
};
```

### 渲染系统集成

#### OpenGL 上下文集成
```cpp
void MainOpenGlWindow::paintGL() {
    // 1. 准备渲染上下文
    auto* gl = QOpenGLContext::currentContext()->functions();
    
    // 2. 更新资源上下文
    m_uiRoot->updateResourceContext(m_iconCache.get(), gl, devicePixelRatio());
    
    // 3. 收集渲染数据
    FrameData frameData;
    m_uiRoot->append(frameData);
    
    // 4. 执行渲染
    m_renderer->render(frameData, size(), devicePixelRatio());
}
```

#### 资源管理集成
```cpp
// 图标缓存与主题系统集成
class IconCache {
public:
    void updateTheme(const Theme& theme) {
        // 清理旧的主题相关资源
        clearThemeResources();
        
        // 根据新主题加载资源
        loadThemeResources(theme);
    }
};
```

### 事件系统集成

#### 用户输入事件流
```
Qt 事件 → MainOpenGlWindow → UiRoot → 目标组件 → ViewModel → UseCase
```

```cpp
void MainOpenGlWindow::mousePressEvent(QMouseEvent* event) {
    QPoint pos = event->pos();
    
    // 转换坐标并分发到 UI 系统
    if (m_uiRoot->onMousePress(pos.x(), pos.y())) {
        event->accept();
    }
}
```

#### 应用生命周期事件
```cpp
// 应用启动时的集成序列
void MainOpenGlWindow::initializeGL() {
    // 1. 初始化 OpenGL 渲染器
    m_renderer = std::make_unique<Renderer>();
    
    // 2. 初始化图标缓存
    m_iconCache = std::make_unique<IconCache>();
    
    // 3. 设置主题监听
    m_themeManager->onThemeChanged([this](const Theme& theme) {
        m_iconCache->updateTheme(theme);
        scheduleUpdate();
    });
    
    // 4. 构建初始 UI 层次结构
    buildUIHierarchy();
}
```

## 测试集成策略

### 分层测试
```cpp
// 领域层单元测试
TEST(GetSettingsUseCaseTest, ReturnsCorrectSettings) {
    auto mockRepo = std::make_shared<MockSettingsRepository>();
    GetSettingsUseCase useCase(mockRepo);
    
    EXPECT_CALL(*mockRepo, getSettings())
        .WillOnce(Return(expectedSettings));
    
    auto result = useCase.execute();
    ASSERT_EQ(result.themeMode, "dark");
}

// 集成测试
TEST(ThemeIntegrationTest, ThemeChangeUpdatesUI) {
    TestApplication app;
    
    // 切换主题
    app.toggleTheme();
    
    // 验证 UI 更新
    ASSERT_TRUE(app.isUsingDarkTheme());
}
```

### Mock 集成
```cpp
class MockDependencyProvider : public DependencyProvider {
public:
    void setupMocks() {
        setGetSettingsUseCase(std::make_shared<MockGetSettingsUseCase>());
        setUpdateSettingsUseCase(std::make_shared<MockUpdateSettingsUseCase>());
    }
};
```

## 性能集成考虑

### 渲染性能
- **批量更新**: 收集所有 UI 变更后统一渲染
- **视图裁剪**: 只渲染可见区域内的组件
- **缓存策略**: 图标和纹理的智能缓存管理

### 内存管理
- **共享资源**: 通过 std::shared_ptr 管理生命周期
- **RAII 模式**: 自动资源清理
- **对象池**: 频繁创建的临时对象使用对象池

### 数据绑定性能
- **惰性重建**: 只有真正需要时才重建 UI
- **依赖追踪**: 精确追踪数据依赖关系
- **批量通知**: 合并多个属性变更的通知

## 未来集成演进

### 计划改进
1. **统一 DI 系统**: 将所有依赖迁移到 Boost.DI
2. **事件总线**: 引入事件总线实现松耦合通信
3. **异步数据流**: 支持异步数据加载和处理
4. **插件架构**: 支持动态加载功能模块

### 扩展指南
1. 新增层次时遵循依赖方向原则
2. 通过接口定义层间契约
3. 使用依赖注入管理组件生命周期
4. 保持数据流的单向性

## 相关文档

- **[架构概览](./overview.md)** - 整体系统架构和设计原则
- **[依赖注入设计](./dependency-injection.md)** - DI 容器和依赖管理详细设计
- **[领域层设计](../domain/design.md)** - 领域层的内部设计和模式
- **[表现层架构](../presentation/architecture.md)** - UI 层的声明式系统设计
- **[图形与渲染系统](../infrastructure/gfx.md)** - 底层渲染和图形集成