#pragma once
#include "UiComponent.hpp"
#include "UiContent.hpp"
#include "ILayoutable.hpp"
#include <functional>
#include <memory>
#include <algorithm>

namespace UI {

	// 可重建的宿主组件：用于将某一段 Widget 构造成的 IUiComponent 动态重建
	class RebuildHost : public IUiComponent, public IUiContent, public ILayoutable {
	public:
		using BuildFn = std::function<std::unique_ptr<IUiComponent>()>;

		RebuildHost() = default;
		~RebuildHost() override = default;

		// 设置子树的构建函数
		// 默认会立即构建一次，避免初始为空（可通过 buildImmediately=false 关闭）
		void setBuilder(BuildFn fn, bool buildImmediately = true) {
			m_builder = std::move(fn);
			if (buildImmediately) {
				requestRebuild();
			}
		}

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
					m_child->updateResourceContext(*m_cache, m_gl, m_dpr);
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

		// ILayoutable
		QSize measure(const SizeConstraints& cs) override {
			if (!m_child) {
				return QSize(std::clamp(0, cs.minW, cs.maxW),
					std::clamp(0, cs.minH, cs.maxH));
			}

			QSize inner(0, 0);
			if (auto* l = dynamic_cast<ILayoutable*>(m_child.get())) {
				inner = l->measure(cs);
			}
			else {
				inner = m_child->bounds().size();
				inner.setWidth(std::clamp(inner.width(), cs.minW, cs.maxW));
				inner.setHeight(std::clamp(inner.height(), cs.minH, cs.maxH));
			}
			return inner;
		}

		void arrange(const QRect& finalRect) override {
			m_viewport = finalRect;
			m_hasViewport = true;
			
			if (!m_child || !finalRect.isValid()) return;

			// Propagate viewport to child
			if (auto* c = dynamic_cast<IUiContent*>(m_child.get())) {
				c->setViewportRect(finalRect);
			}
			// Call child's arrange if available  
			if (auto* l = dynamic_cast<ILayoutable*>(m_child.get())) {
				l->arrange(finalRect);
			}
		}

		// IUiComponent
		void updateLayout(const QSize& windowSize) override {
			m_winSize = windowSize;
			m_hasWinSize = true;
			if (m_child) m_child->updateLayout(windowSize);
		}

		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float devicePixelRatio) override {
			m_cache = &cache; m_gl = gl; m_dpr = devicePixelRatio;
			m_hasCtx = true;
			if (m_child) m_child->updateResourceContext(cache, gl, devicePixelRatio);
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

		bool onWheel(const QPoint& pos, const QPoint& angleDelta) override {
			return m_child ? m_child->onWheel(pos, angleDelta) : false;
		}

		bool tick() override {
			return m_child ? m_child->tick() : false;
		}

		QRect bounds() const override {
			// Prefer the assigned viewport if available and valid
			if (m_hasViewport && m_viewport.isValid()) {
				return m_viewport;
			}
			// Fall back to child bounds or invalid viewport
			return m_child ? m_child->bounds() : m_viewport;
		}

		void onThemeChanged(const bool isDark) override {
			m_isDark = isDark; m_hasTheme = true;
			if (m_child) m_child->onThemeChanged(isDark);
		}

	private:
		BuildFn m_builder;
		std::unique_ptr<IUiComponent> m_child;

		// 环境缓存，供重建后立即同步
		QRect m_viewport;
		QSize m_winSize;
		IconCache* m_cache{ nullptr };
		QOpenGLFunctions* m_gl{ nullptr };
		float m_dpr{ 1.0f };
		bool m_isDark{ false };

		bool m_hasViewport{ false };
		bool m_hasWinSize{ false };
		bool m_hasCtx{ false };
		bool m_hasTheme{ false };
	};

} // namespace UI