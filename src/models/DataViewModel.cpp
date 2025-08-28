#include "DataViewModel.h"
#include "AppConfig.h"
#include <qlogging.h>

DataViewModel::DataViewModel(AppConfig& config, QObject* parent)
    : ViewModelBase(parent)
    , m_config(config)
    , m_tabViewModel(std::make_unique<TabViewModel>(this))
{
    // 初始化标签页数据
    initializeTabs();
    
    // 连接信号槽，监听标签页选中变更
    connect(m_tabViewModel.get(), &TabViewModel::selectedIndexChanged, 
            this, &DataViewModel::onTabSelectionChanged);
    
    // 从配置恢复最近标签页
    restoreRecentTab();
}

int DataViewModel::selectedTab() const
{
    return m_tabViewModel->selectedIndex();
}

void DataViewModel::onTabSelectionChanged(int index)
{
    // 写回 AppConfig 并保存
    const auto& items = m_tabViewModel->items();
    if (index >= 0 && index < items.size()) {
        const QString& tabId = items[index].id;
        qDebug() << "DataViewModel: Tab changed to" << tabId << "at index" << index;
        
        m_config.setRecentTab(tabId);
        m_config.save();
    }
    
    // 发射属性变更通知
    emit selectedTabChanged(index);
}

void DataViewModel::initializeTabs()
{
    // 初始化标签页数据（与原 DataPage::Impl 保持一致）
    m_tabViewModel->setItems(QVector<TabViewModel::TabItem>{
        {.id = "formula", .label = "方剂", .tooltip = "中医方剂数据库"},
        {.id = "herb", .label = "中药", .tooltip = "中药材信息"},
        {.id = "classic", .label = "经典", .tooltip = "经典医籍"},
        {.id = "case", .label = "医案", .tooltip = "临床医案记录"},
        {.id = "internal", .label = "内科", .tooltip = "内科诊疗"},
        {.id = "diagnosis", .label = "诊断", .tooltip = "诊断方法"}
    });
}

void DataViewModel::restoreRecentTab()
{
    // 从配置恢复最近标签页
    const QString recentTabId = m_config.recentTab();
    if (!recentTabId.isEmpty()) {
        const int tabIdx = m_tabViewModel->findById(recentTabId);
        if (tabIdx >= 0) {
            qDebug() << "DataViewModel: Restoring recent tab" << recentTabId << "at index" << tabIdx;
            m_tabViewModel->setSelectedIndex(tabIdx);
        }
    }
}