# UI框架迁移指南

## 概述

本指南帮助开发者从当前的UI框架平滑迁移到新的WPF风格体系架构。迁移过程采用渐进式策略，确保系统在迁移过程中始终保持可用状态。

## 迁移策略

### 渐进式迁移原则

1. **保持向后兼容**：使用适配器模式确保现有代码继续工作
2. **分阶段实施**：按模块逐步迁移，降低风险
3. **并行开发**：新旧系统并行运行，逐步切换
4. **充分测试**：每个阶段都有完整的测试验证

## 核心概念映射

### 1. 基础接口迁移

#### 当前架构 → 新架构映射

```cpp
// 旧系统
IUiComponent → IUIElement
├── updateLayout() → measureSize() + arrange()
├── updateResourceContext() → (集成到RenderContext)
├── append() → render()
├── onMousePress/Move/Release() → onInput()
├── tick() → (集成到Animation系统)
└── bounds() → bounds()

// 迁移适配器示例
class IUiComponentAdapter : public UIElement {
private:
    std::unique_ptr<IUiComponent> m_legacy;
    
public:
    QSize measureSize(const SizeConstraints& constraints) override {
        // 将新的约束转换为旧的窗口大小
        m_legacy->updateLayout(constraints.toSize());
        return m_legacy->bounds().size();
    }
    
    void render(IRenderContext& context) override {
        // 创建适配的渲染数据
        Render::FrameData frameData;
        m_legacy->append(frameData);
        // 将FrameData转换为新的渲染调用
        convertFrameDataToRenderCalls(frameData, context);
    }
};
```

### 2. 控件层次结构迁移

#### 容器类迁移映射

```cpp
// 当前容器 → 新容器
UiRoot → UIElement (作为应用根)
UiPage → ContentControl (页面容器)
UiPanel → Panel (布局面板)
UiContainer → Canvas (自由布局)
UiGrid → Grid (网格布局)
UiScrollView → ScrollViewer (滚动容器)

// 示例：UiPanel迁移
class UiPanelAdapter : public Panel {
private:
    std::unique_ptr<UiPanel> m_legacy;
    
public:
    UiPanelAdapter(std::unique_ptr<UiPanel> legacy) 
        : m_legacy(std::move(legacy)) {}
    
    void addChild(std::unique_ptr<IUIElement> child) override {
        // 如果child是适配器，提取legacy组件
        if (auto adapter = dynamic_cast<IUiComponentAdapter*>(child.get())) {
            m_legacy->addComponent(adapter->getLegacyComponent());
        }
        Panel::addChild(std::move(child));
    }
};
```

#### 控件类迁移映射

```cpp
// 当前控件 → 新控件
UiPushButton → Button
UiTopBar → (分解为多个控件)
UiNav → NavigationView
UiListBox → ListBox
UiTabView → TabView
UiTreeList → TreeView

// 示例：UiPushButton迁移
class UiPushButtonAdapter : public Button {
public:
    UiPushButtonAdapter(const QString& text) : Button(text) {
        // 迁移现有样式和行为
        migrateStyles();
        migrateEventHandlers();
    }
    
private:
    void migrateStyles() {
        // 将旧样式转换为新样式系统
        // 这里可以保持视觉一致性
    }
};
```

## 分阶段迁移计划

### 阶段1：基础设施准备（第1-2周）

#### 目标
- 建立新架构基础设施
- 创建适配器系统
- 保证现有功能不受影响

#### 具体任务

```cpp
// 1. 创建新的基础接口
// presentation/ui/core/UIElement.hpp
class IUIElement { /* 新接口定义 */ };
class UIElement : public IUIElement { /* 基础实现 */ };

// 2. 创建适配器基类
// presentation/ui/migration/LegacyAdapter.hpp
class LegacyComponentAdapter : public UIElement {
    // 适配旧IUiComponent到新架构
};

// 3. 创建渲染上下文适配器
// presentation/ui/migration/RenderContextAdapter.hpp
class RenderContextAdapter : public IRenderContext {
    // 将新渲染调用转换为旧FrameData
};
```

#### 验证标准
- [ ] 现有UI功能完全正常
- [ ] 新基础类单元测试通过
- [ ] 适配器正确工作
- [ ] 性能无显著下降

### 阶段2：控件逐步迁移（第3-6周）

#### 迁移优先级

1. **高优先级**：基础控件（Button、Label、Panel）
2. **中优先级**：容器控件（Grid、ScrollViewer）
3. **低优先级**：复杂控件（NavigationView、DataGrid）

#### Button控件迁移示例

