#include "AppConfig.h"
#include "DataPage.h"
#include "ServiceLocator.h"
#include "UI.h"
#include "UiFormulaView.h"
#include <BasicWidgets.h>
#include <ComponentWrapper.h>
#include <Layouts.h>
#include <memory>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qstring.h>
#include <TabViewModel.h>
#include <UiComponent.hpp>
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

	WidgetPtr buildUI() {
		return column({
			// Tab栏容器
			container(
				buildTabBar()
			)->height(43)->background(
				isDark ? QColor(255,255,255,10) : QColor(0,0,0,6)
			),

				// 内容区域
				expanded(
					buildContent()
				)
			});
	}

private:
	WidgetPtr buildTabBar() {
		auto items = tabsVm.items();
		WidgetList tabs;

		for (int i = 0; i < items.size(); ++i) {
			tabs.push_back(buildTab(items[i], i));
		}

		return row(tabs)->mainAxisAlignment(Alignment::Start);
	}

	WidgetPtr buildTab(const TabViewModel::TabItem& item, int index) {
		const bool selected = (index == tabsVm.selectedIndex());

		return button(
			container(
				text(item.label)
				->fontSize(14)
				->color(selected ?
					(isDark ? QColor(255, 255, 255, 255) : QColor(40, 46, 54, 255)) :
					(isDark ? QColor(230, 240, 250, 255) : QColor(70, 76, 84, 255))
				)
			)->padding(16, 10)
		)
			->style(Button::ButtonStyle::Text)
			->onTap([this, index]() {
			tabsVm.setSelectedIndex(index);
			rebuildContent();
				});
	}

	WidgetPtr buildContent() {
		const QString selectedId = tabsVm.selectedId();

		if (selectedId == "formula") {
			return buildFormulaContent();
		}

		// 其他标签页的占位内容
		return container(
			column({
				icon(":/icons/development.svg")->size(64)->color(QColor(150,150,150)),
				spacer(16),
				text("功能开发中...")
					->fontSize(16)
					->color(QColor(130,130,130))
				})->mainAxisAlignment(Alignment::Center)
		)->alignment(Alignment::Center);
	}

	WidgetPtr buildFormulaContent() {
		// 使用新的 wrap 函数包装现有的 UiFormulaView
		return wrap(formulaView.get());
	}

	void rebuildContent() {
		// 这里需要通知父组件重新构建
		// 暂时通过保存配置触发
		if (auto config = DI.get<AppConfig>()) {
			config->setRecentTab(tabsVm.selectedId());
		}
	}
};

// DataPage 实现
DataPage::DataPage() : m_impl(std::make_unique<Impl>()) {
	setTitle("数据");
	initializeContent();
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

	// 重新构建UI
	auto widget = m_impl->buildUI();
	m_impl->builtComponent = widget->build();
	setContent(m_impl->builtComponent.get());
}

TabViewModel* DataPage::tabViewModel() {
	return &m_impl->tabsVm;
}