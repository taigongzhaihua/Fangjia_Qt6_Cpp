#include "UiFormulaDetail.h"
#include "UiFormulaView.h"
#include <algorithm>
#include <FormulaViewModel.h>
#include <memory>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qlogging.h>
#include <qmargins.h>
#include <qobject.h>
#include <qpoint.h>
#include <UiBoxLayout.h>
#include <UiTreeList.h>

UiFormulaView::UiFormulaView()
	: UiBoxLayout(Direction::Horizontal)  // 水平布局
{
	qDebug() << "UiFormulaView constructor start \n";

	// 创建子组件
	m_formulaVm = std::make_unique<FormulaViewModel>();
	m_treeList = std::make_unique<UiTreeList>();
	m_detailView = std::make_unique<UiFormulaDetail>();

	// 创建分割条
	m_splitter = std::make_unique<UiBoxLayout>(Direction::Vertical);
	m_splitter->setBackgroundColor(QColor(200, 200, 200, 255));

	// VM 适配器
	m_treeAdapter = std::make_unique<VmTreeAdapter>(m_formulaVm.get());
	m_treeList->setModel(m_treeAdapter.get());

	// 初始化布局
	initializeLayout();

	// 加载数据
	m_formulaVm->loadSampleData();

	qDebug() << "UiFormulaView: loaded sample data, node count:\n"
		<< m_formulaVm->nodeCount();

	// 连接 VM 信号 -> 由页面来驱动树刷新与详情更新
	QObject::connect(m_formulaVm.get(), &FormulaViewModel::selectedChanged,
		[this](int index) {
			qDebug() << "Formula selected:" << index << "\n";
			onFormulaSelected(index);
			if (m_treeList) m_treeList->reloadData();
		});
	QObject::connect(m_formulaVm.get(), &FormulaViewModel::dataChanged,
		[this]() {
			if (m_treeList) m_treeList->reloadData();
		});
	QObject::connect(m_formulaVm.get(), &FormulaViewModel::nodeExpandChanged,
		[this](int, bool) {
			if (m_treeList) m_treeList->reloadData();
		});

	qDebug() << "UiFormulaView constructor end \n";
}

UiFormulaView::~UiFormulaView() = default;

void UiFormulaView::initializeLayout()
{
	qDebug() << "UiFormulaView::initializeLayout\n";

	// 设置布局参数
	setSpacing(1);
	setMargins(QMargins(0, 0, 0, 0));

	// 添加子控件
	addChild(m_treeList.get(), 0.35f, Alignment::Stretch);
	addChild(m_splitter.get(), 0.0f, Alignment::Stretch);
	addChild(m_detailView.get(), 0.65f, Alignment::Stretch);

	qDebug() << "UiFormulaView: added" << childCount() << "children" << "\n";
}

void UiFormulaView::setDarkTheme(bool dark)
{
	m_isDark = dark;
	updatePalettes();
}

void UiFormulaView::updatePalettes()
{
	setBackgroundColor(QColor(0, 0, 0, 0));
	if (m_isDark) {
		// 深色主题配色

		if (m_treeList) {
			m_treeList->setPalette(UiTreeList::Palette{
				.bg = QColor(28, 34, 42, 0),
				.itemHover = QColor(255, 255, 255, 15),
				.itemSelected = QColor(0, 122, 255, 35),
				.expandIcon = QColor(180, 185, 190, 200),
				.textPrimary = QColor(240, 245, 250, 255),
				.textSecondary = QColor(180, 190, 200, 220),
				.separator = QColor(255, 255, 255, 20)
				});
		}

		if (m_detailView) {
			m_detailView->setPalette(UiFormulaDetail::Palette{
				.bg = QColor(32, 38, 46, 0),
				.titleColor = QColor(250, 252, 255, 255),
				.labelColor = QColor(100, 160, 220, 255),
				.textColor = QColor(255, 255, 255, 230),
				.borderColor = QColor(255, 255, 255, 25)
				});
		}

		if (m_splitter) {
			m_splitter->setBackgroundColor(QColor(255, 255, 255, 30));
		}
	}
	else {
		// 浅色主题配色

		if (m_treeList) {
			m_treeList->setPalette(UiTreeList::Palette{
				.bg = QColor(255, 255, 255, 0),
				.itemHover = QColor(0, 0, 0, 10),
				.itemSelected = QColor(0, 122, 255, 25),
				.expandIcon = QColor(100, 105, 110, 200),
				.textPrimary = QColor(32, 38, 46, 255),
				.textSecondary = QColor(100, 110, 120, 200),
				.separator = QColor(0, 0, 0, 25)
				});
		}

		if (m_detailView) {
			m_detailView->setPalette(UiFormulaDetail::Palette{
				.bg = QColor(255, 255, 255, 0),
				.titleColor = QColor(20, 25, 30, 255),
				.labelColor = QColor(60, 120, 180, 255),
				.textColor = QColor(50, 55, 60, 230),
				.borderColor = QColor(0, 0, 0, 35)
				});
		}

		if (m_splitter) {
			m_splitter->setBackgroundColor(QColor(0, 0, 0, 25));
		}
	}
}

