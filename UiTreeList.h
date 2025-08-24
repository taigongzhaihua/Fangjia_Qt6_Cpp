#pragma once
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "FormulaViewModel.h"

#include <qcolor.h>
#include <qelapsedtimer.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <vector>

class UiTreeList final : public IUiComponent
{
public:
    struct Palette {
        QColor bg;              // 背景色
        QColor itemHover;       // 悬停背景
        QColor itemSelected;    // 选中背景
        QColor expandIcon;      // 展开/折叠图标颜色
        QColor textPrimary;     // 主文字颜色
        QColor textSecondary;   // 次级文字颜色
        QColor separator;       // 分隔线颜色
    };

    UiTreeList();
    ~UiTreeList() override = default;

    // 设置数据模型
    void setViewModel(FormulaViewModel* vm);
    
    // 外观配置
    void setPalette(const Palette& p) { m_pal = p; }
    void setViewport(const QRect& r) { m_viewport = r; }
    void setItemHeight(int h) { m_itemHeight = std::max(24, h); }
    void setIndentWidth(int w) { m_indentWidth = std::max(16, w); }
    
    // 滚动支持
    void setScrollOffset(int y) { m_scrollY = y; }
    [[nodiscard]] int scrollOffset() const noexcept { return m_scrollY; }
    [[nodiscard]] int contentHeight() const;
    
    // IUiComponent
    void updateLayout(const QSize& windowSize) override;
    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
    void append(Render::FrameData& fd) const override;
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    bool tick() override;
    QRect bounds() const override { return m_viewport; }

private:
    // 计算可见的节点列表（考虑展开状态）
    struct VisibleNode {
        int index;      // 在 ViewModel 中的索引
        int depth;      // 显示深度（用于缩进）
        QRect rect;     // 显示矩形
    };
    
    void updateVisibleNodes();
    [[nodiscard]] QRect nodeRect(int visibleIdx) const;
    [[nodiscard]] QRect expandIconRect(const QRect& nodeRect, int depth) const;
    
private:
    FormulaViewModel* m_vm{ nullptr };
    QRect m_viewport;
    Palette m_pal{
        .bg = QColor(255,255,255,245),
        .itemHover = QColor(0,0,0,8),
        .itemSelected = QColor(0,122,255,20),
        .expandIcon = QColor(100,100,100,200),
        .textPrimary = QColor(32,38,46,255),
        .textSecondary = QColor(100,110,120,200),
        .separator = QColor(0,0,0,20)
    };
    
    int m_itemHeight{ 36 };
    int m_indentWidth{ 20 };
    int m_scrollY{ 0 };
    
    std::vector<VisibleNode> m_visibleNodes;
    int m_hover{ -1 };
    int m_pressed{ -1 };
    
    // 资源上下文
    IconLoader* m_loader{ nullptr };
    QOpenGLFunctions* m_gl{ nullptr };
    float m_dpr{ 1.0f };
    
    // 展开/折叠动画
    struct NodeAnim {
        bool active{ false };
        int nodeIdx{ -1 };
        float progress{ 0.0f };
        bool expanding{ true };
    };
    NodeAnim m_expandAnim;
    QElapsedTimer m_animClock;
};