```cpp
// 第一步：创建新Button类（保持接口兼容）
// presentation/ui/controls/button/Button.hpp
class Button : public ContentControl {
public:
    // 新的构造函数，支持旧参数
    Button(const QString& text = QString());
    
    // 兼容旧接口
    void setOnClick(std::function<void()> callback) {
        setClickHandler(callback);
    }
};

// 第二步：逐步替换使用点
void migrateButtonUsage() {
    // 旧代码
    // auto button = std::make_unique<UiPushButton>("Click Me");
    
    // 新代码（向前兼容）
    auto button = std::make_unique<Button>("Click Me");
    button->setOnClick([]() { /* same callback */ });
}

// 第三步：更新构建系统
# CMakeLists.txt
# 新增Button相关文件，暂时保留旧文件
list(APPEND UI_SOURCES
    "presentation/ui/controls/button/Button.cpp"
    "presentation/ui/controls/button/ButtonRenderer.cpp"
    # "presentation/ui/widgets/UiPushButton.cpp"  # 暂时保留
)
```

#### 迁移清单模板

```markdown
## [控件名称] 迁移清单

### 准备工作
- [ ] 分析现有功能和依赖
- [ ] 设计新架构接口
- [ ] 创建迁移适配器

### 实现阶段  
- [ ] 实现新控件类
- [ ] 迁移渲染逻辑
- [ ] 迁移事件处理
- [ ] 迁移动画效果

### 集成测试
- [ ] 单元测试通过
- [ ] 集成测试通过
- [ ] 视觉测试通过
- [ ] 性能测试通过

### 部署切换
- [ ] 更新使用点
- [ ] 移除旧代码
- [ ] 更新文档
```

### 阶段3：Window系统集成（第7-8周）

#### MainOpenGlWindow迁移

```cpp
// 当前架构
class MainOpenGlWindow : public QOpenGLWindow {
    // 直接管理UI组件
    std::unique_ptr<UiRoot> m_uiRoot;
    std::unique_ptr<UiTopBar> m_topBar;
};

// 目标架构
class MainWindow : public Window {
    // Window作为UI体系一部分
protected:
    void onRender(IRenderContext& context) override;
    QSize onMeasure(const SizeConstraints& constraints) override;
};

class MainOpenGlWindow : public QOpenGLWindow {
    // 委托给UI系统的Window
    std::unique_ptr<MainWindow> m_window;
    
public:
    void paintGL() override {
        // 委托给UI系统
        OpenGLRenderContext context;
        m_window->render(context);
    }
};
```

#### 迁移步骤

1. **保持现有MainOpenGlWindow**，但重构内部实现
2. **创建新的MainWindow类**，继承自Window
3. **逐步迁移功能**到新Window类
4. **最终重构**MainOpenGlWindow为薄包装层

### 阶段4：高级功能集成（第9-12周）

#### MVVM系统集成

```cpp
// 当前绑定系统
class NavViewModel {
    void setupBinding() {
        observe(expanded, [this](bool value) {
            requestRebuild();
        });
    }
};

// 新绑定系统
class NavViewModel : public ObservableObject {
public:
    Property<bool> expanded{this, "expanded", false};
    
    Command toggleCommand{[this]() {
        expanded.setValue(!expanded.getValue());
    }};
};

// 在UI中使用
auto navView = std::make_unique<NavigationView>();
navView->bind("isExpanded", viewModel, "expanded");
navView->setCommand("toggle", &viewModel->toggleCommand);
```

## 风险控制和回滚策略

### 风险识别

1. **性能风险**：新架构可能影响渲染性能
2. **兼容性风险**：适配器可能不完全兼容
3. **稳定性风险**：新代码可能引入新bug
4. **进度风险**：迁移时间可能超预期

### 回滚策略

```cpp
// 1. 保留旧代码直到确认稳定
#if ENABLE_NEW_UI_SYSTEM
    #include "NewButton.hpp"
    using ButtonType = NewButton;
#else
    #include "UiPushButton.hpp"  
    using ButtonType = UiPushButton;
#endif

// 2. 运行时切换机制
class UISystemManager {
public:
    static bool useNewSystem() {
        return QSettings().value("ui/use_new_system", false).toBool();
    }
    
    template<typename T>
    static std::unique_ptr<T> createControl() {
        if (useNewSystem()) {
            return std::make_unique<typename T::NewType>();
        } else {
            return std::make_unique<typename T::LegacyType>();
        }
    }
};
```

### 监控指标

