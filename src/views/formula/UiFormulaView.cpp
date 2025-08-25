#include "UiFormulaView.h"

#include "FormulaViewModel.h"
#include "UiTreeList.h"

#include <IconLoader.h>
#include <RenderData.hpp>

#include <UI.h>
#include <RebuildHost.h>

#include <memory>
#include <qcolor.h>
#include <qcontainerfwd.h>
#include <qlogging.h>
#include <qmargins.h>
#include <qobject.h>
#include <qpoint.h>
#include <qstring.h>

using namespace UI;

// ===== VM -> UiTreeList::Model 适配器 =====
class UiFormulaView::VmTreeAdapter : public UiTreeList::Model {
public:
    explicit VmTreeAdapter(FormulaViewModel* vm) : m_vm(vm) {}

    QVector<int> rootIndices() const override {
        QVector<int> roots;
        if (!m_vm) return roots;
        const auto& nodes = m_vm->nodes();
        for (int i = 0; i < nodes.size(); ++i) {
            if (nodes[i].parentIndex == -1) roots.push_back(i);
        }
        return roots;
    }

    QVector<int> childIndices(int nodeId) const override {
        return m_vm ? m_vm->childIndices(nodeId) : QVector<int>{};
    }

    UiTreeList::NodeInfo nodeInfo(int nodeId) const override {
        UiTreeList::NodeInfo info;
        if (!m_vm) return info;
        const auto& nodes = m_vm->nodes();
        if (nodeId < 0 || nodeId >= nodes.size()) return info;
        info.label = nodes[nodeId].label;
        info.level = nodes[nodeId].level;
        info.expanded = nodes[nodeId].expanded;
        return info;
    }

    int selectedId() const override {
        return m_vm ? m_vm->selectedIndex() : -1;
    }

    void setSelectedId(int nodeId) override {
        if (m_vm) m_vm->setSelectedIndex(nodeId);
    }

    void setExpanded(int nodeId, bool on) override {
        if (m_vm) m_vm->setExpanded(nodeId, on);
    }

private:
    FormulaViewModel* m_vm{ nullptr };
};

// ===== UiFormulaView 实现（声明式重写） =====
UiFormulaView::UiFormulaView() {
    qDebug() << "[UiFormulaView] ctor (declarative)";

    // 1) 创建 VM + 左侧树
    m_vm = std::make_unique<FormulaViewModel>();
    m_tree = std::make_unique<UiTreeList>();
    m_adapter = std::make_unique<VmTreeAdapter>(m_vm.get());
    m_tree->setModel(m_adapter.get());

    // 2) 右侧详情：使用 RebuildHost 动态重建
    m_detailHost = std::make_unique<UI::RebuildHost>();
    m_detailHost->setBuilder([this]() -> std::unique_ptr<IUiComponent> {
        // 根据是否选中构建右侧内容
        const auto* detail = m_vm->selectedFormula();

        // 主题色
        const QColor cardBg = m_isDark ? QColor(32, 38, 46, 0) : QColor(255, 255, 255, 0);
        const QColor title = m_isDark ? QColor(250, 252, 255) : QColor(20, 25, 30);
        const QColor label = m_isDark ? QColor(100, 160, 220) : QColor(60, 120, 180);
        const QColor body = m_isDark ? QColor(255, 255, 255, 230) : QColor(50, 55, 60, 230);

        WidgetPtr bodyWidget;
        if (!detail) {
            bodyWidget = container(
                text("请从左侧列表选择一个方剂")->fontSize(14)->color(m_isDark ? QColor(220, 230, 240, 180) : QColor(80, 90, 100, 180))
            )->alignment(Alignment::Center)->padding(24);
        }
        else {
            auto section = [&](const QString& head, const QString& content) -> WidgetPtr {
                if (content.isEmpty()) return container(); // 空则不渲染
                return column({
                    text(head + "：")->fontSize(13)->fontWeight(QFont::DemiBold)->color(label),
                    container(
                        text(content)->fontSize(14)->color(body)->wrap(true)
                    )->padding(0, 6, 0, 12)
                    });
                };

            bodyWidget = container(
                column({
                    text(detail->name)->fontSize(22)->fontWeight(QFont::Bold)->color(title),
                    container()->height(16),

                    section("出处",      detail->source),
                    section("组成",      detail->composition),
                    section("用法",      detail->usage),
                    section("功效",      detail->function),
                    section("主治",      detail->indication),
                    section("备注",      detail->note)
                    })->spacing(2)
            )->padding(24);
        }

        auto root = container(
            bodyWidget
        )->background(cardBg, 0.0f);

        return root->build();
        });

    // 3) 监听 VM 变化 => 刷新树/重建详情
    QObject::connect(m_vm.get(), &FormulaViewModel::selectedChanged, [this](int) {
        if (m_tree) m_tree->reloadData();
        if (m_detailHost) m_detailHost->requestRebuild();
        });
    QObject::connect(m_vm.get(), &FormulaViewModel::dataChanged, [this]() {
        if (m_tree) m_tree->reloadData();
        if (m_detailHost) m_detailHost->requestRebuild();
        });
    QObject::connect(m_vm.get(), &FormulaViewModel::nodeExpandChanged, [this](int, bool) {
        if (m_tree) m_tree->reloadData();
        });

    // 4) 加载示例数据
    m_vm->loadSampleData();

    // 5) 初始主题/调色并构建根
    applyPalettes();
    rebuildRoot();
}

