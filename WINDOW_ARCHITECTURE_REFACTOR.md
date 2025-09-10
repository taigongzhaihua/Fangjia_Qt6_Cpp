# 窗口架构重构说明

## 问题分析

原始架构存在以下问题：
1. 窗口循环和事件处理逻辑直接放在 MainOpenGlWindow 中，缺乏封装性
2. MainOpenGlWindow 直接继承自 QOpenGLWindow，框架与业务耦合
3. main.cpp 直接使用 QApplication，应用程序逻辑缺乏抽象
4. 代码复用性差，难以扩展新的窗口类型

## 解决方案

### 1. 创建基础 Window 类 (`presentation/ui/base/Window.hpp/.cpp`)

**职责：**
- 封装 OpenGL 窗口的通用功能
- 提供标准事件循环处理（鼠标、键盘、滚轮）
- 管理主题切换和动画循环
- 定义窗口生命周期虚函数接口

**设计原则：**
- 提供通用窗口功能的基础实现
- 子类通过虚函数定制特定行为
- 避免业务逻辑耦合，保持框架层纯净性

**关键接口：**
```cpp
// 子类必须实现的虚函数
virtual void initializeWindowGL() = 0;
virtual void updateWindowLayout(int w, int h) = 0;
virtual void renderWindow() = 0;
virtual void onThemeChanged(Theme newTheme) = 0;
virtual bool onAnimationTick() = 0;
```

### 2. 创建基础 Application 类 (`presentation/ui/base/Application.hpp/.cpp`)

**职责：**
- 封装 Qt 应用程序的通用功能
- 管理应用程序生命周期
- 提供 OpenGL 上下文全局配置
- 设置应用程序元信息

**设计原则：**
- 提供通用应用程序功能的基础实现
- 子类通过虚函数定制特定配置和行为
- 避免业务逻辑耦合，保持框架层纯净性

**关键接口：**
```cpp
// 子类必须实现的虚函数
virtual bool initializeApplication() = 0;
virtual bool createAndShowMainWindow() = 0;
```

### 3. 创建业务应用程序类 (`presentation/FangjiaApp.hpp/.cpp`)

**职责：**
- 继承自基础 Application 类
- 实现房价应用特定的初始化逻辑
- 管理依赖注入容器和服务
- 创建和配置主窗口

**改进：**
- 业务逻辑从 main.cpp 移到专门的 App 类
- 提高代码组织性和可测试性
- 便于扩展和维护

### 4. 重构 MainOpenGlWindow (`presentation/MainOpenGlWindow.hpp/.cpp`)

**变化：**
- 从直接继承 QOpenGLWindow 改为继承基础 Window 类
- 移动到 presentation 层（业务表现层）
- 专注于业务特定的 UI 逻辑，通用功能由基类提供

**改进：**
- 更好的职责分离
- 代码复用性提高
- 更易于测试和维护

### 5. 简化 main.cpp

**变化：**
```cpp
// 之前：直接使用 QApplication 和复杂的初始化逻辑
QApplication app(argc, argv);
// ... 大量初始化代码 ...

// 现在：使用派生的 App 类
FangjiaApp app(argc, argv);
return app.run();
```

**改进：**
- main.cpp 逻辑大幅简化
- 应用程序入口更清晰
- 便于单元测试

## 架构层次

```
Framework Layer (框架层):
├── presentation/ui/base/Window.hpp/.cpp        // 基础窗口类
└── presentation/ui/base/Application.hpp/.cpp   // 基础应用程序类

Business Layer (业务层):
├── presentation/FangjiaApp.hpp/.cpp           // 房价应用程序类
├── presentation/MainOpenGlWindow.hpp/.cpp     // 主窗口类
└── apps/fangjia/main.cpp                      // 应用程序入口
```

## 封装性和复用性改进

### 封装性提升：
1. **窗口循环封装**：通用窗口功能封装在基础 Window 类中
2. **应用程序生命周期封装**：通用应用逻辑封装在基础 Application 类中
3. **业务逻辑分离**：框架代码与业务代码清晰分离

### 复用性提升：
1. **基础类可复用**：其他窗口类可继承基础 Window 类
2. **应用程序模板**：其他应用可继承基础 Application 类
3. **标准化接口**：定义了清晰的扩展点

### 可维护性提升：
1. **职责明确**：每个类的职责更加明确
2. **依赖清晰**：依赖关系更加清晰
3. **测试友好**：更容易进行单元测试

## 构建配置更新

CMakeLists.txt 中新增：
- `fj_presentation_base`: 基础框架类库
- `fj_presentation_app`: 业务应用程序类库
- 更新依赖关系和包含路径

## 验证结果

通过架构验证测试确认：
✓ 窗口循环已封装在基础 Window 类中
✓ MainWindow 继承自基础 Window 类而非直接继承 QOpenGLWindow
✓ 应用程序逻辑已封装在基础 Application 类中
✓ 业务应用使用派生 App 类而非直接使用 QApplication
✓ 框架类与业务逻辑成功分离
✓ 代码复用性和封装性显著改善