/*
 * 文件名：functional_ui_example.cpp
 * 职责：展示业务UI使用函数式写法的完整示例
 * 目的：验证UI控件可以独立设置高宽，并演示纯函数式API的优雅性
 */

#include "presentation/ui/declarative/UI.h"
#include <QDebug>
#include <memory>

using namespace UI;

namespace FunctionalUIExample {

	/// 示例1：独立的宽度和高度设置
	auto createSizeExamples() {
		return panel({
			// 标题
			text("📏 独立尺寸设置示例")
				->fontSize(18)
				->fontWeight(QFont::Bold)
				->themeColor(QColor(30, 35, 40), QColor(240, 245, 250)),

			spacer(16),

			// 只设置宽度，高度保持自然
			text("只设置宽度(300px)，高度自适应内容")
				->fontSize(14)
				->width(300)  // 只设置宽度
				->background(QColor(240, 248, 255), 6.0f)
				->padding(12),

			spacer(12),

			// 只设置高度，宽度保持自然 
			text("只设置高度(60px)，宽度自适应内容")
				->fontSize(14)
				->height(60)  // 只设置高度
				->background(QColor(255, 248, 240), 6.0f)
				->padding(12),

			spacer(12),

			// 同时设置宽度和高度
			text("同时设置宽度(250px)和高度(80px)")
				->fontSize(14)
				->width(250)
				->height(80)
				->background(QColor(248, 255, 240), 6.0f)
				->padding(12),

		})->vertical()
		  ->spacing(8)
		  ->padding(20);
	}

	/// 示例2：纯函数式业务界面
	auto createBusinessForm() {
		return panel({
			// 表单标题
			text("📝 用户信息表单")
				->fontSize(20)
				->fontWeight(QFont::Bold)
				->themeColor(QColor(20, 25, 30), QColor(250, 255, 260)),

			spacer(24),

			// 表单字段组
			panel({
				// 姓名输入区
				panel({
					text("姓名")
						->fontSize(14)
						->fontWeight(QFont::Medium)
						->themeColor(QColor(60, 70, 80), QColor(200, 210, 220)),

					spacer(8),

					// 输入框占位（实际项目中会是真正的输入组件）
					container()
						->width(300)
						->height(40)
						->background(QColor(255, 255, 255), 6.0f)
						->border(QColor(200, 210, 220), 1.0f, 6.0f)

				})->vertical()->spacing(4),

				spacer(20),

				// 邮箱输入区
				panel({
					text("邮箱地址")
						->fontSize(14)
						->fontWeight(QFont::Medium)
						->themeColor(QColor(60, 70, 80), QColor(200, 210, 220)),

					spacer(8),

					container()
						->width(300)
						->height(40)
						->background(QColor(255, 255, 255), 6.0f)
						->border(QColor(200, 210, 220), 1.0f, 6.0f)

				})->vertical()->spacing(4),

				spacer(20),

				// 按钮组
				panel({
					button("保存")
						->primary()
						->width(120)
						->height(40)
						->onTap([]() {
							qDebug() << "保存按钮被点击";
						}),

					spacer(12),

					button("取消")
						->secondary()
						->width(120)
						->height(40)
						->onTap([]() {
							qDebug() << "取消按钮被点击";
						})

				})->horizontal()
				  ->spacing(12)
				  ->crossAxisAlignment(Alignment::Center)

			})->vertical()
			  ->spacing(8)

		})->vertical()
		  ->spacing(16)
		  ->padding(24)
		  ->background(QColor(250, 252, 255), 12.0f);
	}

	/// 示例3：响应式卡片布局
	auto createResponsiveCards() {
		return panel({
			text("📊 响应式卡片展示")
				->fontSize(18)
				->fontWeight(QFont::Bold)
				->themeColor(QColor(30, 35, 40), QColor(240, 245, 250)),

			spacer(20),

			// 卡片网格 - 演示不同尺寸的卡片
			panel({
				// 第一行卡片
				panel({
					// 小卡片
					card(
						panel({
							icon(":/icons/chart.svg")
								->size(32)
								->color(QColor(59, 130, 246)),

							spacer(12),

							text("统计数据")
								->fontSize(16)
								->fontWeight(QFont::Medium),

							text("2,345")
								->fontSize(24)
								->fontWeight(QFont::Bold)
								->color(QColor(59, 130, 246))

						})->vertical()
						  ->crossAxisAlignment(Alignment::Center)
						  ->spacing(8)
						  ->padding(20)
					)->width(200)    // 固定宽度
					  ->height(160), // 固定高度

					spacer(16),

					// 中等卡片
					card(
						panel({
							text("📈 销售趋势")
								->fontSize(14)
								->fontWeight(QFont::Medium),

							spacer(8),

							text("本月销售额较上月增长 12.5%，表现优异")
								->fontSize(12)
								->wrap(true)
								->maxLines(2),

						})->vertical()
						  ->spacing(6)
						  ->padding(16)
					)->width(250),  // 只设置宽度，高度自适应

					spacer(16),

					// 动态高度卡片
					card(
						panel({
							text("📝 最新动态")
								->fontSize(14)
								->fontWeight(QFont::Medium),

							spacer(8),

							text("系统更新：新增了更灵活的布局系统，支持独立设置宽度和高度，让UI开发更加便捷。")
								->fontSize(12)
								->wrap(true)
								->maxLines(0)  // 不限制行数

						})->vertical()
						  ->spacing(6)
						  ->padding(16)
					)->width(300)  // 只设置宽度，高度根据内容自适应

				})->horizontal()
				  ->spacing(16)
				  ->crossAxisAlignment(Alignment::Start)

			})->vertical()

		})->vertical()
		  ->spacing(16)
		  ->padding(20);
	}

	/// 主示例：组合所有功能展示
	auto createMainExample() {
		return scrollView(
			panel({
				text("🎯 函数式UI框架完整示例")
					->fontSize(24)
					->fontWeight(QFont::Bold)
					->themeColor(QColor(20, 25, 30), QColor(250, 255, 260))
					->align(Qt::AlignHCenter),

				spacer(32),

				createSizeExamples(),
				spacer(32),
				createBusinessForm(),
				spacer(32),
				createResponsiveCards(),

				spacer(40)

			})->vertical()
			  ->spacing(0)
			  ->padding(20)
		);
	}

} // namespace FunctionalUIExample

// 演示函数，用于展示如何构建业务界面
void demonstrateFunctionalUI() {
	qDebug() << "🚀 开始演示函数式UI框架";

	// 创建主界面
	auto mainUI = FunctionalUIExample::createMainExample();

	// 构建UI组件（在实际应用中会渲染到屏幕）
	auto component = mainUI->build();

	if (component) {
		qDebug() << "✅ 函数式UI构建成功";
		qDebug() << "   • 演示了独立的宽度/高度设置";
		qDebug() << "   • 展示了纯函数式API的链式调用";
		qDebug() << "   • 包含了完整的业务表单示例";
		qDebug() << "   • 实现了响应式卡片布局";
	} else {
		qDebug() << "❌ UI构建失败";
	}
}

// 如果需要独立运行此示例
#ifdef STANDALONE_EXAMPLE
#include <QApplication>

int main(int argc, char *argv[]) {
	QApplication app(argc, argv);
	
	demonstrateFunctionalUI();
	
	return 0;
}
#endif