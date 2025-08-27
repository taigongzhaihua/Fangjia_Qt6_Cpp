# Fangjia Qt6 C++ 架构设计

目标
- 清晰：严格分层，单向依赖
- 可扩展：新增功能模块无需修改核心代码
- 可维护：最少的全局状态，构造注入，小而稳定的接口
- 现代 CMake + targets；服务与视图模型可测试

高层分层（自上而下）
- App（组装根、窗口、导航）
- Features（Home、Data/Formula、Explore、Favorites、Settings）
- UI Framework（容器/布局、基础/高级控件、声明式外观）
- Rendering（渲染引擎、图标缓存/加载器、渲染数据）
- Core（配置/设置、主题管理、工具）
- Platform（可选的跨平台特定辅助）

依赖规则
App → Features → UI Framework → Rendering → Core
App → Core
App → Platform（可选）
禁止向上依赖。Feature 之间互不依赖。

建议的项目结构
- src/
  - app/
    - main.cpp
    - MainOpenGlWindow.{h,cpp}          // 承载组装与导航
    - Navigation/PageRouter.{h,cpp}     // 依据 page id 的路由，惰性工厂
  - core/
    - config/
      - AppConfig.{h,cpp}               // 唯一的持久化（QSettings）
      - ISettings.hpp                   // 可选接口
    - theme/
      - ThemeManager.{h,cpp}            // 不内置 QSettings；持久化通过 AppConfig
      - IThemeService.hpp               // 可选接口
    - utils/
      - Logging.hpp, Result.hpp, NotNull.hpp, ...
  - rendering/
    - RenderData.hpp
    - Renderer.{h,cpp}
    - IconCache.{h,cpp}                 // 新：拥有缓存，线程/GL 语义
    - IconLoader.{h,cpp}                // 无状态的加载函数，供 IconCache 使用
    - RenderContext.hpp                 // 新：DPR、QOpenGLFunctions* 等
  - ui/
    - base/（IThemeAware, IUiComponent, ILayoutable）
    - containers/（UiRoot, UiPage, UiPanel, UiGrid, PageRouter 集成）
    - widgets/（UiNav, UiTopBar, UiTabView, UiTreeList）
    - declarative/（Widget + Basic/Advanced/Layouts/TabView/ComponentWrapper/RebuildHost）
  - features/
    - home/   （HomePage.{h,cpp}，可选 VM）
    - data/
      - formula/
        - FormulaViewModel.{h,cpp}
        - UiFormulaView.{h,cpp}
    - explore/（ExplorePage）
    - favorites/（FavoritesPage）
    - settings/（SettingsPage）
  - platform/
    - windows/（WinWindowChrome.{h,cpp}）
- resources/（qrc、图标、字体）
- tests/（核心/主题/VM 的单元测试）
- docs/ARCHITECTURE.md（本文件）

关键架构决策

1) 用 Composition Root 替代 ServiceLocator
- 在 main.cpp（或小型 AppBuilder）中构建所有服务，并通过构造函数注入。
- 移除 ServiceRegistry/ServiceLocator，转为显式装配。

示例（伪代码）:
```cpp
int main(int argc, char** argv) {
  QApplication app(argc, argv);
  AppConfig config;                          // 持久化设置
  ThemeManager theme;                        // 不内置 QSettings
  theme.setMode(fromString(config.themeMode()));
  QObject::connect(&theme, &ThemeManager::modeChanged, [&]{
    config.setThemeMode(toString(theme.mode())); config.save();
  });

  MainOpenGlWindow window{ /* 以指针/引用注入 */ &config, &theme };
  window.show();
  return app.exec();
}
```

2) Core 负责持久化；ThemeManager 对存储保持无状态
- ThemeManager 不应在内部读写 QSettings。
- AppConfig 是唯一的持久化来源（主题、导航、窗口几何等）。
- 移除 ThemeManager::load/save 的 QFile/QSettings 逻辑。枚举与设置字符串的映射由 App 层完成。

3) 渲染：将缓存与加载拆分，定义 RenderContext
- IconLoader 改为一组无状态函数（renderSvgToImage, renderTextToImage）。
- IconCache 拥有纹理缓存（id->size）、键生成与 GL 生命周期（创建/删除）语义。在单个位置隔离纹理生命周期。
- 定义 RenderContext：
  - QOpenGLFunctions* gl
  - float devicePixelRatio
  - IconCache& icons
- 组件在 append() 或每帧时从 App/UIRoot 获取上下文；避免在深层长期持有 gl/dpr。

4) UI 框架保持保留式（retained mode），统一上下文传递
- IUiComponent API 基本不变，但减少 updateResourceContext 的“抖动”：
  - 兼容方案：暂时保留 updateResourceContext。
  - 后续方案：给 append() 提供 RenderContext 参数，逐步移除 updateResourceContext。

5) Feature 垂直切片
- 每个 Feature（如 Data/Formula）内聚视图、VM、适配器。
- Page 只暴露 IUiComponent + 生命周期钩子（applyTheme, updateLayout）。
- Feature 之间不互相引用。

6) 导航 = Router + 工厂，页面惰性创建
- PageRouter（原 PageManager）存储页面工厂：id -> std::function<std::unique_ptr<UiPage>()>。
- switchTo(id) 时惰性创建并缓存实例；旧实例可选择淘汰（可选策略）。
- MainOpenGlWindow 组装导航 → 路由 → UiRoot，并传播主题。

