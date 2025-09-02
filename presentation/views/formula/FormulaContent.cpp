#include "FormulaContent.h"

#include "FormulaViewModel.h"
#include "UiTreeList.h"

#include "UI.h"
#include "Binding.h"
#include "BasicWidgets.h"
#include "Layouts.h"
#include "ScrollView.h"
#include "ComponentWrapper.h"

#include <qcolor.h>
#include <qlogging.h>

using namespace UI;

FormulaContent::FormulaContent(FormulaViewModel* vm)
    : m_viewModel(vm)
{
    if (!m_viewModel) {
        qWarning() << "[FormulaContent] Warning: FormulaViewModel is null";
    }
}

std::unique_ptr<IUiComponent> FormulaContent::build() const
{
    if (!m_viewModel) {
        qWarning() << "[FormulaContent] Cannot build UI without valid FormulaViewModel";
        return nullptr;
    }

    // 创建树形列表
    auto treeList = createTreeList();
    auto treeWidget = wrap(treeList.release());

    // 创建分隔线（1px半透明灰色）
    auto splitter = coloredBox(QColor(128, 128, 128, 100));

    // 创建详情面板
    auto detailsPanel = createDetailsPanel();

    // 使用Grid布局：35% + 1px + 65%，单行
    return grid()
        ->columns({0.35_fr, 1_px, 0.65_fr})
        ->rows({1_fr})
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
        .rootIndices = [this]() -> QVector<int> {
            QVector<int> roots;
            if (!m_viewModel) return roots;
            const auto& nodes = m_viewModel->nodes();
            for (int i = 0; i < nodes.size(); ++i) {
                if (nodes[i].parentIndex == -1) {
                    roots.push_back(i);
                }
            }
            return roots;
        },
        .childIndices = [this](int nodeId) -> QVector<int> {
            return m_viewModel ? m_viewModel->childIndices(nodeId) : QVector<int>{};
        },
        .nodeInfo = [this](int nodeId) -> UiTreeList::NodeInfo {
            UiTreeList::NodeInfo info;
            if (!m_viewModel) return info;
            const auto& nodes = m_viewModel->nodes();
            if (nodeId < 0 || nodeId >= nodes.size()) return info;
            info.label = nodes[nodeId].label;
            info.level = nodes[nodeId].level;
            info.expanded = nodes[nodeId].expanded;
            return info;
        },
        .selectedId = [this]() -> int {
            return m_viewModel ? m_viewModel->selectedIndex() : -1;
        },
        .setSelectedId = [this](int nodeId) {
            if (m_viewModel) {
                m_viewModel->setSelectedIndex(nodeId);
            }
        },
        .setExpanded = [this](int nodeId, bool expanded) {
            if (m_viewModel) {
                m_viewModel->setExpanded(nodeId, expanded);
            }
        }
    });

    // 设置默认调色板（主题切换后续可以优化）
    UiTreeList::Palette palette{
        .bg = QColor(255, 255, 255, 245),
        .itemHover = QColor(0, 0, 0, 14),
        .itemPressed = QColor(0, 0, 0, 26),
        .itemSelected = QColor(0, 122, 255, 32),
        .expandIcon = QColor(100, 100, 100, 200),
        .textPrimary = QColor(32, 38, 46, 255),
        .textSecondary = QColor(100, 110, 120, 200),
        .separator = QColor(0, 0, 0, 20),
        .indicator = QColor(0, 102, 204, 220)
    };
    tree->setPalette(palette);

    return tree;
}

UI::WidgetPtr FormulaContent::createDetailsPanel() const
{
    // 使用BindingHost创建响应式详情面板
    return bindingHost([this]() -> UI::WidgetPtr {
        return buildDetailsContent();
    })->connect([this](UI::RebuildHost* host) {
        // 监听ViewModel信号，触发UI重建
        if (!m_viewModel || !host) return;
        
        observe(m_viewModel, &FormulaViewModel::selectedChanged, [host](int) {
            host->requestRebuild();
        });
        
        observe(m_viewModel, &FormulaViewModel::dataChanged, [host]() {
            host->requestRebuild();
        });
        
        observe(m_viewModel, &FormulaViewModel::nodeExpandChanged, [host](int, bool) {
            // 节点展开不影响详情面板，但为了完整性保留监听
            // host->requestRebuild();
        });
    });
}

