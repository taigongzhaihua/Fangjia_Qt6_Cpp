/*
 * 文件名：BasicWidgets_Button.h
 * 职责：声明式按钮组件，提供流式API构建可重用的按钮控件。
 * 依赖：Widget基类、UiPushButton运行时组件。
 * 线程：仅在UI线程使用。
 * 备注：采用流式API设计，支持链式调用配置按钮属性。
 */

#pragma once
#include "Widget.h"
#include "UiPushButton.h"
#include <functional>
#include <qstring.h>

namespace UI {

	/// 声明式按钮组件：支持文本、图标、变体和交互的流式配置
	/// 
	/// 功能特性：
	/// - 流式API：button("保存")->primary()->onTap(callback)
	/// - 多种变体：Primary（主要）、Secondary（次要）、Ghost（幽灵）
	/// - 灵活尺寸：S（小）、M（中）、L（大）
	/// - 图标支持：单路径或主题相关双路径
	/// - 完整交互：点击回调、禁用状态
	/// - 可定制性：圆角半径、内边距覆盖
	/// 
	/// 使用示例：
	/// auto saveBtn = button("保存")->primary()->size(Size::M)->onTap([]{ save(); });
	/// auto iconBtn = button("")->icon(":/icons/add.svg")->secondary();
	/// auto themeBtn = button("主题")->iconTheme(":/light.svg", ":/dark.svg")->ghost();
	class Button : public Widget {
	public:
		/// 按钮变体枚举（映射到UiPushButton::Variant）
		enum class Variant {
			Primary,     // 主要按钮：高对比背景色，用于主要操作
			Secondary,   // 次要按钮：较低对比背景色，用于辅助操作
			Ghost,       // 幽灵按钮：透明背景，仅边框/文字，用于轻量操作
			Destructive  // 破坏性按钮：警告色背景，用于删除等危险操作
		};

		/// 按钮尺寸枚举（映射到UiPushButton::Size）
		enum class Size {
			S,  // 小尺寸：紧凑界面使用
			M,  // 中等尺寸：标准场景
			L   // 大尺寸：重要操作强调
		};

		explicit Button(QString text) : m_text(std::move(text)) {}

		/// 功能：设置按钮变体为主要样式
		/// 返回：当前按钮实例（支持链式调用）
		/// 说明：主要按钮通常用于确认、提交等重要操作
		std::shared_ptr<Button> primary() {
			m_variant = Variant::Primary;
			return self<Button>();
		}

		/// 功能：设置按钮变体为次要样式
		/// 返回：当前按钮实例（支持链式调用）
		/// 说明：次要按钮用于取消、重置等辅助操作
		std::shared_ptr<Button> secondary() {
			m_variant = Variant::Secondary;
			return self<Button>();
		}

		/// 功能：设置按钮变体为幽灵样式
		/// 返回：当前按钮实例（支持链式调用）
		/// 说明：幽灵按钮用于链接式操作，视觉重量最轻
		std::shared_ptr<Button> ghost() {
			m_variant = Variant::Ghost;
			return self<Button>();
		}

		/// 功能：设置按钮变体为破坏性样式
		/// 返回：当前按钮实例（支持链式调用）
		/// 说明：破坏性按钮用于删除、清空等危险操作，使用警告色
		std::shared_ptr<Button> destructive() {
			m_variant = Variant::Destructive;
			return self<Button>();
		}

		/// 功能：设置按钮尺寸
		/// 参数：s — 尺寸枚举值
		/// 返回：当前按钮实例（支持链式调用）
		std::shared_ptr<Button> size(Size s) {
			m_size = s;
			return self<Button>();
		}

		/// 功能：设置单个图标路径
		/// 参数：path — SVG图标文件路径
		/// 返回：当前按钮实例（支持链式调用）
		/// 说明：设置后将显示图标，可与文本共存
		std::shared_ptr<Button> icon(const QString& path) {
			m_iconPath = path;
			m_useThemeIcons = false;
			return self<Button>();
		}

		/// 功能：设置主题相关的图标路径
		/// 参数：lightPath — 浅色主题使用的图标路径
		/// 参数：darkPath — 深色主题使用的图标路径
		/// 返回：当前按钮实例（支持链式调用）
		/// 说明：系统会根据当前主题自动选择合适的图标
		std::shared_ptr<Button> iconTheme(const QString& lightPath, const QString& darkPath) {
			m_iconLightPath = lightPath;
			m_iconDarkPath = darkPath;
			m_useThemeIcons = true;
			return self<Button>();
		}

		/// 功能：设置圆角半径
		/// 参数：radius — 圆角半径值（逻辑像素）
		/// 返回：当前按钮实例（支持链式调用）
		std::shared_ptr<Button> cornerRadius(float radius) {
			m_cornerRadius = radius;
			return self<Button>();
		}

		/// 功能：设置自定义内边距
		/// 参数：padding — 内边距设置
		/// 返回：当前按钮实例（支持链式调用）
		/// 说明：覆盖尺寸预设的默认内边距
		std::shared_ptr<Button> padding(const QMargins& padding) {
			m_padding = padding;
			m_useCustomPadding = true;
			return self<Button>();
		}

		/// 功能：设置禁用状态
		/// 参数：disabled — 是否禁用（默认true）
		/// 返回：当前按钮实例（支持链式调用）
		/// 说明：禁用后按钮变灰且不响应交互
		std::shared_ptr<Button> disabled(bool disabled = true) {
			m_disabled = disabled;
			return self<Button>();
		}

		/// 功能：设置点击回调函数
		/// 参数：callback — 点击时执行的函数
		/// 返回：当前按钮实例（支持链式调用）
		std::shared_ptr<Button> onTap(std::function<void()> callback) {
			m_onTap = std::move(callback);
			return self<Button>();
		}

	protected:
		/// 功能：创建运行时组件实例
		/// 返回：配置好的UiPushButton实例
		/// 说明：将声明式配置转换为实际的运行时组件
		std::unique_ptr<IUiComponent> build() const override;

	private:
		// === 配置属性 ===
		QString m_text;
		Variant m_variant{ Variant::Primary };
		Size m_size{ Size::M };
		
		QString m_iconPath;
		QString m_iconLightPath;
		QString m_iconDarkPath;
		bool m_useThemeIcons{ false };
		
		float m_cornerRadius{ 8.0f };
		QMargins m_padding;
		bool m_useCustomPadding{ false };
		bool m_disabled{ false };
		
		std::function<void()> m_onTap;
	};

	/// 功能：创建按钮组件的工厂函数
	/// 参数：text — 按钮显示文本
	/// 返回：可进行链式配置的按钮实例
	/// 说明：这是创建按钮的推荐入口点
	inline std::shared_ptr<Button> button(const QString& text) {
		return std::make_shared<Button>(text);
	}

} // namespace UI