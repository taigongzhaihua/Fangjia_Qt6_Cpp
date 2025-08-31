/*
 * 文件名：declarative_nav_topbar_example.cpp
 * 职责：展示声明式NavRail和TopBar组件的使用示例
 * 依赖：NavTopBarWidgets.h、UI.h
 * 线程：仅在UI线程使用
 * 备注：演示链式API的使用模式和与现有组件的集成
 */

#include "NavTopBarWidgets.h"
#include "UI.h"
#include "nav_interface.h"

namespace UI::Examples {

	// 示例1：基本NavRail配置
	auto createBasicNavRail() {
		return navRail()
			->widths(48, 200)          // 折叠48px，展开200px
			->iconSize(22)             // 图标22px
			->itemHeight(48)           // 项目高度48px
			->labelFontPx(13);         // 标签字体13px
	}

	// 示例2：完全自定义的NavRail
	auto createCustomNavRail(fj::presentation::binding::INavDataProvider* dataProvider) {
		// 自定义色彩方案
		Ui::NavPalette customPalette;
		customPalette.railBg = QColor(30, 35, 40, 200);
		customPalette.itemHover = QColor(255, 255, 255, 20);
		customPalette.itemSelected = QColor(0, 120, 255, 180);
		customPalette.iconColor = QColor(220, 225, 230);
		customPalette.labelColor = QColor(240, 245, 250);
		customPalette.indicator = QColor(0, 120, 255, 255);

		return navRail()
			->dataProvider(dataProvider)                              // 绑定数据源
			->widths(64, 220)                                        // 更宽的尺寸
			->iconSize(24)                                           // 更大的图标
			->itemHeight(52)                                         // 更高的项目
			->labelFontPx(14)                                        // 更大的字体
			->toggleSvg(":/icons/nav_expand.svg", ":/icons/nav_collapse.svg")  // 自定义切换图标
			->palette(customPalette)                                 // 应用自定义色彩
			->padding(8)                                             // 内边距
			->margin(4, 0, 0, 0);                                    // 左边距
	}

	// 示例3：基本TopBar配置
	auto createBasicTopBar() {
		return topBar()
			->followSystem(false)                                    // 不跟随系统
			->cornerRadius(6.0f);                                    // 6px圆角
	}

	// 示例4：完全自定义的TopBar
	auto createCustomTopBar() {
		// 自定义色彩方案
		UiTopBar::Palette customPalette;
		customPalette.bg = QColor(45, 55, 70, 180);
		customPalette.bgHover = QColor(60, 72, 88, 200);
		customPalette.bgPressed = QColor(50, 62, 78, 220);
		customPalette.icon = QColor(240, 245, 250);

		return topBar()
			->followSystem(true, true)                               // 跟随系统，启用动画
			->cornerRadius(8.0f)                                     // 8px圆角
			->svgTheme(":/icons/sun_custom.svg", ":/icons/moon_custom.svg")  // 自定义主题图标
			->svgFollow(":/icons/follow_on_custom.svg", ":/icons/follow_off_custom.svg")  // 自定义跟随图标
			->svgSystem(":/icons/min_custom.svg", ":/icons/max_custom.svg", ":/icons/close_custom.svg")  // 自定义系统按钮
			->palette(customPalette)                                 // 应用自定义色彩
			->onThemeToggle([]() {                                   // 主题切换回调
				// 在这里处理主题切换逻辑
				// 例如：通知其他组件、保存用户偏好等
			})
			->padding(4, 8)                                          // 内边距
			->background(QColor(0, 0, 0, 50), 12.0f);               // 半透明背景
	}

	// 示例5：在AppShell中集成NavRail和TopBar
	auto createAppWithNavAndTopBar(fj::presentation::binding::INavDataProvider* navProvider) {
		return appShell()
			->navRail(createCustomNavRail(navProvider))              // 设置导航栏
			->topBar(createCustomTopBar())                           // 设置顶栏
			->content(                                               // 主内容区域
				panel({
					text("Welcome to the application!")
						->fontSize(18)
						->themeColor(QColor(50, 55, 60), QColor(240, 245, 250)),
					spacer(20),
					text("This demonstrates the new declarative NavRail and TopBar widgets.")
						->fontSize(14)
						->wrap(true)
						->maxLines(3)
				})
				->padding(24)
			);
	}

	// 示例6：响应式布局的NavRail
	auto createResponsiveNavRail(bool isCompact) {
		if (isCompact) {
			return navRail()
				->widths(44, 180)                                    // 紧凑尺寸
				->iconSize(20)                                       // 小图标
				->itemHeight(44)                                     // 紧凑高度
				->labelFontPx(12);                                   // 小字体
		} else {
			return navRail()
				->widths(64, 240)                                    // 宽松尺寸
				->iconSize(24)                                       // 大图标
				->itemHeight(56)                                     // 宽松高度
				->labelFontPx(14);                                   // 标准字体
		}
	}

} // namespace UI::Examples