1. **性能指标**：FPS、内存使用、启动时间
2. **稳定性指标**：崩溃率、错误日志
3. **兼容性指标**：功能回归测试通过率
4. **用户体验**：响应时间、动画流畅度

## 测试策略

### 自动化测试

```cpp
// 1. 单元测试：每个新组件
TEST(ButtonTest, BasicFunctionality) {
    Button button("Test");
    EXPECT_EQ(button.text(), "Test");
    
    bool clicked = false;
    button.setClickHandler([&clicked]() { clicked = true; });
    
    // 模拟点击
    InputEvent clickEvent{InputEventType::MousePress, QPoint(10, 10)};
    button.onInput(clickEvent);
    
    EXPECT_TRUE(clicked);
}

// 2. 兼容性测试：适配器正确性
TEST(AdapterTest, LegacyCompatibility) {
    auto legacy = std::make_unique<UiPushButton>("Legacy");
    auto adapter = std::make_unique<LegacyButtonAdapter>(std::move(legacy));
    
    // 验证行为一致性
    EXPECT_EQ(adapter->text(), "Legacy");
}

// 3. 性能测试：渲染性能
TEST(PerformanceTest, RenderingSpeed) {
    auto window = createTestWindow();
    auto startTime = std::chrono::high_resolution_clock::now();
    
    for (int i = 0; i < 1000; ++i) {
        MockRenderContext context;
        window->render(context);
    }
    
    auto duration = std::chrono::high_resolution_clock::now() - startTime;
    EXPECT_LT(duration, std::chrono::milliseconds(100));
}
```

### 集成测试

```cpp
// UI流程测试
TEST(IntegrationTest, CompleteUserFlow) {
    auto app = createTestApp();
    auto window = app->mainWindow();
    
    // 模拟用户操作序列
    auto button = window->findChild<Button>("themeToggle");
    ASSERT_NE(button, nullptr);
    
    // 点击按钮
    simulateClick(button);
    
    // 验证主题变化
    auto topBar = window->findChild<TopBar>("topBar");
    EXPECT_TRUE(topBar->isDarkTheme());
}
```

## 最佳实践

### 1. 代码组织

```cpp
// 保持清晰的文件结构
presentation/ui/
├── migration/          # 迁移相关代码（临时）
│   ├── LegacyAdapter.hpp
│   └── RenderAdapter.hpp
├── core/              # 核心基础设施
│   ├── UIElement.hpp
│   └── RenderContext.hpp
└── controls/          # 新控件实现
    ├── button/
    └── navigation/
```

### 2. 版本控制策略

```bash
# 创建迁移分支
git checkout -b ui-framework-migration

# 小步提交，便于回滚
git commit -m "feat: add UIElement base class"
git commit -m "feat: add Button control implementation"
git commit -m "feat: migrate first usage of Button"

# 定期合并到主分支
git checkout main
git merge ui-framework-migration
```

### 3. 文档同步更新

每个迁移阶段都需要：
1. 更新API文档
2. 更新使用示例
3. 更新迁移进度
4. 更新已知问题列表

## 故障排除

### 常见问题

1. **编译错误**：新旧头文件冲突
   ```cpp
   // 解决方案：使用命名空间隔离
   namespace Legacy { class Button; }
   namespace New { class Button; }
   ```

2. **运行时崩溃**：适配器转换错误
   ```cpp
   // 解决方案：增加类型检查
   if (auto* adapted = dynamic_cast<AdaptedType*>(ptr)) {
       // 安全转换
   }
   ```

3. **性能下降**：多层适配开销
   ```cpp
   // 解决方案：直接迁移关键路径
   #ifdef PERFORMANCE_CRITICAL
       return legacyImplementation();
   #else
       return adaptedImplementation();
   #endif
   ```

### 调试工具

```cpp
// 迁移状态监控
class MigrationMonitor {
public:
    static void logComponentUsage(const std::string& component, bool isNew) {
        if (isNew) {
            qDebug() << "Using NEW" << component.c_str();
        } else {
            qDebug() << "Using LEGACY" << component.c_str();
        }
    }
};

// 性能对比工具
class PerformanceComparator {
public:
    template<typename Func>
    static void compare(const std::string& name, Func legacy, Func newImpl) {
        auto legacyTime = measureTime(legacy);
        auto newTime = measureTime(newImpl);
        
        qDebug() << name.c_str() << "- Legacy:" << legacyTime 
                 << "ms, New:" << newTime << "ms";
    }
};
```

通过遵循这个迁移指南，可以确保UI框架重构过程的平稳进行，最大程度降低对现有功能的影响，同时逐步建立起新的、更加灵活和可维护的UI架构。