UI::WidgetPtr FormulaContent::buildDetailsContent() const
{
    if (!m_viewModel) {
        return panel({
            text("未找到数据模型")
                ->themeColor(QColor(100, 100, 100), QColor(200, 200, 200))
                ->fontSize(14)
        })->vertical()->padding(16);
    }

    const auto* formula = m_viewModel->selectedFormula();
    if (!formula) {
        return scrollView(
            panel({
                text("请选择一个方剂查看详情")
                    ->themeColor(QColor(100, 100, 100), QColor(200, 200, 200))
                    ->fontSize(14)
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
    
    // 出处
    if (!formula->source.isEmpty()) {
        contentWidgets.push_back(panel({
            text("出处")
                ->themeColor(QColor(60, 70, 80), QColor(180, 190, 200))
                ->fontSize(12)
                ->fontWeight(QFont::Bold),
            spacer(4),
            text(formula->source)
                ->themeColor(QColor(80, 90, 100), QColor(160, 170, 180))
                ->fontSize(14)
                ->wrap(true)
        })->vertical());
        contentWidgets.push_back(spacer(12));
    }
    
    // 组成
    if (!formula->composition.isEmpty()) {
        contentWidgets.push_back(panel({
            text("组成")
                ->themeColor(QColor(60, 70, 80), QColor(180, 190, 200))
                ->fontSize(12)
                ->fontWeight(QFont::Bold),
            spacer(4),
            text(formula->composition)
                ->themeColor(QColor(80, 90, 100), QColor(160, 170, 180))
                ->fontSize(14)
                ->wrap(true)
        })->vertical());
        contentWidgets.push_back(spacer(12));
    }
    
    // 用法
    if (!formula->usage.isEmpty()) {
        contentWidgets.push_back(panel({
            text("用法")
                ->themeColor(QColor(60, 70, 80), QColor(180, 190, 200))
                ->fontSize(12)
                ->fontWeight(QFont::Bold),
            spacer(4),
            text(formula->usage)
                ->themeColor(QColor(80, 90, 100), QColor(160, 170, 180))
                ->fontSize(14)
                ->wrap(true)
        })->vertical());
        contentWidgets.push_back(spacer(12));
    }
    
    // 功效
    if (!formula->function.isEmpty()) {
        contentWidgets.push_back(panel({
            text("功效")
                ->themeColor(QColor(60, 70, 80), QColor(180, 190, 200))
                ->fontSize(12)
                ->fontWeight(QFont::Bold),
            spacer(4),
            text(formula->function)
                ->themeColor(QColor(80, 90, 100), QColor(160, 170, 180))
                ->fontSize(14)
                ->wrap(true)
        })->vertical());
        contentWidgets.push_back(spacer(12));
    }
    
    // 主治
    if (!formula->indication.isEmpty()) {
        contentWidgets.push_back(panel({
            text("主治")
                ->themeColor(QColor(60, 70, 80), QColor(180, 190, 200))
                ->fontSize(12)
                ->fontWeight(QFont::Bold),
            spacer(4),
            text(formula->indication)
                ->themeColor(QColor(80, 90, 100), QColor(160, 170, 180))
                ->fontSize(14)
                ->wrap(true)
        })->vertical());
        contentWidgets.push_back(spacer(12));
    }
    
    // 备注
    if (!formula->note.isEmpty()) {
        contentWidgets.push_back(panel({
            text("备注")
                ->themeColor(QColor(60, 70, 80), QColor(180, 190, 200))
                ->fontSize(12)
                ->fontWeight(QFont::Bold),
            spacer(4),
            text(formula->note)
                ->themeColor(QColor(80, 90, 100), QColor(160, 170, 180))
                ->fontSize(14)
                ->wrap(true)
        })->vertical());
    }

    auto content = panel(contentWidgets)->vertical()->padding(16);

    // 包装在ScrollView中
    return scrollView(content);
}