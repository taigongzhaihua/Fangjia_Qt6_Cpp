#include "UiFormulaView.h"
#include <algorithm>

UiFormulaView::UiFormulaView()
{
    m_formulaVm = std::make_unique<FormulaViewModel>();
    m_treeList = std::make_unique<UiTreeList>();
    m_detailView = std::make_unique<UiFormulaDetail>();
    
    // 设置数据模型
    m_treeList->setViewModel(m_formulaVm.get());
    
    // 加载示例数据
    m_formulaVm->loadSampleData();
    
    // 监听选中变化
    QObject::connect(m_formulaVm.get(), &FormulaViewModel::selectedChanged,
                     [this](int index) { onFormulaSelected(index); });
    
    // 监听展开/折叠
    QObject::connect(m_formulaVm.get(), &FormulaViewModel::nodeExpandChanged,
                     [this](int, bool) { updateLayout(QSize()); });
}

UiFormulaView::~UiFormulaView() = default;

void UiFormulaView::setViewportRect(const QRect& r)
{
    m_viewport = r;
    updateLayout(QSize());
}

void UiFormulaView::updateLayout(const QSize& /*windowSize*/)
{
    if (!m_viewport.isValid()) return;
    
    const int splitX = m_viewport.left() + static_cast<int>(m_viewport.width() * m_splitRatio);
    const int gap = 1; // 分隔线宽度
    
    // 左侧树状列表
    QRect treeRect(
        m_viewport.left(),
        m_viewport.top(),
        splitX - m_viewport.left() - gap,
        m_viewport.height()
    );
    m_treeList->setViewport(treeRect);
    m_treeList->updateLayout(QSize());
    
    // 右侧详情
    QRect detailRect(
        splitX + gap,
        m_viewport.top(),
        m_viewport.right() - splitX - gap,
        m_viewport.height()
    );
    m_detailView->setViewport(detailRect);
    m_detailView->updateLayout(QSize());
}

void UiFormulaView::updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio)
{
    m_treeList->updateResourceContext(loader, gl, devicePixelRatio);
    m_detailView->updateResourceContext(loader, gl, devicePixelRatio);
}

void UiFormulaView::append(Render::FrameData& fd) const
{
    // 背景
    fd.roundedRects.push_back(Render::RoundedRectCmd{
        .rect = QRectF(m_viewport),
        .radiusPx = 0.0f,
        .color = m_isDark ? QColor(18,22,28,250) : QColor(248,249,250,250)
    });
    
    // 绘制子组件
    m_treeList->append(fd);
    m_detailView->append(fd);
    
    // 分隔线
    const int splitX = m_viewport.left() + static_cast<int>(m_viewport.width() * m_splitRatio);
    fd.roundedRects.push_back(Render::RoundedRectCmd{
        .rect = QRectF(splitX - 1, m_viewport.top(), 2, m_viewport.height()),
        .radiusPx = 0.0f,
        .color = m_isDark ? QColor(255,255,255,20) : QColor(0,0,0,20)
    });
}

bool UiFormulaView::onMousePress(const QPoint& pos)
{
    if (m_treeList->onMousePress(pos)) return true;
    if (m_detailView->onMousePress(pos)) return true;
    return false;
}

bool UiFormulaView::onMouseMove(const QPoint& pos)
{
    bool changed = false;
    changed = m_treeList->onMouseMove(pos) || changed;
    changed = m_detailView->onMouseMove(pos) || changed;
    return changed;
}

bool UiFormulaView::onMouseRelease(const QPoint& pos)
{
    if (m_treeList->onMouseRelease(pos)) return true;
    if (m_detailView->onMouseRelease(pos)) return true;
    return false;
}

bool UiFormulaView::tick()
{
    bool active = false;
    active = m_treeList->tick() || active;
    active = m_detailView->tick() || active;
    return active;
}

void UiFormulaView::setDarkTheme(bool dark)
{
    m_isDark = dark;
    
    // 更新子组件调色板
    if (dark) {
        m_treeList->setPalette(UiTreeList::Palette{
            .bg = QColor(28,34,42,245),
            .itemHover = QColor(255,255,255,12),
            .itemSelected = QColor(0,122,255,30),
            .expandIcon = QColor(180,180,180,200),
            .textPrimary = QColor(240,245,250,255),
            .textSecondary = QColor(180,190,200,220),
            .separator = QColor(255,255,255,15)
        });
        
        m_detailView->setPalette(UiFormulaDetail::Palette{
            .bg = QColor(32,38,46,250),
            .titleColor = QColor(250,252,255,255),
            .labelColor = QColor(100,160,220,255),
            .textColor = QColor(210,220,230,230),
            .borderColor = QColor(255,255,255,20)
        });
    } else {
        m_treeList->setPalette(UiTreeList::Palette{
            .bg = QColor(255,255,255,245),
            .itemHover = QColor(0,0,0,8),
            .itemSelected = QColor(0,122,255,20),
            .expandIcon = QColor(100,100,100,200),
            .textPrimary = QColor(32,38,46,255),
            .textSecondary = QColor(100,110,120,200),
            .separator = QColor(0,0,0,20)
        });
        
        m_detailView->setPalette(UiFormulaDetail::Palette{
            .bg = QColor(255,255,255,250),
            .titleColor = QColor(20,25,30,255),
            .labelColor = QColor(60,120,180,255),
            .textColor = QColor(50,55,60,230),
            .borderColor = QColor(0,0,0,30)
        });
    }
}

void UiFormulaView::onFormulaSelected(int index)
{
    if (index >= 0) {
        const auto* formula = m_formulaVm->selectedFormula();
        m_detailView->setFormula(formula);
    } else {
        m_detailView->setFormula(nullptr);
    }
}