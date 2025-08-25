#include "ComponentWrapper.h"
#include "IconLoader.h"
#include "RenderData.hpp"
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>

namespace UI {

	// 代理组件实现
	class ProxyComponent : public IUiComponent {
	public:
		explicit ProxyComponent(IUiComponent* wrapped) : m_wrapped(wrapped) {}

		void updateLayout(const QSize& windowSize) override {
			if (m_wrapped) m_wrapped->updateLayout(windowSize);
		}

		void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override {
			if (m_wrapped) m_wrapped->updateResourceContext(loader, gl, devicePixelRatio);
		}

		void append(Render::FrameData& fd) const override {
			if (m_wrapped) m_wrapped->append(fd);
		}

		bool onMousePress(const QPoint& pos) override {
			return m_wrapped ? m_wrapped->onMousePress(pos) : false;
		}

		bool onMouseMove(const QPoint& pos) override {
			return m_wrapped ? m_wrapped->onMouseMove(pos) : false;
		}

		bool onMouseRelease(const QPoint& pos) override {
			return m_wrapped ? m_wrapped->onMouseRelease(pos) : false;
		}

		bool tick() override {
			return m_wrapped ? m_wrapped->tick() : false;
		}

		QRect bounds() const override {
			return m_wrapped ? m_wrapped->bounds() : QRect();
		}

		void onThemeChanged(bool isDark) override {
			if (m_wrapped) m_wrapped->onThemeChanged(isDark);
		}

	private:
		IUiComponent* m_wrapped;
	};

	std::unique_ptr<IUiComponent> ComponentWrapper::build() const {
		return std::make_unique<ProxyComponent>(m_component);
	}

} // namespace UI