#pragma once
#include "IconLoader.h"
#include "RenderData.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "UiTreeList.h"
#include "UiFormulaDetail.h"
#include "FormulaViewModel.h"

#include <memory>
#include <qrect.h>

// 方剂视图：左侧树状列表 + 右侧详情
class UiFormulaView final : public IUiComponent, public IUiContent
{
public:
    UiFormulaView();
    ~UiFormulaView() override;
    
    // IUiContent
    void setViewportRect(const QRect& r) override;
    
    // IUiComponent
    void updateLayout(const QSize& windowSize) override;
    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
    void append(Render::FrameData& fd) const override;
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    bool tick() override;
    QRect bounds() const override { return m_viewport; }
    
    // 设置主题调色板
    void setDarkTheme(bool dark);
    
private:
    void onFormulaSelected(int index);
    
private:
    QRect m_viewport;
    std::unique_ptr<FormulaViewModel> m_formulaVm;
    std::unique_ptr<UiTreeList> m_treeList;
    std::unique_ptr<UiFormulaDetail> m_detailView;
    
    bool m_isDark{ false };
    
    // 布局比例
    float m_splitRatio{ 0.35f }; // 左侧占35%
};