#pragma once
#include "Widget.h"
#include <qfont.h>
#include <qstring.h>

#include "Layouts.h"

namespace UI {

	// 文本组件
	class Text : public Widget {
	public:
		explicit Text(QString text) : m_text(std::move(text)) {}

		// 显式设色：一旦调用则不再跟随主题自动变色
		std::shared_ptr<Text> color(QColor c) {
			m_color = c;
			m_autoColor = false;
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

		std::shared_ptr<Text> align(Qt::Alignment align) {
			m_alignment = align;
			return self<Text>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		QString m_text;
		QColor m_color{ 0, 0, 0 };      // 若未显式设色，将在 onThemeChanged 中根据主题覆盖
		int m_fontSize{ 14 };
		QFont::Weight m_fontWeight{ QFont::Normal };
		Qt::Alignment m_alignment{ Qt::AlignLeft };
		bool m_autoColor{ true };       // 新增：自动跟随主题
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
		QColor m_color{ 0, 0, 0 };
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