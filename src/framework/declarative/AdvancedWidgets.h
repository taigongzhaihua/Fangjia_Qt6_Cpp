#pragma once
#include "Layouts.h"
#include "UiPanel.h"
#include "Widget.h"
#include <qmargins.h>

namespace UI
{
	// Card组件
	class Card : public Widget
	{
	public:
		struct Palette {
			// 亮/暗两套背景颜色
			QColor bgLight{ QColor(255,255,255,245) };
			QColor bgDark{ QColor(28,38,50,220) };

			// 可选边框（默认无）
			QColor borderLight{ Qt::transparent };
			QColor borderDark{ Qt::transparent };
			float  borderW{ 0.0f };

			float  radius{ 8.0f };
			QMargins padding{ 16,16,16,16 };
		};

		explicit Card(WidgetPtr child) : m_child(std::move(child)) {}

		// 阴影占位（暂留，不影响绘制）
		std::shared_ptr<Card> elevation(float e) { m_elevation = e; return self<Card>(); }

		// 配置亮/暗主题的背景（外部可分别设置）
		std::shared_ptr<Card> backgroundTheme(QColor light, QColor dark, float radius = 8.0f) {
			m_pal.bgLight = light; m_pal.bgDark = dark; m_pal.radius = radius; return self<Card>();
		}
		// 配置亮/暗主题的边框（外部可分别设置）
		std::shared_ptr<Card> borderTheme(QColor light, QColor dark, float width = 1.0f, float radius = -1.0f) {
			m_pal.borderLight = light; m_pal.borderDark = dark; m_pal.borderW = std::max(0.0f, width);
			if (radius >= 0.0f) m_pal.radius = radius;
			return self<Card>();
		}
		// 配置 padding
		std::shared_ptr<Card> padding(const QMargins& p) { m_pal.padding = p; return self<Card>(); }
		std::shared_ptr<Card> padding(int all) { m_pal.padding = QMargins(all, all, all, all); return self<Card>(); }
		std::shared_ptr<Card> padding(int h, int v) { m_pal.padding = QMargins(h, v, h, v); return self<Card>(); }
		std::shared_ptr<Card> padding(int l, int t, int r, int b) { m_pal.padding = QMargins(l, t, r, b); return self<Card>(); }

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetPtr m_child;
		float m_elevation{ 2.0f };
		Palette m_pal;              // 主题化配色/内边距
	};


	// 条件渲染
	class Conditional : public Widget
	{
	public:
		Conditional(bool condition, WidgetPtr ifTrue, WidgetPtr ifFalse = nullptr)
			: m_condition(condition), m_ifTrue(std::move(ifTrue)), m_ifFalse(std::move(ifFalse))
		{
		}

		std::unique_ptr<IUiComponent> build() const override
		{
			if (m_condition && m_ifTrue)
			{
				return m_ifTrue->build();
			}
			else if (m_ifFalse)
			{
				return m_ifFalse->build();
			}
			return std::make_unique<UiPanel>();
		}

	private:
		bool m_condition;
		WidgetPtr m_ifTrue;
		WidgetPtr m_ifFalse;
	};

	// 列表渲染
	template <typename T>
	class ListView : public Widget
	{
	public:
		using ItemBuilder = std::function<WidgetPtr(const T&, int)>;

		ListView(std::vector<T> items, ItemBuilder builder)
			: m_items(std::move(items)), m_builder(std::move(builder))
		{
		}

		std::unique_ptr<IUiComponent> build() const override
		{
			WidgetList children;
			for (int i = 0; i < m_items.size(); ++i)
			{
				children.push_back(m_builder(m_items[i], i));
			}
			return make_widget<Panel>(children)->build();
		}

	private:
		std::vector<T> m_items;
		ItemBuilder m_builder;
	};
} // namespace UI