void UiFormulaView::onFormulaSelected(int index) const
{
	if (index >= 0) {
		const auto* formula = m_formulaVm->selectedFormula();
		m_detailView->setFormula(formula);
	}
	else {
		m_detailView->setFormula(nullptr);
	}
}

bool UiFormulaView::onMousePress(const QPoint& pos)
{
	// 分割条拖动
	if (m_splitter->bounds().contains(pos)) {
		m_draggingSplitter = true;
		m_dragStartX = pos.x();
		return true;
	}
	return UiBoxLayout::onMousePress(pos);
}

bool UiFormulaView::onMouseMove(const QPoint& pos)
{
	if (m_draggingSplitter) {
		const int deltaX = pos.x() - m_dragStartX;
		const float deltaRatio = static_cast<float>(deltaX) / bounds().width();
		m_splitRatio = std::clamp(m_splitRatio + deltaRatio, 0.2f, 0.8f);
		// TODO: setChildWeight(0, m_splitRatio); setChildWeight(2, 1.0f - m_splitRatio);
		m_dragStartX = pos.x();
		return true;
	}
	return UiBoxLayout::onMouseMove(pos);
}

bool UiFormulaView::onMouseRelease(const QPoint& pos)
{
	if (m_draggingSplitter) {
		m_draggingSplitter = false;
		return true;
	}
	return UiBoxLayout::onMouseRelease(pos);
}

// ===== VmTreeAdapter =====

QVector<int> UiFormulaView::VmTreeAdapter::rootIndices() const
{
	QVector<int> roots;
	if (!m_vm) return roots;
	const auto& nodes = m_vm->nodes();
	for (int i = 0; i < nodes.size(); ++i) {
		if (nodes[i].parentIndex == -1) roots.push_back(i);
	}
	return roots;
}

QVector<int> UiFormulaView::VmTreeAdapter::childIndices(const int nodeId) const
{
	if (!m_vm) return {};
	return m_vm->childIndices(nodeId);
}

UiTreeList::NodeInfo UiFormulaView::VmTreeAdapter::nodeInfo(const int nodeId) const
{
	UiTreeList::NodeInfo info;
	if (!m_vm) return info;
	const auto& nodes = m_vm->nodes();
	if (nodeId < 0 || nodeId >= nodes.size()) return info;
	info.label = nodes[nodeId].label;
	info.level = nodes[nodeId].level;
	info.expanded = nodes[nodeId].expanded;
	return info;
}

int UiFormulaView::VmTreeAdapter::selectedId() const
{
	return m_vm ? m_vm->selectedIndex() : -1;
}

void UiFormulaView::VmTreeAdapter::setSelectedId(const int nodeId)
{
	if (m_vm) m_vm->setSelectedIndex(nodeId);
}

void UiFormulaView::VmTreeAdapter::setExpanded(const int nodeId, const bool on)
{
	if (m_vm) m_vm->setExpanded(nodeId, on);
}