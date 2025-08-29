#include "AppConfig.h"
#include "DataPage.h"
#include "DataViewModel.h"
#include "UI.h"
#include "UiFormulaView.h"
#include <BasicWidgets.h>
#include <ComponentWrapper.h>
#include <Layouts.h>
#include <memory>
#include <qlogging.h>
#include <qnamespace.h>
#include <TabViewModel.h>
#include <UiComponent.hpp>
#include <UiTabView.h>
#include <Widget.h>

// 内部实现类
class DataPage::Impl
{
public:
	std::unique_ptr<DataViewModel> dataViewModel;
	std::unique_ptr<UiFormulaView> formulaView;
	std::unique_ptr<IUiComponent> builtComponent;
	bool isDark = false;

	explicit Impl(AppConfig* config)
	{
		// 创建 DataViewModel，负责标签页状态和配置交互
		dataViewModel = std::make_unique<DataViewModel>(*config);

		// 创建方剂视图
		formulaView = std::make_unique<UiFormulaView>();
	}

	WidgetPtr buildUI() const
	{
		return tabView()
			// 使用 DataViewModel 中的 TabViewModel（移除对 ServiceLocator 的依赖）
			->viewModel(dataViewModel->tabs())
			->indicatorStyle(UiTabView::IndicatorStyle::Bottom)
			->tabHeight(43)
			->animationDuration(220)
			->contents({
			// 仅为前几个 Tab 提供示例内容，其他 Tab 可按需补充
			wrap(formulaView.get()),
			container(
				text("中药功能开发中")
				->fontSize(16)
				->align(Qt::AlignHCenter)
				->wrap(true)
			)->alignment(Alignment::Stretch),
			container(text("经典功能开发中")->fontSize(16)->align(Qt::AlignCenter))->alignment(Alignment::Stretch),
				})
			->onChanged([this](const int idx)
				{
					// VM 模式下此回调仍会触发，可用于埋点/日志
					// 持久化逻辑已移到 DataViewModel 中处理
					qDebug() << "DataPage: Tab changed to" << idx;
				});
	}

};

// DataPage 实现
DataPage::DataPage(AppConfig* config) : m_impl(std::make_unique<Impl>(config))
{
	setTitle("数据");
	DataPage::initializeContent();
}

DataPage::~DataPage() = default;

void DataPage::initializeContent()
{
	// 构建声明式UI
	const auto widget = m_impl->buildUI();
	m_impl->builtComponent = widget->build();

	// 设置为页面内容
	setContent(m_impl->builtComponent.get());
}

void DataPage::applyPageTheme(const bool isDark)
{
	m_impl->isDark = isDark;

	// 传递主题到方剂视图
	if (m_impl->formulaView)
	{
		m_impl->formulaView->setDarkTheme(isDark);
	}
}

void DataPage::onAppear()
{
	qDebug() << "DataPage: onAppear() - 数据页面显示，可在此加载数据";
}

void DataPage::onDisappear()
{
	qDebug() << "DataPage: onDisappear() - 数据页面隐藏，可在此保存状态";
}

TabViewModel* DataPage::tabViewModel() const
{
	return m_impl->dataViewModel->tabs();
}
