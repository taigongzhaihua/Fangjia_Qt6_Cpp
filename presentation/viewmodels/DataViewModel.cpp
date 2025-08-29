#include "DataViewModel.h"
#include "usecases/GetRecentTabUseCase.h"
#include "usecases/SetRecentTabUseCase.h"
#include <qlogging.h>
#include <qdebug.h>

DataViewModel::DataViewModel(std::shared_ptr<domain::usecases::GetRecentTabUseCase> getRecentTabUseCase,
                             std::shared_ptr<domain::usecases::SetRecentTabUseCase> setRecentTabUseCase,
                             QObject* parent)
	: ViewModelBase(parent)
	, m_tabViewModel(std::make_unique<TabViewModel>(this))
	, m_getRecentTabUseCase(std::move(getRecentTabUseCase))
	, m_setRecentTabUseCase(std::move(setRecentTabUseCase))
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

void DataViewModel::onTabSelectionChanged(const int index)
{
	// 写回设置并保存 (Use domain use cases)
	const auto& items = m_tabViewModel->items();
	if (index >= 0 && index < items.size()) {
		const QString& tabId = items[index].id;
		qDebug() << "DataViewModel: Tab changed to" << tabId << "at index" << index;

		if (m_setRecentTabUseCase) {
			m_setRecentTabUseCase->execute(tabId.toStdString());
		}
	}

	// 发射属性变更通知
	emit selectedTabChanged(index);
}

void DataViewModel::initializeTabs()
{
	// 初始化标签页数据（与原 DataPage::Impl 保持一致）
	m_tabViewModel->setItems(QVector<TabViewModel::TabItem>{
		{.id = "formula", .label = "方剂", .tooltip = "中医方剂数据库"},
		{ .id = "herb", .label = "中药", .tooltip = "中药材信息" },
		{ .id = "classic", .label = "经典", .tooltip = "经典医籍" },
		{ .id = "case", .label = "医案", .tooltip = "临床医案记录" },
		{ .id = "internal", .label = "内科", .tooltip = "内科诊疗" },
		{ .id = "diagnosis", .label = "诊断", .tooltip = "诊断方法" }
	});
}

void DataViewModel::restoreRecentTab()
{
	// 从配置恢复最近标签页 (Use domain use cases)
	if (m_getRecentTabUseCase) {
		const auto tabId = m_getRecentTabUseCase->execute();
		const QString recentTabId = QString::fromStdString(tabId);
		
		if (!recentTabId.isEmpty()) {
			const int tabIdx = m_tabViewModel->findById(recentTabId);
			if (tabIdx >= 0) {
				qDebug() << "DataViewModel: Restoring recent tab" << recentTabId << "at index" << tabIdx;
				m_tabViewModel->setSelectedIndex(tabIdx);
			}
		}
	}
}