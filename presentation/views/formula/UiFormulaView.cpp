#include "UiFormulaView.h"

#include "FormulaViewModel.h"
#include "UiTreeList.h"

#include <RenderData.hpp>

#include <RebuildHost.h>
#include <UI.h>
#include <Binding.h>

#include <algorithm>
#include <cmath>
#include <IconCache.h>
#include <ILayoutable.hpp>
#include <Layouts.h>
#include <memory>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qfont.h>
#include <qlogging.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <qstring.h>
#include <UiComponent.hpp>
#include <UiContent.hpp>
#include <Widget.h>

using namespace UI;

// ====== UiFormulaView 实现（简化版本使用声明式Grid） ======
UiFormulaView::UiFormulaView()
{
	qDebug() << "[UiFormulaView] ctor (declarative Grid-based)";

	// 1) VM + 左树
	m_vm = std::make_unique<FormulaViewModel>();
	m_tree = std::make_unique<UiTreeList>();
	
	// 2) 使用函数式绑定替代适配器类
	m_tree->setModelFns({
		.rootIndices = [this]() -> QVector<int> {
			QVector<int> roots;
			if (!m_vm) return roots;
			const auto& nodes = m_vm->nodes();
			for (int i = 0; i < nodes.size(); ++i) {
				if (nodes[i].parentIndex == -1) roots.push_back(i);
			}
			return roots;
		},
		.childIndices = [this](int nodeId) -> QVector<int> {
			return m_vm ? m_vm->childIndices(nodeId) : QVector<int>{};
		},
		.nodeInfo = [this](int nodeId) -> UiTreeList::NodeInfo {
			UiTreeList::NodeInfo info;
			if (!m_vm) return info;
			const auto& nodes = m_vm->nodes();
			if (nodeId < 0 || nodeId >= nodes.size()) return info;
			info.label = nodes[nodeId].label;
			info.level = nodes[nodeId].level;
			info.expanded = nodes[nodeId].expanded;
			return info;
		},
		.selectedId = [this]() -> int {
			return m_vm ? m_vm->selectedIndex() : -1;
		},
		.setSelectedId = [this](int nodeId) {
			if (m_vm) m_vm->setSelectedIndex(nodeId);
		},
		.setExpanded = [this](int nodeId, bool on) {
			if (m_vm) m_vm->setExpanded(nodeId, on);
		}
	});

	// 3) 右侧详情：声明式 BindingHost（保持原有逻辑）
	m_detailBindingHost = bindingHost([this]() -> WidgetPtr
		{
			const auto* detail = m_vm->selectedFormula();

			const QColor cardBg = m_isDark ? QColor(32, 38, 46, 0) : QColor(255, 255, 255, 0);
			const QColor title = m_isDark ? QColor(250, 252, 255) : QColor(20, 25, 30);
			const QColor label = m_isDark ? QColor(100, 160, 220) : QColor(60, 120, 180);
			const QColor body = m_isDark ? QColor(255, 255, 255, 230) : QColor(50, 55, 60, 230);

			WidgetPtr bodyWidget;
			if (!detail)
			{
				bodyWidget = container(
					text("请从左侧列表选择一个方剂")->fontSize(14)->align(Qt::AlignCenter)
				)->alignment(Alignment::Center);
			}
			else
			{
				auto section = [&](const QString& head, const QString& content) -> WidgetPtr
					{
						if (content.isEmpty()) return container();
						return
							panel({
								text(head + "：")
								->fontSize(13)
								->fontWeight(QFont::DemiBold)
								->color(label)
								->align(Qt::AlignTop),
								container(
									text(content)
									->fontSize(14)
									->color(body)
									->wrap(true)
								)->padding(20, 0, 0, 0)
								})->vertical()
							->spacing(12)
							->crossAxisAlignment(Alignment::Stretch);
					};

				bodyWidget = panel({
						text(detail->name)->fontSize(22)
										  ->fontWeight(QFont::Bold)
										  ->color(title),
						spacer(8),
						section("出处", detail->source),
						section("组成", detail->composition),
						section("用法", detail->usage),
						section("功效", detail->function),
						section("主治", detail->indication),
						section("备注", detail->note)
					})->vertical()
					->spacing(16)
					->padding(20, 30, 20, 10);
			}

			auto root =
				container(bodyWidget)->alignment(Alignment::Stretch)
				->background(cardBg, 0.0f);

			// 使用声明式 ScrollView 包装内容
			return scrollView(root);
		})
		->connect([this](UI::RebuildHost* host) {
			// 使用声明式绑定替代手动信号连接
			observe(m_vm.get(), &FormulaViewModel::selectedChanged, [this, host](int) {
				if (m_tree) m_tree->reloadData();
				host->requestRebuild();
			});
			observe(m_vm.get(), &FormulaViewModel::dataChanged, [this, host]() {
				if (m_tree) m_tree->reloadData();
				host->requestRebuild();
			});
			observe(m_vm.get(), &FormulaViewModel::nodeExpandChanged, [this](int, bool) {
				if (m_tree) m_tree->reloadData();
			});
		});

	// 4) 加载示例数据
	m_vm->loadSampleData();

	// 5) 初始调色 + 构建布局
	applyPalettes();
	buildUI();
}

