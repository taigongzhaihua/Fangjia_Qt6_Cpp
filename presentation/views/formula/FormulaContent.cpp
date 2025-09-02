#include "FormulaContent.h"

#include "FormulaViewModel.h"
#include "UiTreeList.h"

#include "BasicWidgets.h"
#include "Binding.h"
#include "ComponentWrapper.h"
#include "Layouts.h"
#include "UI.h"

#include <memory>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qfont.h>
#include <qlogging.h>
#include <qstring.h>
#include <RebuildHost.h>
#include <UiComponent.hpp>
#include <Widget.h>

using namespace UI;

FormulaContent::FormulaContent(FormulaViewModel* vm)
	: m_viewModel(vm)
{
	if (!m_viewModel)
	{
		qWarning() << "[FormulaContent] Warning: FormulaViewModel is null";
	}
}

std::unique_ptr<IUiComponent> FormulaContent::build() const
{
	if (!m_viewModel)
	{
		qWarning() << "[FormulaContent] Cannot build UI without valid FormulaViewModel";
		return nullptr;
	}

	// 创建树形列表
	auto treeList = createTreeList();
	const auto treeWidget = wrap(treeList.release());

	// 创建分隔线（1px半透明灰色）
	const auto splitter = coloredBox(QColor(128, 128, 128, 100));

	// 创建详情面板
	const auto detailsPanel = createDetailsPanel();

	// 使用Grid布局：35% + 1px + 65%，单行
	return grid()
		->columns({ 220_px, 1_px, 0.65_fr })
		->rows({ 1_fr })
		->rowSpacing(0)
		->colSpacing(0)
		->add(treeWidget, 0, 0, 1, 1, Grid::CellAlign::Stretch, Grid::CellAlign::Stretch)
		->add(splitter, 0, 1, 1, 1, Grid::CellAlign::Stretch, Grid::CellAlign::Stretch)
		->add(detailsPanel, 0, 2, 1, 1, Grid::CellAlign::Stretch, Grid::CellAlign::Stretch)
		->build();
}

std::unique_ptr<UiTreeList> FormulaContent::createTreeList() const
{
	auto tree = std::make_unique<UiTreeList>();

	// 配置函数式模型绑定，避免适配器类
	tree->setModelFns({
		.rootIndices = [this]() -> QVector<int>
		{
			QVector<int> roots;
			if (!m_viewModel) return roots;
			const auto& nodes = m_viewModel->nodes();
			for (int i = 0; i < nodes.size(); ++i)
			{
				if (nodes[i].parentIndex == -1)
				{
					roots.push_back(i);
				}
			}
			return roots;
		},
		.childIndices = [this](int nodeId) -> QVector<int>
		{
			return m_viewModel ? m_viewModel->childIndices(nodeId) : QVector<int>{};
		},
		.nodeInfo = [this](int nodeId) -> UiTreeList::NodeInfo
		{
			UiTreeList::NodeInfo info;
			if (!m_viewModel) return info;
			const auto& nodes = m_viewModel->nodes();
			if (nodeId < 0 || nodeId >= nodes.size()) return info;
			info.label = nodes[nodeId].label;
			info.level = nodes[nodeId].level;
			info.expanded = nodes[nodeId].expanded;
			return info;
		},
		.selectedId = [this]() -> int
		{
			return m_viewModel ? m_viewModel->selectedIndex() : -1;
		},
		.setSelectedId = [this](int nodeId)
		{
			if (m_viewModel)
			{
				m_viewModel->setSelectedIndex(nodeId);
			}
		},
		.setExpanded = [this](int nodeId, bool expanded)
		{
			if (m_viewModel)
			{
				m_viewModel->setExpanded(nodeId, expanded);
			}
		}
		});


	return tree;
}

WidgetPtr FormulaContent::createDetailsPanel() const
{
	// 使用BindingHost创建响应式详情面板
	return bindingHost([this]() -> WidgetPtr
		{
			return buildDetailsContent();
		})->connect([this](RebuildHost* host)
			{
				// 监听ViewModel信号，触发UI重建
				if (!m_viewModel || !host) return;

				observe(m_viewModel, &FormulaViewModel::selectedChanged, [host](int)
					{
						host->requestRebuild();
					});

				observe(m_viewModel, &FormulaViewModel::dataChanged, [host]()
					{
						host->requestRebuild();
					});

				observe(m_viewModel, &FormulaViewModel::nodeExpandChanged, [host](int, bool)
					{
						// 节点展开不影响详情面板，但为了完整性保留监听
						// host->requestRebuild();
					});
			});
}

WidgetPtr FormulaContent::buildDetailsContent() const
{
	if (!m_viewModel)
	{
		return panel({
			text("未找到数据模型")
			->themeColor(QColor(100, 100, 100), QColor(200, 200, 200))
			->fontSize(14)
			})->vertical()->padding(16);
	}

	const auto* formula = m_viewModel->selectedFormula();
	if (!formula)
	{
		return scrollView(
			panel({
				text("请选择一个方剂查看详情")
				->themeColor(QColor(100, 100, 100), QColor(200, 200, 200))
				->fontSize(14)
				->wrap(true)
				})->vertical()->padding(16)
		);
	}

	// 构建方剂详情内容
	WidgetList contentWidgets;

	// 标题
	contentWidgets.push_back(
		text(formula->name)
		->themeColor(QColor(32, 38, 46), QColor(240, 245, 250))
		->fontSize(20)
		->fontWeight(QFont::Bold)
	);
	contentWidgets.push_back(spacer(16));

	// 统一的标题/正文样式
	auto headerText = [](const QString& t)
		{
			return text(t)
				->themeColor(QColor(60, 70, 80), QColor(180, 190, 200))
				->fontSize(16)
				->fontWeight(QFont::Bold);
		};
	auto bodyText = [](const QString& t)
		{
			return text(t)
				->themeColor(QColor(80, 90, 100), QColor(160, 170, 180))
				->fontSize(14)
				->wrap(true)
				->padding(20, 0, 0, 0);
		};

	// 抽象各段信息，按原顺序渲染
	struct Section
	{
		const char* title;
		const QString* value;
	};
	const Section sections[] = {
		{.title = "出处", .value = &formula->source},
		{.title = "组成", .value = &formula->composition},
		{.title = "用法", .value = &formula->usage},
		{.title = "功效", .value = &formula->function},
		{.title = "主治", .value = &formula->indication},
		{.title = "备注", .value = &formula->note}
	};

	// 统计非空段数量，控制末尾不加间距
	int nonEmptyCount = 0;
	for (const auto& [title, value] : sections)
	{
		if (value && !value->isEmpty()) ++nonEmptyCount;
	}

	int emitted = 0;
	for (const auto& [title, value] : sections)
	{
		if (!value || value->isEmpty()) continue;

		contentWidgets.push_back(panel({
				headerText(QString::fromUtf8(title)),
				bodyText(*value)
			})->vertical()
			->spacing(10));

		++emitted;
		if (emitted < nonEmptyCount)
		{
			contentWidgets.push_back(spacer(12));
		}
	}

	const auto content = panel(contentWidgets)
		->vertical()
		->padding(16);
	return scrollView(content);
}
