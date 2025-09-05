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

		// 阴影高度控制（elevation 映射为实际阴影效果）
		std::shared_ptr<Card> elevation(const float e) { m_elevation = e; return self<Card>(); }

		// 配置亮/暗主题的背景（外部可分别设置）
		std::shared_ptr<Card> backgroundTheme(const QColor light, const QColor dark, const float radius = 8.0f) {
			m_pal.bgLight = light; m_pal.bgDark = dark; m_pal.radius = radius; return self<Card>();
		}
		// 配置亮/暗主题的边框（外部可分别设置）
		std::shared_ptr<Card> borderTheme(const QColor light, const QColor dark, const float width = 1.0f, const float radius = -1.0f) {
			m_pal.borderLight = light; m_pal.borderDark = dark; m_pal.borderW = std::max(0.0f, width);
			if (radius >= 0.0f) m_pal.radius = radius;
			return self<Card>();
		}
		// 配置 padding
		std::shared_ptr<Card> padding(const QMargins& p) { m_pal.padding = p; return self<Card>(); }
		std::shared_ptr<Card> padding(const int all) { m_pal.padding = QMargins(all, all, all, all); return self<Card>(); }
		std::shared_ptr<Card> padding(const int h, const int v) { m_pal.padding = QMargins(h, v, h, v); return self<Card>(); }
		std::shared_ptr<Card> padding(const int l, const int t, const int r, const int b) { m_pal.padding = QMargins(l, t, r, b); return self<Card>(); }

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
		Conditional(const bool condition, WidgetPtr ifTrue, WidgetPtr ifFalse = nullptr)
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

	// 声明式 ListBox：包装 UiListBox 提供单选列表控件
	class ListBox : public Widget
	{
	public:
		ListBox() = default;

		// 设置项目列表
		std::shared_ptr<ListBox> items(const std::vector<QString>& items) {
			m_items = items;
			return self<ListBox>();
		}

		// 设置项目高度
		std::shared_ptr<ListBox> itemHeight(int height) {
			m_itemHeight = height;
			return self<ListBox>();
		}

		// 设置选中索引
		std::shared_ptr<ListBox> selectedIndex(int index) {
			m_selectedIndex = index;
			return self<ListBox>();
		}

		// 设置激活回调
		std::shared_ptr<ListBox> onActivated(std::function<void(int)> callback) {
			m_onActivated = std::move(callback);
			return self<ListBox>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		std::vector<QString> m_items;
		int m_itemHeight{ 36 };
		int m_selectedIndex{ -1 };
		std::function<void(int)> m_onActivated;
	};

	// 新的简洁弹出组件 - 无内置触发器
	class Popup : public Widget
	{
	public:
		/// 弹出位置策略
		enum class Placement {
			Bottom,      // 相对于指定位置下方
			Top,         // 相对于指定位置上方
			Right,       // 相对于指定位置右侧
			Left,        // 相对于指定位置左侧
			BottomLeft,  // 相对于指定位置左下方
			BottomRight, // 相对于指定位置右下方
			TopLeft,     // 相对于指定位置左上方
			TopRight,    // 相对于指定位置右上方
			Center       // 屏幕中央
		};

		Popup() = default;

		/// 设置弹出内容
		std::shared_ptr<Popup> content(WidgetPtr content) {
			m_content = std::move(content);
			return self<Popup>();
		}

		/// 设置弹出窗口大小
		std::shared_ptr<Popup> size(const QSize& size) {
			m_popupSize = size;
			return self<Popup>();
		}

		/// 设置弹出位置策略
		std::shared_ptr<Popup> placement(Placement placement) {
			m_placement = placement;
			return self<Popup>();
		}

		/// 设置位置偏移
		std::shared_ptr<Popup> offset(const QPoint& offset) {
			m_offset = offset;
			return self<Popup>();
		}

		/// 设置背景样式
		std::shared_ptr<Popup> backgroundColor(const QColor& color) {
			m_backgroundColor = color;
			return self<Popup>();
		}

		/// 设置圆角半径
		std::shared_ptr<Popup> cornerRadius(float radius) {
			m_cornerRadius = radius;
			return self<Popup>();
		}

		/// 设置可见性变化回调
		std::shared_ptr<Popup> onVisibilityChanged(std::function<void(bool)> callback) {
			m_onVisibilityChanged = std::move(callback);
			return self<Popup>();
		}

		/// 设置依附对象，作为弹出位置参考
		std::shared_ptr<Popup> attachTo(WidgetPtr attachmentObject) {
			m_attachmentObject = std::move(attachmentObject);
			return self<Popup>();
		}

		/// 构建弹出组件 - 需要父窗口上下文
		std::unique_ptr<IUiComponent> buildWithWindow(QWindow* parentWindow) const;

		// 保持向后兼容的build()方法
		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetPtr m_content;
		WidgetPtr m_attachmentObject;  // 依附对象，作为弹出位置参考
		QSize m_popupSize{200, 150};
		Placement m_placement{Placement::Bottom};
		QPoint m_offset{0, 0};
		QColor m_backgroundColor{255, 255, 255, 240};
		float m_cornerRadius{8.0f};
		std::function<void(bool)> m_onVisibilityChanged;
	};


} // namespace UI