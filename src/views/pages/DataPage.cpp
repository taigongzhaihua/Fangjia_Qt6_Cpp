#include "AppConfig.h"
#include "Binding.h"  // 新增：引入绑定功能
#include "DataPage.h"
#include "DataViewModel.h"
#include "UI.h"
#include "UiFormulaView.h"
#include <BasicWidgets.h>
#include <ComponentWrapper.h>
#include <Layouts.h>
#include <memory>
#include <qcolor.h>
#include <qfont.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qstring.h>
#include <RebuildHost.h>
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
		return panel({
			// 添加绑定演示：显示当前选中的标签页信息
			buildTabBindingDemo(),
			spacer(10),

			// 原有的标签页视图
			tabView()
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
			->onChanged([this](int idx)
				{
					// VM 模式下此回调仍会触发，可用于埋点/日志
					// 持久化逻辑已移到 DataViewModel 中处理
					qDebug() << "DataPage: Tab changed to" << idx;
				})
			})->vertical();
	}

private:
	// 新增：构建标签页绑定演示
	WidgetPtr buildTabBindingDemo() const
	{
		return card(
			bindingHost([this]() -> WidgetPtr {
				// 这里展示当前选中标签的信息，当标签切换时会自动重建
				auto tabs = dataViewModel->tabs();
				int selectedIndex = tabs->selectedIndex();
				QString selectedId = tabs->selectedId();
				QString selectedLabel = selectedIndex >= 0 && selectedIndex < tabs->count()
					? tabs->items()[selectedIndex].label : "无";

				return panel({
					text("当前标签页信息")->fontSize(14)->fontWeight(QFont::Medium),
					spacer(5),
					text(QString("索引: %1").arg(selectedIndex))->fontSize(12),
					text(QString("ID: %1").arg(selectedId))->fontSize(12),
					text(QString("标签: %1").arg(selectedLabel))->fontSize(12),
					})->vertical()->crossAxisAlignment(Alignment::Start);
				})->connect([this](UI::RebuildHost* host) {
					// 当标签页选择发生变化时，自动重建信息显示
					UI::observe(dataViewModel->tabs(), &TabViewModel::selectedIndexChanged, [host](int) {
						host->requestRebuild();
						});
					})
					)->elevation(1.0f)
					->backgroundTheme(QColor(245, 248, 255), QColor(25, 30, 40))
					->padding(10);
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
	auto widget = m_impl->buildUI();
	m_impl->builtComponent = widget->build();

	// 设置为页面内容
	setContent(m_impl->builtComponent.get());
}

void DataPage::applyPageTheme(bool isDark)
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
