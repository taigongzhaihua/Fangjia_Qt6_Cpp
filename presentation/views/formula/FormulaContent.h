#pragma once

#include "UiComponent.hpp"
#include "IconCache.h"
#include <memory>
#include <qopenglfunctions.h>
#include <qrect.h>
#include <qsize.h>

class FormulaViewModel;
class UiTreeList;

namespace UI {
    class Widget;
    using WidgetPtr = std::shared_ptr<Widget>;
}

namespace Render { struct FrameData; }

// FormulaContent: thin runtime container that owns the tree component and builds a declarative UI
class FormulaContent final : public IUiComponent {
public:
    FormulaContent();
    ~FormulaContent() override;

    // Theme API (forwarded from DataPage)
    void setDarkTheme(bool dark);

    // IUiComponent
    void updateLayout(const QSize& windowSize) override;
    void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override;
    void append(Render::FrameData& fd) const override;
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    bool onWheel(const QPoint& pos, const QPoint& angleDelta) override;
    bool tick() override;
    QRect bounds() const override;
    void onThemeChanged(bool isDark) override;

private:
    void buildUI();
    void applyPalettes() const;

private:
    std::unique_ptr<FormulaViewModel> m_vm;
    std::unique_ptr<UiTreeList> m_tree;

    // Right detail (declarative binding host)
    UI::WidgetPtr m_detailBindingHost;

    // Root UI built from Grid
    std::unique_ptr<IUiComponent> m_mainUI;

    bool m_isDark{ false };
};