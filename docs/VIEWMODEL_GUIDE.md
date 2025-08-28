# ViewModel 编写约定（MVVM）

核心原则
- View 仅负责渲染与转发交互；
- ViewModel 暴露状态（Q_PROPERTY + NOTIFY）与命令（公共方法），持久化/服务经构造注入；
- Model/Service 与 VM 解耦，便于单元测试。

基础骨架
```cpp
class DataViewModel : public QObject {
  Q_OBJECT
  Q_PROPERTY(int selectedTab READ selectedTab WRITE setSelectedTab NOTIFY selectedTabChanged)
public:
  explicit DataViewModel(AppConfig& cfg, QObject* parent=nullptr);
  int selectedTab() const { return m_selected; }
  void setSelectedTab(int i);
signals:
  void selectedTabChanged(int);
private:
  AppConfig& m_cfg; // 构造注入的服务
  int m_selected{0};
};
```

属性与信号
- 命名：prop → propChanged；布尔命名避免 isX + isXChanged 的不对称。
- NOTIFY 必要时包含变化参数（如索引/ID），便于绑定层快速使用。

命令（可选）
- 直接使用公共方法即可（如 cycleMode()/setExpanded()）。
- 复杂命令可引入轻量 Command 包装（可选，不强制）。

持久化约定
- 持久化统一走 AppConfig；VM 内只负责触发/调用 AppConfig 写入。
- 读写键集中定义于 AppConfig::Keys，避免散落。

依赖注入
- 构造函数注入（推荐）或成员引用；避免全局定位器。

测试建议
- 使用 Qt Test 针对 VM 的状态机编写测试（切换/持久化/边界条件）。