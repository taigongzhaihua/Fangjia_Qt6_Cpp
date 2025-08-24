#include "AppConfig.h"
#include "DataPage.h"
#include "ServiceLocator.h"
#include "UiFormulaView.h"
#include <memory>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <TabViewModel.h>
#include <UiPage.h>
#include <UiTabView.h>

DataPage::DataPage()
{
	setTitle("数据");
	DataPage::initializeContent();
}

DataPage::~DataPage() = default;

void DataPage::initializeContent()
{
	UiPage::initializeContent();
	// 初始化标签页
	m_tabsVm.setItems(QVector<TabViewModel::TabItem>{
		{.id = "formula", .label = "方剂", .tooltip = "中医方剂数据库"},
		{ .id = "herb", .label = "中药", .tooltip = "中药材信息" },
		{ .id = "classic", .label = "经典", .tooltip = "经典医籍" },
		{ .id = "case", .label = "医案", .tooltip = "临床医案记录" },
		{ .id = "internal", .label = "内科", .tooltip = "内科诊疗" },
		{ .id = "diagnosis", .label = "诊断", .tooltip = "诊断方法" }
	});

	// 从配置恢复最近的Tab
	if (auto config = DI.get<AppConfig>()) {
		if (!config->recentTab().isEmpty()) {
			int tabIdx = m_tabsVm.findById(config->recentTab());
			if (tabIdx >= 0) {
				m_tabsVm.setSelectedIndex(tabIdx);
			}
		}

		// 连接Tab变化到配置保存
		QObject::connect(&m_tabsVm, &TabViewModel::selectedIndexChanged,
			[config](int) {
				config->setRecentTab(config->recentTab());
				config->save();
			});
	}

	// 创建方剂视图
	m_formulaView = std::make_unique<UiFormulaView>();

	// 设置TabView
	m_tabView.setViewModel(&m_tabsVm);
	m_tabView.setIndicatorStyle(UiTabView::IndicatorStyle::Bottom);
	m_tabView.setTabHeight(43);
	m_tabView.setAnimationDuration(220);
	m_tabView.setContent(0, m_formulaView.get());

	// 设置内容为TabView
	setContent(&m_tabView);
}


void DataPage::applyPageTheme(bool isDark)
{
	// 设置TabView的主题
	if (isDark) {
		m_tabView.setPalette(UiTabView::Palette{
			.barBg = QColor(255,255,255,10),
			.tabHover = QColor(255,255,255,20),
			.tabSelectedBg = QColor(100,100,100,128),
			.indicator = QColor(0,122,255,220),
			.label = QColor(230,240,250,255),
			.labelSelected = QColor(255,255,255,255)
			});
	}
	else {
		m_tabView.setPalette(UiTabView::Palette{
			.barBg = QColor(0,0,0,6),
			.tabHover = QColor(0,0,0,10),
			.tabSelectedBg = QColor(0,0,0,14),
			.indicator = QColor(0,102,204,220),
			.label = QColor(70,76,84,255),
			.labelSelected = QColor(40,46,54,255)
			});
	}

	// 设置方剂视图的主题
	if (m_formulaView) {
		m_formulaView->setDarkTheme(isDark);
	}
}