UiFormulaView::~UiFormulaView() = default;

void UiFormulaView::setDarkTheme(bool dark) {
    if (m_isDark == dark) return;
    m_isDark = dark;
    applyPalettes();
    if (m_detailHost) m_detailHost->requestRebuild();
    if (m_root) m_root->onThemeChanged(m_isDark);
}

void UiFormulaView::applyPalettes() {
    if (!m_tree) return;
    if (m_isDark) {
        m_tree->setPalette(UiTreeList::Palette{
            .bg = QColor(28, 34, 42, 0),
            .itemHover = QColor(255, 255, 255, 15),
            .itemSelected = QColor(0, 122, 255, 35),
            .expandIcon = QColor(180, 185, 190, 200),
            .textPrimary = QColor(240, 245, 250, 255),
            .textSecondary = QColor(180, 190, 200, 220),
            .separator = QColor(255, 255, 255, 20)
            });
    }
    else {
        m_tree->setPalette(UiTreeList::Palette{
            .bg = QColor(255, 255, 255, 0),
            .itemHover = QColor(0, 0, 0, 10),
            .itemSelected = QColor(0, 122, 255, 25),
            .expandIcon = QColor(100, 105, 110, 200),
            .textPrimary = QColor(32, 38, 46, 255),
            .textSecondary = QColor(100, 110, 120, 200),
            .separator = QColor(0, 0, 0, 25)
            });
    }
}

void UiFormulaView::rebuildRoot() {
    // 左树
    auto left = wrap(m_tree.get());

    // 中间分割线（视觉 1px）
    const QColor splitClr = m_isDark ? QColor(255, 255, 255, 30) : QColor(0, 0, 0, 25);
    auto splitter = container()->width(1)->background(splitClr);

    // 右详情
    auto detail = wrap(m_detailHost.get());

    auto rootRow = row({
        expanded(left, m_leftRatio),
        splitter,
        expanded(detail, 1.0f - m_leftRatio)
        })->spacing(0);

    m_root = rootRow->build();

    // 同步上下文/视口/主题
    if (m_loader && m_gl) m_root->updateResourceContext(*m_loader, m_gl, m_dpr);
    if (!m_lastWinSize.isEmpty()) m_root->updateLayout(m_lastWinSize);
    if (m_viewport.isValid()) {
        if (auto* c = dynamic_cast<IUiContent*>(m_root.get())) c->setViewportRect(m_viewport);
    }
    m_root->onThemeChanged(m_isDark);
}

// ========== IUiContent ==========
void UiFormulaView::setViewportRect(const QRect& r) {
    m_viewport = r;
    if (m_root) {
        if (auto* c = dynamic_cast<IUiContent*>(m_root.get())) c->setViewportRect(r);
    }
}

// ========== IUiComponent ==========
void UiFormulaView::updateLayout(const QSize& windowSize) {
    m_lastWinSize = windowSize;
    if (m_root) m_root->updateLayout(windowSize);
}

void UiFormulaView::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) {
    m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
    if (m_root) m_root->updateResourceContext(loader, gl, devicePixelRatio);
}

void UiFormulaView::append(Render::FrameData& fd) const {
    if (m_root) m_root->append(fd);
}

bool UiFormulaView::onMousePress(const QPoint& pos) {
    return m_root ? m_root->onMousePress(pos) : false;
}

bool UiFormulaView::onMouseMove(const QPoint& pos) {
    return m_root ? m_root->onMouseMove(pos) : false;
}

bool UiFormulaView::onMouseRelease(const QPoint& pos) {
    return m_root ? m_root->onMouseRelease(pos) : false;
}

bool UiFormulaView::tick() {
    return m_root ? m_root->tick() : false;
}

QRect UiFormulaView::bounds() const {
    return m_root ? m_root->bounds() : m_viewport;
}

void UiFormulaView::onThemeChanged(bool isDark) {
    setDarkTheme(isDark);
}