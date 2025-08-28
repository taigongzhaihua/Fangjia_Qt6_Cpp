#include "ScrollView.h"
#include "UiScrollView.h"
#include "ILayoutable.hpp"
#include "UiContent.hpp"
#include "UiComponent.hpp"
#include <algorithm>
#include <memory>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>

namespace UI {

	// 运行期组件：拥有 UiScrollView 与其内容，转发 IUiComponent/IUiContent/ILayoutable
	class ScrollViewComponent : public IUiComponent, public IUiContent, public ILayoutable {
	public:
		explicit ScrollViewComponent(std::unique_ptr<IUiComponent> child)
			: m_builtChild(std::move(child))
		{
			// 将构建好的子组件设置给滚动视图
			m_scrollView.setChild(m_builtChild.get());
		}

		~ScrollViewComponent() override = default;

		// IUiContent - 转发给 UiScrollView
		void setViewportRect(const QRect& r) override {
			m_scrollView.setViewportRect(r);
		}

		// ILayoutable - 转发给 UiScrollView
		QSize measure(const SizeConstraints& cs) override {
			return m_scrollView.measure(cs);
		}

		void arrange(const QRect& finalRect) override {
			m_scrollView.arrange(finalRect);
		}

		// IUiComponent - 转发给 UiScrollView
		void updateLayout(const QSize& windowSize) override {
			m_scrollView.updateLayout(windowSize);
		}

		void updateResourceContext(IconCache& cache, QOpenGLFunctions* gl, float devicePixelRatio) override {
			m_scrollView.updateResourceContext(cache, gl, devicePixelRatio);
		}

		void append(Render::FrameData& fd) const override {
			m_scrollView.append(fd);
		}

		bool onMousePress(const QPoint& pos) override {
			return m_scrollView.onMousePress(pos);
		}

		bool onMouseMove(const QPoint& pos) override {
			return m_scrollView.onMouseMove(pos);
		}

		bool onMouseRelease(const QPoint& pos) override {
			return m_scrollView.onMouseRelease(pos);
		}

		bool tick() override {
			return m_scrollView.tick();
		}

		QRect bounds() const override {
			return m_scrollView.bounds();
		}

		void applyTheme(bool isDark) override {
			m_scrollView.applyTheme(isDark);
		}

	private:
		UiScrollView m_scrollView;
		std::unique_ptr<IUiComponent> m_builtChild;
	};

	std::unique_ptr<IUiComponent> ScrollView::build() const
	{
		// 将子 Widget 先 build 成 IUiComponent
		std::unique_ptr<IUiComponent> builtChild;
		if (m_child) {
			builtChild = m_child->build();
		}

		return std::make_unique<ScrollViewComponent>(std::move(builtChild));
	}

} // namespace UI