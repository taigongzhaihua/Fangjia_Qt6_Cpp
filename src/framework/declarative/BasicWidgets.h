#pragma once
#include "Widget.h"
#include <qfont.h>
#include <qstring.h>

#include "Layouts.h"

namespace UI {

	// 文本组件
	class Text : public Widget {
	public:
		enum class Overflow {
			Visible,  // 超出也继续绘制（尽量避免）
			Clip,     // 按容器边界裁切
			Ellipsis  // 超出用省略号
		};

		explicit Text(QString text) : m_text(std::move(text)) {}

		// 显式设色：一旦调用则不再跟随主题自动变色
		std::shared_ptr<Text> color(QColor c) {
			m_color = c;
			m_autoColor = false;
			m_useThemeColor = false; // 显式设色优先，取消主题色
			return self<Text>();
		}

		// 新增：分别设置亮/暗两套颜色，随主题自动切换
		std::shared_ptr<Text> themeColor(QColor light, QColor dark) {
			m_colorLight = light;
			m_colorDark = dark;
			m_useThemeColor = true;
			m_autoColor = false; // 主题色优先，关闭默认自动配色
			return self<Text>();
		}

		std::shared_ptr<Text> fontSize(int size) {
			m_fontSize = size;
			return self<Text>();
		}

		std::shared_ptr<Text> fontWeight(QFont::Weight weight) {
			m_fontWeight = weight;
			return self<Text>();
		}

		// 文本在自身矩形内的对齐（水平+垂直）
		std::shared_ptr<Text> align(Qt::Alignment align) {
			m_alignment = align;
			return self<Text>();
		}

		// 开启自动换行（默认 false）
		std::shared_ptr<Text> wrap(bool on = true) {
			m_wrap = on;
			// 默认多行时不限行数（0 表示不限），单行时限定为 1
			if (on && m_maxLines == 1) m_maxLines = 0;
			if (!on && m_maxLines == 0) m_maxLines = 1;
			return self<Text>();
		}

		// 最大行数（0 表示不限）
		std::shared_ptr<Text> maxLines(int n) {
			m_maxLines = std::max(0, n);
			return self<Text>();
		}

		// 溢出处理：Visible / Clip / Ellipsis
		std::shared_ptr<Text> overflow(Overflow o) {
			m_overflow = o;
			return self<Text>();
		}

		// 单词优先换行（默认 true），false 时按字符断行
		std::shared_ptr<Text> wordWrap(bool on) {
			m_wordWrap = on;
			return self<Text>();
		}

		// 行距（像素，逻辑像素；负数表示使用默认行距）
		std::shared_ptr<Text> lineSpacing(int px) {
			m_lineSpacing = px;
			return self<Text>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		QString m_text;
		QColor m_color{ 0, 0, 0 };      // 若未显式设色，将在 onThemeChanged 中根据主题覆盖
		int m_fontSize{ 14 };
		QFont::Weight m_fontWeight{ QFont::Normal };
		Qt::Alignment m_alignment{ Qt::AlignLeft | Qt::AlignTop };
		bool m_autoColor{ true };       // 自动跟随主题（未设主题色/显式色时）

		// 新增：按主题切换的颜色
		bool  m_useThemeColor{ false };
		QColor m_colorLight{ QColor(30,35,40) };
		QColor m_colorDark{ QColor(240,245,250) };

		bool m_wrap{ false };
		int m_maxLines{ 1 };            // 0 = 不限；wrap=false 时建议为 1
		Overflow m_overflow{ Overflow::Clip };
		bool m_wordWrap{ true };
		int  m_lineSpacing{ -1 };       // -1 使用默认（基于字体高度的 0.2 倍）
	};

	// 图标组件
	class Icon : public Widget {
	public:
		explicit Icon(QString path) : m_path(std::move(path)) {}

		std::shared_ptr<Icon> color(QColor c) {
			m_color = c;
			m_autoColor = false;
			return self<Icon>();
		}

		std::shared_ptr<Icon> size(int s) {
			m_size = s;
			return self<Icon>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		QString m_path;
		QColor m_color;
		int m_size{ 24 };
		bool m_autoColor{ true };       // 预留：如需可按主题自动变色
	};

	// 按钮组件
	class Button : public Widget {
	public:
		enum class ButtonStyle {
			Primary,
			Secondary,
			Text,
			Outlined
		};
		explicit Button(WidgetPtr child) : m_child(std::move(child)) {}

		std::shared_ptr<Button> style(ButtonStyle s) {
			m_style = s;
			return self<Button>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetPtr m_child;
		ButtonStyle m_style{ ButtonStyle::Primary };
	};

	// 容器组件
	class Container : public Widget {
	public:
		explicit Container(WidgetPtr child = nullptr) : m_child(std::move(child)) {}

		std::shared_ptr<Container> child(WidgetPtr c) {
			m_child = std::move(c);
			return self<Container>();
		}

		std::shared_ptr<Container> alignment(Alignment align) {
			m_alignment = align;
			return self<Container>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetPtr m_child;
		Alignment m_alignment{ Alignment::Center };
	};

} // namespace UI