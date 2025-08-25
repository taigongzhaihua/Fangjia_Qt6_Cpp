#include "IconLoader.h"
#include "RenderData.hpp"
#include "TabView.h"
#include "UiContent.hpp"
#include "UiTabView.h"
#include <functional>
#include <memory>
#include <qcontainerfwd.h>
#include <qmargins.h>
#include <qopenglfunctions.h>
#include <qpoint.h>
#include <qrect.h>
#include <qsize.h>
#include <UiComponent.hpp>
#include <utility>
#include <vector>

namespace UI {

	// 运行期组件：拥有 UiTabView 与其内容，转发 IUiComponent/IUiContent
	class TabViewComponent : public IUiComponent, public IUiContent {
	public:
		struct Props {
			TabViewModel* vm{ nullptr };
			QStringList tabs;
			int selectedIndex{ 0 };
			UiTabView::IndicatorStyle indicatorStyle{ UiTabView::IndicatorStyle::Bottom };
			int tabHeight{ 43 };
			int animDuration{ 220 };
			std::function<void(int)> onChanged;

			// 新增：布局与间距 + 自定义调色
			QMargins margin{ 0,0,0,0 };
			QMargins padding{ 0,0,0,0 };
			QMargins tabBarMargin{ 0,0,0,0 };
			QMargins tabBarPadding{ 8,6,8,6 };
			QMargins contentMargin{ 0,0,0,0 };
			QMargins contentPadding{ 4,4,4,4 };
			int tabBarSpacing{ 4 };
			int spacing{ 8 };
			UiTabView::Palette palette{};
			bool customPalette{ false };
		};

		TabViewComponent(Props p,
			std::vector<std::unique_ptr<IUiComponent>> contents)
			: m_props(std::move(p)), m_contents(std::move(contents))
		{
			// 配置 UiTabView
			if (m_props.customPalette) {
				m_view.setPalette(m_props.palette);
			}
			m_view.setIndicatorStyle(m_props.indicatorStyle);
			m_view.setTabHeight(m_props.tabHeight);
			m_view.setAnimationDuration(m_props.animDuration);

			// 透传布局与间距
			m_view.setMargins(m_props.margin);
			m_view.setPadding(m_props.padding);
			m_view.setTabBarMargin(m_props.tabBarMargin);
			m_view.setTabBarPadding(m_props.tabBarPadding);
			m_view.setContentMargin(m_props.contentMargin);
			m_view.setContentPadding(m_props.contentPadding);
			m_view.setTabBarSpacing(m_props.tabBarSpacing);
			m_view.setSpacing(m_props.spacing);

			if (m_props.vm) {
				m_view.setViewModel(m_props.vm);
			}
			else if (!m_props.tabs.isEmpty()) {
				m_view.setTabs(m_props.tabs);
				m_view.setSelectedIndex(m_props.selectedIndex);
			}

			// 设置每个 tab 的内容（UiTabView 内部仅存裸指针，这里需持有 unique_ptr 保证生命周期）
			for (size_t i = 0; i < m_contents.size(); ++i) {
				if (m_contents[i]) {
					m_view.setContent(static_cast<int>(i), m_contents[i].get());
				}
			}

			// 初始化 lastSelected
			m_lastSelected = m_view.selectedIndex();
		}

		// IUiContent
		void setViewportRect(const QRect& r) override {
			m_view.setViewportRect(r);
		}

		// IUiComponent
		void updateLayout(const QSize& windowSize) override {
			m_view.updateLayout(windowSize);
		}

		void updateResourceContext(IconLoader& loader, QOpenGLFunctions* gl, float devicePixelRatio) override {
			m_view.updateResourceContext(loader, gl, devicePixelRatio);
			// 子内容资源上下文 UiTabView 内部已在当前选项卡上处理
		}

		void append(Render::FrameData& fd) const override {
			m_view.append(fd);
		}

		bool onMousePress(const QPoint& pos) override { return m_view.onMousePress(pos); }
		bool onMouseMove(const QPoint& pos) override { return m_view.onMouseMove(pos); }
		bool onMouseRelease(const QPoint& pos) override { return m_view.onMouseRelease(pos); }

		bool tick() override {
			bool any = m_view.tick();

			// 在所有模式下检测选中变化并回调（VM 模式与 fallback 模式均支持）
			if (m_props.onChanged) {
				const int cur = m_view.selectedIndex();
				if (cur != m_lastSelected) {
					m_lastSelected = cur;
					m_props.onChanged(cur);
					any = true;
				}
			}

			// 让当前内容也有机会进行动画（若有的话）
			if (IUiComponent* curContent = m_view.content(m_view.selectedIndex())) {
				any = curContent->tick() || any;
			}
			return any;
		}

		QRect bounds() const override { return m_view.bounds(); }

		void onThemeChanged(bool isDark) override {
			m_view.onThemeChanged(isDark);
			// 当前内容主题变更 UiTabView 内部已传播
		}

	private:
		Props m_props;
		UiTabView m_view;
		std::vector<std::unique_ptr<IUiComponent>> m_contents;
		int m_lastSelected{ -1 };
	};

	std::unique_ptr<IUiComponent> TabView::build() const
	{
		// 将 Widget 内容先 build 成 IUiComponent 并在 TabViewComponent 中持有
		std::vector<std::unique_ptr<IUiComponent>> built;
		built.reserve(m_contents.size());
		for (const auto& w : m_contents) {
			built.push_back(w ? w->build() : nullptr);
		}

		TabViewComponent::Props p;
		p.vm = m_vm;
		p.tabs = m_tabs;
		p.selectedIndex = m_selectedIndex;
		p.indicatorStyle = m_indicatorStyle;
		p.tabHeight = m_tabHeight;
		p.animDuration = m_animDuration;
		p.onChanged = m_onChanged;

		// 透传布局与间距、自定义调色板
		p.margin = m_margin;
		p.padding = m_padding;
		p.tabBarMargin = m_tabBarMargin;
		p.tabBarPadding = m_tabBarPadding;
		p.contentMargin = m_contentMargin;
		p.contentPadding = m_contentPadding;
		p.tabBarSpacing = m_tabBarSpacing;
		p.spacing = m_spacing;
		p.palette = m_palette;
		p.customPalette = m_customPalette;

		auto comp = std::make_unique<TabViewComponent>(std::move(p), std::move(built));

		// 应用通用装饰（当前 applyDecorations 为空实现；如后续引入 DecoratedBox，可替换为 decorate）
		applyDecorations(comp.get());
		return comp;
	}

} // namespace UI