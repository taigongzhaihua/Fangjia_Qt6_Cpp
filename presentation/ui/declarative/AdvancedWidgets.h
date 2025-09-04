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

	// 声明式 Popup：包装 UiPopup 提供弹出窗口控件
	class Popup : public Widget
	{
	public:
		/// 弹出位置策略
		enum class Placement {
			Bottom,      // 在触发器下方
			Top,         // 在触发器上方
			Right,       // 在触发器右侧
			Left,        // 在触发器左侧
			BottomLeft,  // 在触发器左下方
			BottomRight, // 在触发器右下方
			TopLeft,     // 在触发器左上方
			TopRight,    // 在触发器右上方
			Custom       // 自定义位置
		};

		Popup() = default;

		/// 设置触发器内容
		/// 参数：trigger — 触发器组件
		/// 返回：当前Popup实例（支持链式调用）
		/// 说明：触发器通常是按钮或其他可交互元素
		std::shared_ptr<Popup> trigger(WidgetPtr trigger) {
			m_trigger = std::move(trigger);
			return self<Popup>();
		}

		/// 设置弹出内容
		/// 参数：content — 弹出内容组件
		/// 返回：当前Popup实例（支持链式调用）
		/// 说明：弹出时显示的内容
		std::shared_ptr<Popup> content(WidgetPtr content) {
			m_content = std::move(content);
			return self<Popup>();
		}

		/// 设置弹出窗口大小
		/// 参数：size — 窗口大小
		/// 返回：当前Popup实例（支持链式调用）
		std::shared_ptr<Popup> size(const QSize& size) {
			m_popupSize = size;
			return self<Popup>();
		}

		/// 设置弹出位置策略
		/// 参数：placement — 位置策略
		/// 返回：当前Popup实例（支持链式调用）
		std::shared_ptr<Popup> placement(Placement placement) {
			m_placement = placement;
			return self<Popup>();
		}

		/// 设置位置偏移
		/// 参数：offset — 偏移量
		/// 返回：当前Popup实例（支持链式调用）
		std::shared_ptr<Popup> offset(const QPoint& offset) {
			m_offset = offset;
			return self<Popup>();
		}

		/// 设置背景样式
		/// 参数：backgroundColor — 背景颜色
		/// 参数：cornerRadius — 圆角半径
		/// 返回：当前Popup实例（支持链式调用）
		std::shared_ptr<Popup> style(const QColor& backgroundColor, float cornerRadius = 8.0f) {
			m_backgroundColor = backgroundColor;
			m_cornerRadius = cornerRadius;
			return self<Popup>();
		}

		/// 设置点击外部时是否关闭
		/// 参数：close — 是否在点击外部时关闭
		/// 返回：当前Popup实例（支持链式调用）
		std::shared_ptr<Popup> closeOnClickOutside(bool close = true) {
			m_closeOnClickOutside = close;
			return self<Popup>();
		}

		/// 设置可见性变化回调
		/// 参数：callback — 可见性变化回调函数
		/// 返回：当前Popup实例（支持链式调用）
		std::shared_ptr<Popup> onVisibilityChanged(std::function<void(bool)> callback) {
			m_onVisibilityChanged = std::move(callback);
			return self<Popup>();
		}

		std::unique_ptr<IUiComponent> build() const override;
		
		/// 使用父窗口构建弹出组件（推荐使用）
		/// 参数：parentWindow — 父窗口指针
		/// 返回：构建的弹出组件
		std::unique_ptr<IUiComponent> buildWithWindow(QWindow* parentWindow) const;

		/// 辅助函数：配置弹出窗口上下文
		/// 参数：component — build()返回的组件
		/// 参数：parentWindow — 父窗口指针
		/// 说明：必须在将组件添加到UI树之前调用
		static void configurePopupWindow(IUiComponent* component, QWindow* parentWindow);

	private:
		WidgetPtr m_trigger;
		WidgetPtr m_content;
		QSize m_popupSize{ 200, 150 };
		Placement m_placement{ Placement::Bottom };
		QPoint m_offset{ 0, 0 };
		QColor m_backgroundColor{ 255, 255, 255, 240 };
		float m_cornerRadius{ 8.0f };
		bool m_closeOnClickOutside{ true };
		std::function<void(bool)> m_onVisibilityChanged;
	};
} // namespace UI