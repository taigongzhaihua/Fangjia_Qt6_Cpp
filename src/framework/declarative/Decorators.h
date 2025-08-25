#pragma once
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "Widget.h"
#include <memory>

namespace UI {

// 通用装饰器：将 Widget 的 Decorations 落地到一个 IUiComponent 包裹层
class DecoratedBox : public IUiComponent, public IUiContent {
public:
    struct Props {
        QMargins padding{0,0,0,0};
        QMargins margin{0,0,0,0}; // 目前仅作为视觉外边距，不参与父布局测量
        QColor   bg{Qt::transparent};
        float    bgRadius{0.0f};
        QColor   border{Qt::transparent};
        float    borderW{0.0f};
        float    borderRadius{0.0f};
        QSize    fixedSize{-1, -1};
        bool     visible{true};
        float    opacity{1.0f};
        std::function<void()> onTap;
        std::function<void(bool)> onHover;
    };

    DecoratedBox(std::unique_ptr<IUiComponent> child, Props p)
        : m_child(std::move(child)), m_p(std::move(p)) {}

    // IUiContent
    void setViewportRect(const QRect& r) override {
        m_viewport = r;
        // 内部内容区域 = viewport - padding
        m_contentRect = m_viewport.adjusted(
            m_p.padding.left(), m_p.padding.top(),
            -m_p.padding.right(), -m_p.padding.bottom()
        );
        if (auto* c = dynamic_cast<IUiContent*>(m_child.get())) {
            c->setViewportRect(m_contentRect);
        }
    }

    // IUiComponent
    void updateLayout(const QSize& windowSize) override {
        if (m_child) m_child->updateLayout(windowSize);
    }

    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override {
        m_loader = &loader; m_gl = gl; m_dpr = std::max(0.5f, devicePixelRatio);
        if (m_child) m_child->updateResourceContext(loader, gl, devicePixelRatio);
    }

    void append(Render::FrameData& fd) const override {
        if (!m_p.visible) return;
        // 背景
        if (m_p.bg.alpha() > 0 && m_viewport.isValid()) {
            fd.roundedRects.push_back(Render::RoundedRectCmd{
                .rect = QRectF(m_viewport),
                .radiusPx = m_p.bgRadius,
                .color = withOpacity(m_p.bg, m_p.opacity)
            });
        }
        // TODO: 边框（需要单独着色/路径，当前渲染器可用两个叠矩形近似）

        // 子项
        if (m_child) m_child->append(fd);
    }

    bool onMousePress(const QPoint& pos) override {
        if (!m_p.visible || !m_viewport.contains(pos)) return false;
        if (m_child && m_child->onMousePress(pos)) return true;
        return false;
    }

    bool onMouseMove(const QPoint& pos) override {
        if (!m_p.visible) return false;
        bool handled = false;
        if (m_child) handled = m_child->onMouseMove(pos) || handled;
        if (m_p.onHover) {
            const bool hov = m_viewport.contains(pos);
            if (hov != m_hover) {
                m_hover = hov;
                m_p.onHover(m_hover);
                handled = true;
            }
        }
        return handled;
    }

    bool onMouseRelease(const QPoint& pos) override {
        if (!m_p.visible) return false;
        bool handled = false;
        if (m_child) handled = m_child->onMouseRelease(pos) || handled;
        if (m_p.onTap && m_viewport.contains(pos)) {
            m_p.onTap();
            handled = true;
        }
        return handled;
    }

    bool tick() override {
        return m_child && m_child->tick();
    }

    QRect bounds() const override {
        // 给父布局的“首选尺寸”：若设置了固定尺寸，则作为 preferred size（纵横有值才生效）
        if (m_p.fixedSize.width() > 0 || m_p.fixedSize.height() > 0) {
            return QRect(0, 0,
                std::max(0, m_p.fixedSize.width()),
                std::max(0, m_p.fixedSize.height()));
        }
        // 否则按子项 bounds + padding 粗略估算
        if (m_child) {
            const QRect cb = m_child->bounds();
            return QRect(0, 0,
                cb.width() + m_p.padding.left() + m_p.padding.right(),
                cb.height() + m_p.padding.top() + m_p.padding.bottom());
        }
        return QRect();
    }

private:
    static QColor withOpacity(QColor c, float mul) {
        const int a = std::clamp(int(std::lround(c.alphaF() * mul * 255.0f)), 0, 255);
        c.setAlpha(a);
        return c;
    }

private:
    std::unique_ptr<IUiComponent> m_child;
    Props   m_p;
    QRect   m_viewport;
    QRect   m_contentRect;

    bool    m_hover{false};
    IconLoader* m_loader{nullptr};
    QOpenGLFunctions* m_gl{nullptr};
    float m_dpr{1.0f};
};

} // namespace UI