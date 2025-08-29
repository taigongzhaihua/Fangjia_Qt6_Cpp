/*
 * 文件名：BasicWidgets.h
 * 职责：声明式UI框架的基础组件定义，包括文本和图标组件。
 * 依赖：Widget基类、布局系统。
 * 线程：仅在UI线程使用。
 * 备注：采用流式API设计，支持链式调用配置组件属性，自动主题适配。
 */

#pragma once
#include "Widget.h"
#include <qfont.h>
#include <qstring.h>

#include "Layouts.h"

namespace UI {

	/// 文本组件：支持多行、自动换行、溢出处理和主题色适配
	/// 
	/// 功能特性：
	/// - 灵活的颜色配置（手动指定、主题自适应、系统默认）
	/// - 文本溢出处理（可见、剪裁、省略号）
	/// - 自动换行与最大行数限制
	/// - 字体属性完整控制（大小、粗细、对齐）
	/// 
	/// 使用示例：
	/// auto label = text("Hello")->fontSize(14)->color(Qt::blue);
	/// auto themed = text("Title")->themeColor(Qt::black, Qt::white);
	class Text : public Widget {
	public:
		/// 溢出处理策略
		enum class Overflow {
			Visible,  // 超出边界继续绘制（可能造成重叠）
			Clip,     // 按容器边界剪裁
			Ellipsis  // 超出部分显示省略号
		};

		explicit Text(QString text) : m_text(std::move(text)) {}

		/// 功能：设置固定颜色
		/// 参数：c — 文本颜色
		/// 返回：当前文本组件实例（支持链式调用）
		/// 说明：一旦调用则不再跟随主题自动变色
		std::shared_ptr<Text> color(const QColor c) {
			m_color = c;
			m_autoColor = false;
			m_useThemeColor = false; // 显式设色优先，取消主题色
			return self<Text>();
		}

		/// 功能：设置主题自适应颜色
		/// 参数：light — 亮色主题下的文本颜色
		/// 参数：dark — 暗色主题下的文本颜色
		/// 返回：当前文本组件实例（支持链式调用）
		/// 说明：随主题变化自动切换颜色，优先级高于默认自动配色
		std::shared_ptr<Text> themeColor(const QColor light, const QColor dark) {
			m_colorLight = light;
			m_colorDark = dark;
			m_useThemeColor = true;
			m_autoColor = false; // 主题色优先，关闭默认自动配色
			return self<Text>();
		}

		/// 功能：设置字体大小
		/// 参数：size — 字体大小（逻辑像素）
		/// 返回：当前文本组件实例（支持链式调用）
		std::shared_ptr<Text> fontSize(const int size) {
			m_fontSize = size;
			return self<Text>();
		}

		/// 功能：设置字体粗细
		/// 参数：weight — 字体粗细枚举值
		/// 返回：当前文本组件实例（支持链式调用）
		std::shared_ptr<Text> fontWeight(const QFont::Weight weight) {
			m_fontWeight = weight;
			return self<Text>();
		}

		/// 功能：设置文本对齐方式
		/// 参数：align — 对齐方式（水平+垂直组合）
		/// 返回：当前文本组件实例（支持链式调用）
		/// 说明：控制文本在自身矩形内的对齐位置
		std::shared_ptr<Text> align(const Qt::Alignment align) {
			m_alignment = align;
			return self<Text>();
		}

		// 开启自动换行（默认 false）
		std::shared_ptr<Text> wrap(const bool on = true) {
			m_wrap = on;
			// 默认多行时不限行数（0 表示不限），单行时限定为 1
			if (on && m_maxLines == 1) m_maxLines = 0;
			if (!on && m_maxLines == 0) m_maxLines = 1;
			return self<Text>();
		}

		// 最大行数（0 表示不限）
		std::shared_ptr<Text> maxLines(const int n) {
			m_maxLines = std::max(0, n);
			return self<Text>();
		}

		// 溢出处理：Visible / Clip / Ellipsis
		std::shared_ptr<Text> overflow(const Overflow o) {
			m_overflow = o;
			return self<Text>();
		}

		// 单词优先换行（默认 true），false 时按字符断行
		std::shared_ptr<Text> wordWrap(const bool on) {
			m_wordWrap = on;
			return self<Text>();
		}

		// 行距（像素，逻辑像素；负数表示使用默认行距）
		std::shared_ptr<Text> lineSpacing(const int px) {
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

		// 显式设色：一旦调用则不再跟随主题自动变色
		std::shared_ptr<Icon> color(const QColor c) {
			m_color = c;
			m_autoColor = false;
			return self<Icon>();
		}

		std::shared_ptr<Icon> size(const int s) {
			m_size = s;
			return self<Icon>();
		}

		// 新增：设置按主题切换的两套 SVG 路径（浅色=linear，深色=fill）
		std::shared_ptr<Icon> themePaths(QString lightPath, QString darkPath) {
			m_lightPath = std::move(lightPath);
			m_darkPath = std::move(darkPath);
			m_useThemePaths = true;
			return self<Icon>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		QString m_path;
		QColor m_color;
		int m_size{ 24 };
		bool m_autoColor{ true }; // 主题自动着色

		// 新增：主题路径
		bool m_useThemePaths{ false };
		QString m_lightPath;
		QString m_darkPath;
	};

	// 容器组件
	class Container : public Widget {
	public:
		explicit Container(WidgetPtr child = nullptr) : m_child(std::move(child)) {}

		std::shared_ptr<Container> child(WidgetPtr c) {
			m_child = std::move(c);
			return self<Container>();
		}

		std::shared_ptr<Container> alignment(const Alignment align) {
			m_alignment = align;
			return self<Container>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		WidgetPtr m_child;
		Alignment m_alignment{ Alignment::Center };
	};

} // namespace UI