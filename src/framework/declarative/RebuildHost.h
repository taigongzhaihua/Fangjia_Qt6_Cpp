#pragma once
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include <functional>
#include <memory>

namespace UI {

// 可重建的宿主组件：用于将某一段 Widget 构造成的 IUiComponent 动态重建
class RebuildHost : public IUiComponent, public IUiContent {
public:
    using BuildFn = std::function<std::unique_ptr<IUiComponent>()>;

    RebuildHost() = default;
    ~RebuildHost() override = default;

    // 设置子树的构建函数
    void setBuilder(BuildFn fn) { m_builder = std::move(fn); }

    // 请求重建（可在任意时机调用，比如 VM 的某个 signal 里）
    void requestRebuild() {
        if (!m_builder) return;
        m_child = m_builder();
        // 重建后立即同步上下文与视口
        if (m_child) {
            if (m_hasViewport) {
                if (auto* c = dynamic_cast<IUiContent*>(m_child.get())) {
                    c->setViewportRect(m_viewport);
                }
            }
            if (m_hasCtx) {
                m_child->updateResourceContext(*m_loader, m_gl, m_dpr);
            }
            if (m_hasWinSize) {
                m_child->updateLayout(m_winSize);
            }
            if (m_hasTheme) {
                m_child->onThemeChanged(m_isDark);
            }
        }
    }

    // IUiContent
    void setViewportRect(const QRect& r) override {
        m_viewport = r;
        m_hasViewport = true;
        if (auto* c = dynamic_cast<IUiContent*>(m_child.get())) {
            c->setViewportRect(r);
        }
    }

    // IUiComponent
    void updateLayout(const QSize& windowSize) override {
        m_winSize = windowSize;
        m_hasWinSize = true;
        if (m_child) m_child->updateLayout(windowSize);
    }

    void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override {
        m_loader = &loader; m_gl = gl; m_dpr = devicePixelRatio;
        m_hasCtx = true;
        if (m_child) m_child->updateResourceContext(loader, gl, devicePixelRatio);
    }

    void append(Render::FrameData& fd) const override {
        if (m_child) m_child->append(fd);
    }

    bool onMousePress(const QPoint& pos) override {
        return m_child ? m_child->onMousePress(pos) : false;
    }
    bool onMouseMove(const QPoint& pos) override {
        return m_child ? m_child->onMouseMove(pos) : false;
    }
    bool onMouseRelease(const QPoint& pos) override {
        return m_child ? m_child->onMouseRelease(pos) : false;
    }

    bool tick() override {
        return m_child ? m_child->tick() : false;
    }

    QRect bounds() const override {
        return m_child ? m_child->bounds() : m_viewport;
    }

    void onThemeChanged(bool isDark) override {
        m_isDark = isDark; m_hasTheme = true;
        if (m_child) m_child->onThemeChanged(isDark);
    }

private:
    BuildFn m_builder;
    std::unique_ptr<IUiComponent> m_child;

    // 环境缓存，供重建后立即同步
    QRect m_viewport;
    QSize m_winSize;
    IconLoader* m_loader{nullptr};
    QOpenGLFunctions* m_gl{nullptr};
    float m_dpr{1.0f};
    bool m_isDark{false};

    bool m_hasViewport{false};
    bool m_hasWinSize{false};
    bool m_hasCtx{false};
    bool m_hasTheme{false};
};

} // namespace UI