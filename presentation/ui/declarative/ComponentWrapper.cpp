#include "ComponentWrapper.h"
#include "IconCache.h"
#include "RenderData.hpp"
#include "IFocusable.hpp"
#include "IFocusContainer.hpp"

#include "UiContent.hpp"

namespace UI {

	class ProxyComponent : public IUiComponent, public IUiContent, public IFocusContainer {
	public:
		explicit ProxyComponent(IUiComponent* wrapped) : m_wrapped(wrapped) {}

		// IUiContent
		void setViewportRect(const QRect& r) override {
			m_viewport = r;
			if (auto* c = dynamic_cast<IUiContent*>(m_wrapped)) c->setViewportRect(r);
		}

		// IUiComponent
		void updateLayout(const QSize& windowSize) override {
			if (m_wrapped) m_wrapped->updateLayout(windowSize);
		}
		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, const float dpr) override {
			if (m_wrapped) m_wrapped->updateResourceContext(cache, gl, dpr);
		}
		void append(Render::FrameData& fd) const override {
			if (m_wrapped) m_wrapped->append(fd);
		}
		bool onMousePress(const QPoint& pos) override { return m_wrapped ? m_wrapped->onMousePress(pos) : false; }
		bool onMouseMove(const QPoint& pos) override { return m_wrapped ? m_wrapped->onMouseMove(pos) : false; }
		bool onMouseRelease(const QPoint& pos) override { return m_wrapped ? m_wrapped->onMouseRelease(pos) : false; }
		bool onWheel(const QPoint& pos, const QPoint& angleDelta) override { return m_wrapped ? m_wrapped->onWheel(pos, angleDelta) : false; }
		bool tick() override { return m_wrapped ? m_wrapped->tick() : false; }
		QRect bounds() const override { return m_wrapped ? m_wrapped->bounds() : m_viewport; }
		void onThemeChanged(const bool isDark) override { if (m_wrapped) m_wrapped->onThemeChanged(isDark); }

		// IFocusContainer
		void enumerateFocusables(std::vector<IFocusable*>& out) const override {
			if (!m_wrapped) return;
			
			// 如果包装的组件本身可以获得焦点，添加它
			if (auto* focusable = dynamic_cast<IFocusable*>(m_wrapped)) {
				if (focusable->canFocus()) {
					out.push_back(focusable);
				}
			}
			
			// 如果包装的组件是容器，递归枚举其可焦点子组件
			if (auto* container = dynamic_cast<IFocusContainer*>(m_wrapped)) {
				container->enumerateFocusables(out);
			}
		}

	private:
		IUiComponent* m_wrapped{ nullptr };
		QRect m_viewport;
	};

	std::unique_ptr<IUiComponent> ComponentWrapper::build() const {
		return std::make_unique<ProxyComponent>(m_component);
	}

	void ComponentWrapper::enumerateFocusables(std::vector<IFocusable*>& out) const
	{
		if (!m_component) return;
		
		// 如果包装的组件本身可以获得焦点，添加它
		if (auto* focusable = dynamic_cast<IFocusable*>(m_component)) {
			if (focusable->canFocus()) {
				out.push_back(focusable);
			}
		}
		
		// 如果包装的组件是容器，递归枚举其可焦点子组件
		if (auto* container = dynamic_cast<IFocusContainer*>(m_component)) {
			container->enumerateFocusables(out);
		}
	}

} // namespace UI