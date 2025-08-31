/*
 * 文件名：NavTopBarWidgets.h
 * 职责：声明式UI框架的导航栏和顶栏组件定义，提供链式API配置
 * 依赖：Widget基类、运行时NavRail和TopBar组件
 * 线程：仅在UI线程使用
 * 备注：包装现有运行时组件，提供声明式接口，保持行为和样式一致性
 */

#pragma once
#include "Widget.h"
#include "UiNav.h"
#include "UiTopBar.h"
#include "nav_interface.h"
#include <functional>
#include <memory>
#include <qstring.h>

namespace UI {

	/// 声明式导航栏组件：包装Ui::NavRail，提供链式配置API
	/// 
	/// 功能特性：
	/// - 链式API配置导航栏属性
	/// - 自动转发生命周期事件和主题变化
	/// - 与现有Ui::NavRail保持行为一致
	/// - 支持数据绑定和样式自定义
	class NavRail : public Widget {
	public:
		NavRail() = default;

		/// 功能：绑定导航数据提供者
		/// 参数：provider - 导航数据提供者指针
		/// 返回：当前NavRail实例（支持链式调用）
		std::shared_ptr<NavRail> dataProvider(fj::presentation::binding::INavDataProvider* provider) {
			m_dataProvider = provider;
			return self<NavRail>();
		}

		/// 功能：设置导航栏宽度（折叠/展开状态）
		/// 参数：collapsed - 折叠状态宽度（默认48）
		/// 参数：expanded - 展开状态宽度（默认200）
		/// 返回：当前NavRail实例（支持链式调用）
		std::shared_ptr<NavRail> widths(int collapsed, int expanded) {
			m_collapsedWidth = collapsed;
			m_expandedWidth = expanded;
			return self<NavRail>();
		}

		/// 功能：设置图标大小
		/// 参数：logicalPx - 图标逻辑像素大小（默认22）
		/// 返回：当前NavRail实例（支持链式调用）
		std::shared_ptr<NavRail> iconSize(int logicalPx) {
			m_iconSize = logicalPx;
			return self<NavRail>();
		}

		/// 功能：设置导航项高度
		/// 参数：px - 单个导航项的高度（默认48）
		/// 返回：当前NavRail实例（支持链式调用）
		std::shared_ptr<NavRail> itemHeight(int px) {
			m_itemHeight = px;
			return self<NavRail>();
		}

		/// 功能：设置标签字体大小
		/// 参数：px - 标签字体像素大小（默认13）
		/// 返回：当前NavRail实例（支持链式调用）
		std::shared_ptr<NavRail> labelFontPx(int px) {
			m_labelFontPx = px;
			return self<NavRail>();
		}

		/// 功能：设置切换按钮SVG图标
		/// 参数：expand - 展开状态图标路径
		/// 参数：collapse - 折叠状态图标路径
		/// 返回：当前NavRail实例（支持链式调用）
		std::shared_ptr<NavRail> toggleSvg(QString expand, QString collapse) {
			m_expandSvg = std::move(expand);
			m_collapseSvg = std::move(collapse);
			return self<NavRail>();
		}

		/// 功能：设置导航栏色彩方案
		/// 参数：pal - 导航栏色彩配置
		/// 返回：当前NavRail实例（支持链式调用）
		std::shared_ptr<NavRail> palette(const Ui::NavPalette& pal) {
			m_palette = pal;
			m_hasCustomPalette = true;
			return self<NavRail>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		fj::presentation::binding::INavDataProvider* m_dataProvider{ nullptr };
		int m_collapsedWidth{ 48 };
		int m_expandedWidth{ 200 };
		int m_iconSize{ 22 };
		int m_itemHeight{ 48 };
		int m_labelFontPx{ 13 };
		QString m_expandSvg;
		QString m_collapseSvg;
		Ui::NavPalette m_palette;
		bool m_hasCustomPalette{ false };
	};

	/// 声明式顶栏组件：包装UiTopBar，提供链式配置API
	/// 
	/// 功能特性：
	/// - 链式API配置顶栏属性
	/// - 自动转发生命周期事件和主题变化
	/// - 与现有UiTopBar保持行为一致
	/// - 支持主题切换回调和样式自定义
	class TopBar : public Widget {
	public:
		TopBar() = default;

		/// 功能：设置跟随系统主题状态
		/// 参数：on - 是否跟随系统主题
		/// 参数：animate - 是否启用动画（默认false）
		/// 返回：当前TopBar实例（支持链式调用）
		std::shared_ptr<TopBar> followSystem(bool on, bool animate = false) {
			m_followSystem = on;
			m_animateFollow = animate;
			return self<TopBar>();
		}

		/// 功能：设置按钮圆角半径
		/// 参数：r - 圆角半径
		/// 返回：当前TopBar实例（支持链式调用）
		std::shared_ptr<TopBar> cornerRadius(float r) {
			m_cornerRadius = r;
			return self<TopBar>();
		}

		/// 功能：设置主题切换按钮图标
		/// 参数：sunWhenDark - 暗色主题时显示的图标（通常为太阳）
		/// 参数：moonWhenLight - 亮色主题时显示的图标（通常为月亮）
		/// 返回：当前TopBar实例（支持链式调用）
		std::shared_ptr<TopBar> svgTheme(QString sunWhenDark, QString moonWhenLight) {
			m_svgThemeDark = std::move(sunWhenDark);
			m_svgThemeLight = std::move(moonWhenLight);
			return self<TopBar>();
		}

		/// 功能：设置跟随系统按钮图标
		/// 参数：on - 启用跟随时的图标
		/// 参数：off - 禁用跟随时的图标
		/// 返回：当前TopBar实例（支持链式调用）
		std::shared_ptr<TopBar> svgFollow(QString on, QString off) {
			m_svgFollowOn = std::move(on);
			m_svgFollowOff = std::move(off);
			return self<TopBar>();
		}

		/// 功能：设置系统窗口控制按钮图标
		/// 参数：min - 最小化按钮图标
		/// 参数：max - 最大化按钮图标
		/// 参数：close - 关闭按钮图标
		/// 返回：当前TopBar实例（支持链式调用）
		std::shared_ptr<TopBar> svgSystem(QString min, QString max, QString close) {
			m_svgMin = std::move(min);
			m_svgMax = std::move(max);
			m_svgClose = std::move(close);
			return self<TopBar>();
		}

		/// 功能：设置顶栏色彩方案
		/// 参数：pal - 顶栏色彩配置
		/// 返回：当前TopBar实例（支持链式调用）
		std::shared_ptr<TopBar> palette(const UiTopBar::Palette& pal) {
			m_palette = pal;
			m_hasCustomPalette = true;
			return self<TopBar>();
		}

		/// 功能：设置主题切换回调函数
		/// 参数：callback - 主题切换时的回调函数
		/// 返回：当前TopBar实例（支持链式调用）
		std::shared_ptr<TopBar> onThemeToggle(std::function<void()> callback) {
			m_themeToggleCallback = std::move(callback);
			return self<TopBar>();
		}

		std::unique_ptr<IUiComponent> build() const override;

	private:
		bool m_followSystem{ false };
		bool m_animateFollow{ false };
		float m_cornerRadius{ 6.0f };
		QString m_svgThemeDark;
		QString m_svgThemeLight;
		QString m_svgFollowOn;
		QString m_svgFollowOff;
		QString m_svgMin;
		QString m_svgMax;
		QString m_svgClose;
		UiTopBar::Palette m_palette;
		bool m_hasCustomPalette{ false };
		std::function<void()> m_themeToggleCallback;
	};

} // namespace UI