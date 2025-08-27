#include "ComponentWrapper.h"
#include "IconCache.h"
#include "RenderData.hpp"
#include <qopenglfunctions.h>

#include "UiContent.hpp"

namespace UI {

	class ProxyComponent : public IUiComponent, public IUiContent {
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
		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float dpr) override {
			if (m_wrapped) m_wrapped->updateResourceContext(loader, gl, dpr);
		}
		void append(Render::FrameData& fd) const override {
			if (m_wrapped) m_wrapped->append(fd);
		}
		bool onMousePress(const QPoint& pos) override { return m_wrapped ? m_wrapped->onMousePress(pos) : false; }
		bool onMouseMove(const QPoint& pos) override { return m_wrapped ? m_wrapped->onMouseMove(pos) : false; }
		bool onMouseRelease(const QPoint& pos) override { return m_wrapped ? m_wrapped->onMouseRelease(pos) : false; }
		bool tick() override { return m_wrapped ? m_wrapped->tick() : false; }
		QRect bounds() const override { return m_wrapped ? m_wrapped->bounds() : m_viewport; }
		void onThemeChanged(bool isDark) override { if (m_wrapped) m_wrapped->onThemeChanged(isDark); }

	private:
		IUiComponent* m_wrapped{ nullptr };
		QRect m_viewport;
	};

	std::unique_ptr<IUiComponent> ComponentWrapper::build() const {
		return std::make_unique<ProxyComponent>(m_component);
	}

} // namespace UI