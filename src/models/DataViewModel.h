#pragma once
#include "ViewModelBase.hpp"
#include "TabViewModel.h"
#include <memory>

// 前向声明
class AppConfig;

// DataPage 的 ViewModel，负责管理标签页状态和与 AppConfig 的持久化交互
class DataViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(int selectedTab READ selectedTab NOTIFY selectedTabChanged)

public:
    explicit DataViewModel(AppConfig& config, QObject* parent = nullptr);
    ~DataViewModel() override = default;

    // 标签页相关
    TabViewModel* tabs() const { return m_tabViewModel.get(); }
    
    // Q_PROPERTY 访问器
    int selectedTab() const;

signals:
    void selectedTabChanged(int index);

private slots:
    // 监听 TabViewModel 的选中变更，写回 AppConfig
    void onTabSelectionChanged(int index);

private:
    void initializeTabs();
    void restoreRecentTab();

private:
    AppConfig& m_config;
    std::unique_ptr<TabViewModel> m_tabViewModel;
};