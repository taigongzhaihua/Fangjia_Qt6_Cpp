#pragma once
#include "UiTabView.h"
#include "Widget.h"
#include <functional>
#include <memory>
#include <qcontainerfwd.h>
#include <qmargins.h>
#include <UiComponent.hpp>
#include <utility>
#include <vector>

namespace UI {

	// 声明式 TabView：包装 UiTabView
	class TabView : public Widget {
	public:
		TabView() = default;

		// 配置：VM 或 fallback tabs（二选一；同时存在以 VM 为准）
		std::shared_ptr<TabView> viewModel(TabViewModel* vm) {
			m_vm = vm;
			return self<TabView>();
		}

		std::shared_ptr<TabView> tabs(QStringList labels) {
			m_tabs = std::move(labels);
			return self<TabView>();
		}

		std::shared_ptr<TabView> selectedIndex(int idx) {
			m_selectedIndex = idx;
			return self<TabView>();
		}

		std::shared_ptr<TabView> indicatorStyle(UiTabView::IndicatorStyle s) {
			m_indicatorStyle = s;
			return self<TabView>();
		}

		std::shared_ptr<TabView> tabHeight(int h) {
			m_tabHeight = h;
			return self<TabView>();
		}

		std::shared_ptr<TabView> animationDuration(int ms) {
			m_animDuration = ms;
			return self<TabView>();
		}

		// 内容：与 tabs 序列一致（可为空）
		std::shared_ptr<TabView> contents(WidgetList items) {
			m_contents = std::move(items);
			return self<TabView>();
		}

		std::shared_ptr<TabView> setContent(int index, WidgetPtr item) {
			if (index < 0) return self<TabView>();
			if (static_cast<size_t>(index) >= m_contents.size()) {
				m_contents.resize(static_cast<size_t>(index) + 1);
			}
			m_contents[static_cast<size_t>(index)] = std::move(item);
			return self<TabView>();
		}

		// 非 VM 模式下使用（VM 模式下也会触发回调）
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
		std::shared_ptr<TabView> tabBarSpacing(int s) { m_tabBarSpacing = s; return self<TabView>(); }
		std::shared_ptr<TabView> spacing(int s) { m_spacing = s; return self<TabView>(); }

		// 新增：自定义调色板（覆盖主题默认）
		std::shared_ptr<TabView> palette(const UiTabView::Palette& pal) {
			m_palette = pal;
			m_customPalette = true;
			return self<TabView>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		TabViewModel* m_vm{ nullptr };
		QStringList m_tabs;
		int m_selectedIndex{ 0 };
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