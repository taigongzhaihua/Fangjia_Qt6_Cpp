#pragma once
#include "UiTabView.h"
#include "Widget.h"
#include "tab_interface.h"
#include <functional>
#include <memory>
#include <qmargins.h>
#include <UiComponent.hpp>
#include <utility>
#include <vector>

namespace UI {

	// 声明式 TabView：包装 UiTabView（仅支持 VM 模式）
	class TabView : public Widget {
	public:
		TabView() = default;

		// 配置：必须设置 DataProvider
		std::shared_ptr<TabView> dataProvider(fj::presentation::binding::ITabDataProvider* provider) {
			m_dataProvider = provider;
			return self<TabView>();
		}

		std::shared_ptr<TabView> indicatorStyle(const UiTabView::IndicatorStyle s) {
			m_indicatorStyle = s;
			return self<TabView>();
		}

		std::shared_ptr<TabView> tabHeight(const int h) {
			m_tabHeight = h;
			return self<TabView>();
		}

		std::shared_ptr<TabView> animationDuration(const int ms) {
			m_animDuration = ms;
			return self<TabView>();
		}

		// 内容：与 tabs 序列一致（可为空）
		std::shared_ptr<TabView> contents(WidgetList items) {
			m_contents = std::move(items);
			return self<TabView>();
		}

		std::shared_ptr<TabView> setContent(const int index, WidgetPtr item) {
			if (index < 0) return self<TabView>();
			if (static_cast<size_t>(index) >= m_contents.size()) {
				m_contents.resize(static_cast<size_t>(index) + 1);
			}
			m_contents[static_cast<size_t>(index)] = std::move(item);
			return self<TabView>();
		}

		// VM 模式下的回调（当 VM 状态变化时触发）
		std::shared_ptr<TabView> onChanged(std::function<void(int)> cb) {
			m_onChanged = std::move(cb);
			return self<TabView>();
		}

		// 新增：布局与间距
		std::shared_ptr<TabView> margins(const QMargins& m) { m_margin = m; return self<TabView>(); }
		std::shared_ptr<TabView> padding(const QMargins& p) { m_padding = p; return self<TabView>(); }
		std::shared_ptr<TabView> tabBarMargin(const QMargins& m) { m_tabBarMargin = m; return self<TabView>(); }
		std::shared_ptr<TabView> tabBarPadding(const QMargins& p) { m_tabBarPadding = p; return self<TabView>(); }
		std::shared_ptr<TabView> contentMargin(const QMargins& m) { m_contentMargin = m; return self<TabView>(); }
		std::shared_ptr<TabView> contentPadding(const QMargins& p) { m_contentPadding = p; return self<TabView>(); }
		std::shared_ptr<TabView> tabBarSpacing(const int s) { m_tabBarSpacing = s; return self<TabView>(); }
		std::shared_ptr<TabView> spacing(const int s) { m_spacing = s; return self<TabView>(); }

		// 新增：自定义调色板（覆盖主题默认）
		std::shared_ptr<TabView> palette(const UiTabView::Palette& pal) {
			m_palette = pal;
			m_customPalette = true;
			return self<TabView>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		fj::presentation::binding::ITabDataProvider* m_dataProvider{ nullptr };
		UiTabView::IndicatorStyle m_indicatorStyle{ UiTabView::IndicatorStyle::Bottom };
		int m_tabHeight{ 43 };
		int m_animDuration{ 220 };
		WidgetList m_contents;
		std::function<void(int)> m_onChanged;

		// 新增：布局与间距默认值与自定义调色
		QMargins m_margin{ 0,0,0,0 };
		QMargins m_padding{ 0,0,0,0 };
		QMargins m_tabBarMargin{ 0,0,0,0 };
		QMargins m_tabBarPadding{ 8,6,8,6 };
		QMargins m_contentMargin{ 0,0,0,0 };
		QMargins m_contentPadding{ 4,4,4,4 };
		int m_tabBarSpacing{ 4 };
		int m_spacing{ 8 };

		UiTabView::Palette m_palette{};
		bool m_customPalette{ false };
	};

} // namespace UI