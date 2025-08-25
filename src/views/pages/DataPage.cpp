#include "AppConfig.h"
#include "DataPage.h"
#include "ServiceLocator.h"
#include "UI.h"
#include "UiFormulaView.h"
#include <BasicWidgets.h>
#include <ComponentWrapper.h>
#include <Layouts.h>
#include <memory>
#include <qcontainerfwd.h>
#include <qlogging.h>
#include <TabViewModel.h>
#include <UiComponent.hpp>
#include <UiTabView.h>
#include <Widget.h>

// 内部实现类
class DataPage::Impl {
public:
	TabViewModel tabsVm;
	std::unique_ptr<UiFormulaView> formulaView;
	std::unique_ptr<IUiComponent> builtComponent;
	bool isDark = false;

	Impl() {
		// 初始化标签页
		tabsVm.setItems(QVector<TabViewModel::TabItem>{
			{.id = "formula", .label = "方剂", .tooltip = "中医方剂数据库"},
			{ .id = "herb", .label = "中药", .tooltip = "中药材信息" },
			{ .id = "classic", .label = "经典", .tooltip = "经典医籍" },
			{ .id = "case", .label = "医案", .tooltip = "临床医案记录" },
			{ .id = "internal", .label = "内科", .tooltip = "内科诊疗" },
			{ .id = "diagnosis", .label = "诊断", .tooltip = "诊断方法" }
		});

		// 创建方剂视图
		formulaView = std::make_unique<UiFormulaView>();

		// 从配置恢复
		if (auto config = DI.get<AppConfig>()) {
			if (!config->recentTab().isEmpty()) {
				int tabIdx = tabsVm.findById(config->recentTab());
				if (tabIdx >= 0) {
					tabsVm.setSelectedIndex(tabIdx);
				}
			}
		}
	}

	WidgetPtr buildUI() const
	{
		return tabView()
			->tabs(QStringList({ "方剂","中药","经典", "医案","内科", "诊断" }))
			->selectedIndex(0)
			->indicatorStyle(UiTabView::IndicatorStyle::Bottom)
			->tabHeight(43)
			->animationDuration(220)
			->contents(WidgetList{
				wrap(formulaView.get()),
				container(text("中药功能开发中")->fontSize(16))->alignment(Alignment::Stretch),
				container(text("经典功能开发中")->fontSize(16))->alignment(Alignment::Center),
				})
				->onChanged([this](int idx) {
			// 非 VM 模式的回调
			qDebug() << "Tab changed to" << idx;
					});
	}

private:


	void rebuildContent() const
	{
		// 这里需要通知父组件重新构建
		// 暂时通过保存配置触发
		if (const auto config = DI.get<AppConfig>()) {
			config->setRecentTab(tabsVm.selectedId());
		}
	}
};

// DataPage 实现
DataPage::DataPage() : m_impl(std::make_unique<Impl>()) {
	setTitle("数据");
	DataPage::initializeContent();
}

DataPage::~DataPage() = default;

void DataPage::initializeContent() {
	// 构建声明式UI
	auto widget = m_impl->buildUI();
	m_impl->builtComponent = widget->build();

	// 设置为页面内容
	setContent(m_impl->builtComponent.get());
}

void DataPage::applyPageTheme(bool isDark) {
	m_impl->isDark = isDark;

	// 传递主题到方剂视图
	if (m_impl->formulaView) {
		m_impl->formulaView->setDarkTheme(isDark);
	}

}

TabViewModel* DataPage::tabViewModel() const
{
	return &m_impl->tabsVm;
}
