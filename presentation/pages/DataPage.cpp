#include "AppConfig.h"
#include "DataPage.h"
#include "DataViewModel.h"
#include "FormulaContent.h"
#include "FormulaRepository.h"
#include "FormulaService.h"
#include "FormulaViewModel.h"
#include "GetRecentTabUseCase.h"
#include "SetRecentTabUseCase.h"
#include "SettingsRepository.h"
#include "UI.h"
#include <BasicWidgets.h>
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
	std::unique_ptr<FormulaViewModel> formulaViewModel;
	std::shared_ptr<FormulaContent> formulaContent;  // Changed to shared_ptr for UI system
	std::unique_ptr<IUiComponent> builtComponent;
	bool isDark = false;

	explicit Impl(AppConfig* config)
	{
		// Create local use cases for DataViewModel using the provided AppConfig
		// This bridges the legacy AppConfig to the new domain use cases
		auto settingsRepository = std::make_shared<data::repositories::SettingsRepository>(
			std::shared_ptr<AppConfig>(config, [](AppConfig*) {})); // Non-owning shared_ptr
		auto getRecentTabUseCase = std::make_shared<domain::usecases::GetRecentTabUseCase>(settingsRepository);
		auto setRecentTabUseCase = std::make_shared<domain::usecases::SetRecentTabUseCase>(settingsRepository);

		// Create DataViewModel with domain use cases
		dataViewModel = std::make_unique<DataViewModel>(getRecentTabUseCase, setRecentTabUseCase);

		// Create Formula service chain: Repository → Service → ViewModel
		auto formulaRepository = std::make_shared<data::repositories::FormulaRepository>();
		auto formulaService = std::make_shared<domain::services::FormulaService>(formulaRepository);

		// Create FormulaViewModel with service injection
		formulaViewModel = std::make_unique<FormulaViewModel>(formulaService);
		formulaViewModel->loadData(); // Load data (from service or fallback to sample)

		// 创建方剂内容组件（非拥有ViewModel）
		formulaContent = std::make_shared<FormulaContent>(formulaViewModel.get());
	}

	WidgetPtr buildUI() const
	{
		return tabView()
			// 使用适配器替代直接使用 TabViewModel（实现 UI-ViewModel 解耦）
			->dataProvider(dataViewModel->tabs())
			->indicatorStyle(UiTabView::IndicatorStyle::Bottom)
			->tabHeight(43)
			->animationDuration(220)
			->contents({
			// 仅为前几个 Tab 提供示例内容，其他 Tab 可按需补充
			formulaContent,  // FormulaContent widget
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

	// FormulaContent handles theme automatically through declarative widgets
	// No manual theme application needed
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
