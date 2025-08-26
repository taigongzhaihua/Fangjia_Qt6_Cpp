#pragma once
#include "ILayoutable.hpp"
#include "UiComponent.hpp"
#include "UiContent.hpp"

#include <qrect.h>
#include <qsize.h>

class UiContainer final : public IUiComponent, public IUiContent, public ILayoutable {
public:
    enum class Align { Start, Center, End, Stretch };

    UiContainer() = default;
    ~UiContainer() override = default;

    void setChild(IUiComponent* c) { m_child = c; }
    IUiComponent* child() const noexcept { return m_child; }

    // 统一设置两轴对齐
    void setAlignment(Align h, Align v) { m_hAlign = h; m_vAlign = v; }
    void setAlignment(Align a) { setAlignment(a, a); }

    // IUiContent
    void setViewportRect(const QRect& r) override;

    // ILayoutable
    QSize measure(const SizeConstraints& cs) override;
    void arrange(const QRect& finalRect) override;

    // IUiComponent
    void updateLayout(const QSize& windowSize) override;
    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override;
    void append(Render::FrameData& fd) const override;
    bool onMousePress(const QPoint& pos) override;
    bool onMouseMove(const QPoint& pos) override;
    bool onMouseRelease(const QPoint& pos) override;
    bool tick() override;
    QRect bounds() const override { return m_viewport; }
    void onThemeChanged(bool isDark) override;

private:
    static int clampDim(int v, int minV, int maxV) { return std::clamp(v, minV, maxV); }

    QRect placeChildRect(const QRect& area, const QSize& desired) const;

private:
    IUiComponent* m_child{ nullptr }; // 非拥有
    QRect m_viewport;

    Align m_hAlign{ Align::Stretch };
    Align m_vAlign{ Align::Stretch };

    // 资源上下文缓存（只转发给子项）
    IconLoader* m_loader{ nullptr };
    QOpenGLFunctions* m_gl{ nullptr };
    float m_dpr{ 1.0f };
};