7) 主题：事件驱动，单一来源，单一映射
- 有效主题 = ThemeManager::effectiveColorScheme → App 的主题枚举（Light/Dark）。
- App 连接 ThemeManager.modeChanged → 写入 AppConfig 并保存。
- UiRoot 将主题变化传播到所有组件；组件不直接读取设置。

8) 资源：单套图标 + 着色
- 仅保留一套逻辑图标（移除 light/dark 重复），使用“白膜纹理 + tint”动态上色。
- 缓存键格式由 RenderUtils::make*Key 统一；避免在控件中写重复 helper。

9) 错误处理与日志
- 统一的 qDebug/qWarning 包装宏与分类。
- 避免跨 Qt 事件边界抛异常；在组装期快速失败。

10) 构建系统（现代 CMake）
- 按层目标；PRIVATE/PUBlIC 链接清晰；只通过 target 暴露包含目录。
- 目标示例：
  - fangjia_core（config、theme、utils）
  - fangjia_rendering（renderer、icon cache/loader、data）
  - fangjia_ui（base/containers/widgets/declarative）
  - fangjia_features_home / fangjia_features_data / ...
  - fangjia_app（MainOpenGlWindow、router）→ 链接其他目标
- 可按需合并小目标以减少脚本噪音；即便合并，也保持边界。

关键接口（草图）

ISettings（可选）
```cpp
struct ISettings {
  virtual ~ISettings() = default;
  virtual QVariant value(const QString& key, const QVariant& def={}) const = 0;
  virtual void setValue(const QString& key, const QVariant&) = 0;
  virtual void sync() = 0;
};
```

IThemeService（可选）
```cpp
struct IThemeService {
  enum class Mode { FollowSystem, Light, Dark };
  virtual ~IThemeService() = default;
  virtual Mode mode() const = 0;
  virtual void setMode(Mode) = 0;
  virtual Qt::ColorScheme effectiveColorScheme() const = 0;
  // 信号：modeChanged, effectiveColorSchemeChanged
};
```

RenderContext
```cpp
struct RenderContext {
  QOpenGLFunctions* gl{nullptr};
  float dpr{1.0f};
  IconCache* icons{nullptr};
};
```

PageRouter
```cpp
class PageRouter {
public:
  using Factory = std::function<std::unique_ptr<UiPage>()>;
  void registerPage(const QString& id, Factory f);
  bool switchTo(const QString& id);
  UiPage* current() const;
private:
  std::unordered_map<QString, Factory> m_factories;
  std::unordered_map<QString, std::unique_ptr<UiPage>> m_pages;
  UiPage* m_current{nullptr};
};
```

主题与配置数据流
system → ThemeManager.effectiveColorScheme()
→ MainOpenGlWindow::Theme Light/Dark
→ UiRoot.propagateThemeChange(isDark)
→ 组件根据主题调整配色
→ ThemeManager.modeChanged 时：AppConfig.Theme/Mode 更新并保存

迁移指南

阶段 0：快速收益（无行为变化）
- 移除 ServiceRegistry::registerViewModels 及其包含；必要时仅保留核心服务。
- 停止 ThemeManager 内部通过 QSettings 持久化；改由 AppConfig 在 App 层持久化。
- 删除 RebuildHost.cpp 空实现；仅保留头文件。
- 将 UiTopBar::iconCacheKey 合并到 RenderUtils::makeIconCacheKey。
- 从 IconLoader 的 renderSvgToImage/ensureSvgPx 签名中移除未使用的颜色参数（连同调用点）。

阶段 1：Composition Root + DI 清理
- 在 main.cpp 构建 AppConfig 与 ThemeManager；连接 modeChanged → config 保存。
- 通过构造函数把 &config 和 &theme 注入 MainOpenGlWindow（使用原始指针或 QPointer；若 main 拥有，则不使用 shared_ptr）。
- 删除 ServiceLocator/ServiceRegistry 与相关宏；把 MainOpenGlWindow::initializeServices 中的获取逻辑改为构造注入。
- 统一主题持久化：仅 AppConfig 持久化。

阶段 2：Router + Features
- PageManager 重命名为 PageRouter（注册工厂，按需/惰性构建页面）。
- 将页面移动到 features/* 目录；头文件保持最小化。
- 将导航 id 与 Router id 直接对齐。

阶段 3：Rendering Context 加强
- 引入 IconCache，持有纹理缓存（id、尺寸、GL 删除）。
- 将 IconLoader 改为无状态 helper，由 IconCache 调用。
- 引入 RenderContext，并开始贯穿传递（过渡期可保留 updateResourceContext 兼容）。

阶段 4：资源整合
- 用单套图标 + tint 替换 *_light/*_dark；清理 qrc。
- 统一缓存键生成到 RenderUtils（移除零散实现）。

阶段 5：测试与工具
- 增加单元测试（Qt Test）：
  - ThemeManager（模式切换、跟随系统行为）
  - AppConfig（键值映射、保存/加载）
  - TabViewModel（选择语义）
- 增加 clang-tidy（modernize-*），在 CI 中视为错误。

线程与 GL 注意事项
- 所有 GL 资源创建/删除（IconCache 纹理、Renderer 程序）必须在活动 GL 线程执行。严格保持该不变式。
- 在窗口析构时显式释放图标纹理。

附录：命名与约定
- 按层命名空间：core::、rendering::、ui::、features::data::、app::
- 避免宏（除最少的平台宏）。使用 enum class 替代 int 标志（除非 Qt API 需要）。
- 最小化 QObject 单例的使用。
