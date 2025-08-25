#pragma once
#include "BasicWidgets.h"
#include "Layouts.h"
#include "Widget.h"

namespace UI {

	// Card组件
	class Card : public Widget {
	public:
		explicit Card(WidgetPtr child) : m_child(std::move(child)) {}

		std::shared_ptr<Card> elevation(float e) {
			m_elevation = e;
			return std::static_pointer_cast<Card>(shared_from_this());
		}

		std::unique_ptr<IUiComponent> build() const override {
			return Container(m_child)
				.padding(16)
				->background(QColor(255, 255, 255), 8.0f)
				->build();
		}

	private:
		WidgetPtr m_child;
		float m_elevation{ 2.0f };
	};

	// ListTile组件
	class ListTile : public Widget {
	public:
		ListTile() = default;

		std::shared_ptr<ListTile> leading(WidgetPtr w) {
			m_leading = std::move(w);
			return std::static_pointer_cast<ListTile>(shared_from_this());
		}

		std::shared_ptr<ListTile> title(WidgetPtr w) {
			m_title = std::move(w);
			return std::static_pointer_cast<ListTile>(shared_from_this());
		}

		std::shared_ptr<ListTile> subtitle(WidgetPtr w) {
			m_subtitle = std::move(w);
			return std::static_pointer_cast<ListTile>(shared_from_this());
		}

		std::shared_ptr<ListTile> trailing(WidgetPtr w) {
			m_trailing = std::move(w);
			return std::static_pointer_cast<ListTile>(shared_from_this());
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetPtr m_leading;
		WidgetPtr m_title;
		WidgetPtr m_subtitle;
		WidgetPtr m_trailing;
	};

	// TabBar组件
	class TabBar : public Widget {
	public:
		struct Tab {
			QString label;
			WidgetPtr icon;
		};

		explicit TabBar(std::vector<Tab> tabs) : m_tabs(std::move(tabs)) {}

		std::shared_ptr<TabBar> selectedIndex(int index) {
			m_selectedIndex = index;
			return std::static_pointer_cast<TabBar>(shared_from_this());
		}

		std::shared_ptr<TabBar> onChanged(std::function<void(int)> handler) {
			m_onChanged = std::move(handler);
			return std::static_pointer_cast<TabBar>(shared_from_this());
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		std::vector<Tab> m_tabs;
		int m_selectedIndex{ 0 };
		std::function<void(int)> m_onChanged;
	};

	// 条件渲染
	class Conditional : public Widget {
	public:
		Conditional(bool condition, WidgetPtr ifTrue, WidgetPtr ifFalse = nullptr)
			: m_condition(condition), m_ifTrue(std::move(ifTrue)), m_ifFalse(std::move(ifFalse)) {
		}

		std::unique_ptr<IUiComponent> build() const override {
			if (m_condition && m_ifTrue) {
				return m_ifTrue->build();
			}
			else if (m_ifFalse) {
				return m_ifFalse->build();
			}
			return std::make_unique<UiBoxLayout>();
		}

	private:
		bool m_condition;
		WidgetPtr m_ifTrue;
		WidgetPtr m_ifFalse;
	};

	// 列表渲染
	template<typename T>
	class ListView : public Widget {
	public:
		using ItemBuilder = std::function<WidgetPtr(const T&, int)>;

		ListView(std::vector<T> items, ItemBuilder builder)
			: m_items(std::move(items)), m_builder(std::move(builder)) {
		}

		std::unique_ptr<IUiComponent> build() const override {
			WidgetList children;
			for (int i = 0; i < m_items.size(); ++i) {
				children.push_back(m_builder(m_items[i], i));
			}
			return Column(children).build();
		}

	private:
		std::vector<T> m_items;
		ItemBuilder m_builder;
	};

} // namespace UI