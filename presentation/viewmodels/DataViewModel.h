#pragma once
#include "ViewModelBase.hpp"
#include "TabViewModel.h"
#include <memory>

// Forward declaration
class AppConfig;

// DataPage 的 ViewModel - transitional implementation during refactor
class DataViewModel : public ViewModelBase
{
    Q_OBJECT
    Q_PROPERTY(int selectedTab READ selectedTab NOTIFY selectedTabChanged)

public:
    // Keep old constructor for backward compatibility during refactor
    explicit DataViewModel(AppConfig& config, QObject* parent = nullptr);
    ~DataViewModel() override = default;

    // 标签页相关
    TabViewModel* tabs() const { return m_tabViewModel.get(); }
    
    // Q_PROPERTY 访问器
    int selectedTab() const;

signals:
    void selectedTabChanged(int index);

private slots:
    // 监听 TabViewModel 的选中变更，通过用例更新设置
    void onTabSelectionChanged(int index);

private:
    void initializeTabs();
    void restoreRecentTab();

private:
    AppConfig& m_config; // Temporary: will be replaced with use cases
    std::unique_ptr<TabViewModel> m_tabViewModel;
};