UiFormulaView::~UiFormulaView() = default;

void UiFormulaView::buildUI()
{
	// 分割条颜色随主题
	const QColor splitClr = m_isDark ? QColor(255, 255, 255, 30) : QColor(0, 0, 0, 25);
	
	// 使用声明式Grid布局：35% tree, 1px splitter, 65% detail
	auto gridWidget = grid()
		->columns({
			Grid::Track::Star(0.35f),  // 35% for tree
			Grid::Track::Px(1),        // 1px for splitter
			Grid::Track::Star(0.65f)   // 65% for detail
		})
		->rows({ Grid::Track::Star(1.0f) })  // Single row stretches
		->colSpacing(0)
		->rowSpacing(0)
		->add(wrap(m_tree.get()), 0, 0, 1, 1, Grid::CellAlign::Stretch, Grid::CellAlign::Stretch)
		->add(coloredBox(splitClr), 0, 1, 1, 1, Grid::CellAlign::Stretch, Grid::CellAlign::Stretch)
		->add(m_detailBindingHost, 0, 2, 1, 1, Grid::CellAlign::Stretch, Grid::CellAlign::Stretch);

	m_mainUI = gridWidget->build();
}

void UiFormulaView::setDarkTheme(const bool dark)
{
	if (m_isDark == dark) return;
	m_isDark = dark;
	applyPalettes();
	// 重建UI以应用新主题（BindingHost 会自动处理详情重建）
	buildUI();
	onThemeChanged(m_isDark);
}

void UiFormulaView::applyPalettes() const
{
	if (!m_tree) return;
	if (m_isDark)
	{
		m_tree->setPalette(UiTreeList::Palette{
			.bg = QColor(28, 34, 42, 0),
			.itemHover = QColor(255, 255, 255, 18),
			.itemPressed = QColor(255, 255, 255, 30),
			.itemSelected = QColor(255, 255, 255, 36),
			.expandIcon = QColor(180, 185, 190, 200),
			.textPrimary = QColor(240, 245, 250, 255),
			.textSecondary = QColor(180, 190, 200, 220),
			.separator = QColor(255, 255, 255, 20),
			.indicator = QColor(0, 122, 255, 220)
			});
	}
	else
	{
		m_tree->setPalette(UiTreeList::Palette{
			.bg = QColor(255, 255, 255, 0),
			.itemHover = QColor(0, 0, 0, 14),
			.itemPressed = QColor(0, 0, 0, 26),
			.itemSelected = QColor(0, 0, 0, 32),
			.expandIcon = QColor(100, 105, 110, 200),
			.textPrimary = QColor(32, 38, 46, 255),
			.textSecondary = QColor(100, 110, 120, 200),
			.separator = QColor(0, 0, 0, 25),
			.indicator = QColor(0, 102, 204, 220)
			});
	}
}

void UiFormulaView::onThemeChanged(const bool isDark)
{
	m_isDark = isDark;
	applyPalettes();
	// 重建UI以应用新主题（BindingHost 会自动处理详情重建）
	buildUI();

	// 传递主题变化给主UI组件
	if (m_mainUI) m_mainUI->onThemeChanged(isDark);
}

// IUiComponent interface implementation - delegate to main UI
void UiFormulaView::updateLayout(const QSize& windowSize)
{
	if (m_mainUI) m_mainUI->updateLayout(windowSize);
}

void UiFormulaView::updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio)
{
	if (m_mainUI) m_mainUI->updateResourceContext(cache, gl, devicePixelRatio);
}

void UiFormulaView::append(Render::FrameData& fd) const
{
	if (m_mainUI) m_mainUI->append(fd);
}

bool UiFormulaView::onMousePress(const QPoint& pos)
{
	return m_mainUI ? m_mainUI->onMousePress(pos) : false;
}

bool UiFormulaView::onMouseMove(const QPoint& pos)
{
	return m_mainUI ? m_mainUI->onMouseMove(pos) : false;
}

bool UiFormulaView::onMouseRelease(const QPoint& pos)
{
	return m_mainUI ? m_mainUI->onMouseRelease(pos) : false;
}

bool UiFormulaView::onWheel(const QPoint& pos, const QPoint& angleDelta)
{
	return m_mainUI ? m_mainUI->onWheel(pos, angleDelta) : false;
}

bool UiFormulaView::tick()
{
	return m_mainUI ? m_mainUI->tick() : false;
}

QRect UiFormulaView::bounds() const
{
	return m_mainUI ? m_mainUI->bounds() : QRect();
}