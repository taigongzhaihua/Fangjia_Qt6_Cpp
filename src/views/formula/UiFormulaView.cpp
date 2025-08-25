#include "UiFormulaDetail.h"
#include "UiFormulaView.h"
#include <algorithm>
#include <FormulaViewModel.h>
#include <memory>
#include <qcolor.h>
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

	// 初始化布局
	initializeLayout();

	// 设置数据模型
	m_treeList->setViewModel(m_formulaVm.get());
	m_formulaVm->loadSampleData();

	qDebug() << "UiFormulaView: loaded sample data, node count:\n"
		<< m_formulaVm->nodeCount();

	// 连接信号
	QObject::connect(m_formulaVm.get(), &FormulaViewModel::selectedChanged,
		[this](int index) {
			qDebug() << "Formula selected:" << index << "\n";
			onFormulaSelected(index);
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
	if (m_isDark) {
		// 深色主题配色

		// 自身背景（可选，如果需要整体背景）
		setBackgroundColor(QColor(18, 22, 28, 0));

		// 树列表配色
		if (m_treeList) {
			m_treeList->setPalette(UiTreeList::Palette{
				.bg = QColor(28, 34, 42, 0),              // 深色背景
				.itemHover = QColor(255, 255, 255, 15),     // 白色半透明悬停
				.itemSelected = QColor(0, 122, 255, 35),    // 蓝色选中
				.expandIcon = QColor(180, 185, 190, 200),   // 浅灰展开图标
				.textPrimary = QColor(240, 245, 250, 255),  // 主文字白色
				.textSecondary = QColor(180, 190, 200, 220),// 次级文字灰色
				.separator = QColor(255, 255, 255, 20)      // 分隔线
				});
		}

		// 详情视图配色
		if (m_detailView) {
			m_detailView->setPalette(UiFormulaDetail::Palette{
				.bg = QColor(32, 38, 46, 0),              // 深色背景
				.titleColor = QColor(250, 252, 255, 255),   // 标题白色
				.labelColor = QColor(100, 160, 220, 255),   // 标签蓝色
				.textColor = QColor(255, 255, 255, 230),    // 正文浅灰
				.borderColor = QColor(255, 255, 255, 25)    // 边框
				});
		}

		// 分割条颜色
		if (m_splitter) {
			m_splitter->setBackgroundColor(QColor(255, 255, 255, 30));
		}

	}
	else {
		// 浅色主题配色

		// 自身背景
		setBackgroundColor(QColor(248, 249, 250, 0));

		// 树列表配色
		if (m_treeList) {
			m_treeList->setPalette(UiTreeList::Palette{
				.bg = QColor(255, 255, 255, 0),           // 白色背景
				.itemHover = QColor(0, 0, 0, 10),           // 黑色半透明悬停
				.itemSelected = QColor(0, 122, 255, 25),    // 蓝色选中
				.expandIcon = QColor(100, 105, 110, 200),   // 深灰展开图标
				.textPrimary = QColor(32, 38, 46, 255),     // 主文字深色
				.textSecondary = QColor(100, 110, 120, 200),// 次级文字灰色
				.separator = QColor(0, 0, 0, 25)            // 分隔线
				});
		}

		// 详情视图配色
		if (m_detailView) {
			m_detailView->setPalette(UiFormulaDetail::Palette{
				.bg = QColor(255, 255, 255, 0),           // 白色背景
				.titleColor = QColor(20, 25, 30, 255),      // 标题深色
				.labelColor = QColor(60, 120, 180, 255),    // 标签蓝色
				.textColor = QColor(50, 55, 60, 230),       // 正文深灰
				.borderColor = QColor(0, 0, 0, 35)          // 边框
				});
		}

		// 分割条颜色
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
	// 检查是否点击分割条
	if (m_splitter->bounds().contains(pos)) {
		m_draggingSplitter = true;
		m_dragStartX = pos.x();
		return true;
	}

	// 否则调用基类处理
	return UiBoxLayout::onMousePress(pos);
}

bool UiFormulaView::onMouseMove(const QPoint& pos)
{
	if (m_draggingSplitter) {
		// 处理分割条拖动
		const int deltaX = pos.x() - m_dragStartX;
		const float deltaRatio = static_cast<float>(deltaX) / bounds().width();
		m_splitRatio = std::clamp(m_splitRatio + deltaRatio, 0.2f, 0.8f);

		// 更新子控件权重
		if (childCount() >= 3) {
			// 假设第0个是树，第2个是详情
			// 这里需要 UiBoxLayout 提供修改权重的接口
			// setChildWeight(0, m_splitRatio);
			// setChildWeight(2, 1.0f - m_splitRatio);
		}

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