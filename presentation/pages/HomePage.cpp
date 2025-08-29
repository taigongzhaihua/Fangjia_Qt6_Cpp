#include "HomePage.h"
#include <AdvancedWidgets.h>
#include <BasicWidgets.h>
#include <Binding.h>
#include <Layouts.h>
#include <memory>
#include <qcolor.h>
#include <qlogging.h>

#include "UI.h"
#include <exception>
#include <qfont.h>
#include <qnamespace.h>
#include <qobject.h>
#include <qstring.h>
#include <qtmetamacros.h>
#include <RebuildHost.h>
#include <UiComponent.hpp>
#include <Widget.h>
using namespace UI;

// CounterViewModel 实现
CounterViewModel::CounterViewModel(QObject* parent) : QObject(parent)
{
}

void CounterViewModel::increment()
{
	m_count++;
	emit countChanged();
}

void CounterViewModel::decrement()
{
	if (m_count > 0)
	{
		m_count--;
		emit countChanged();
	}
}

class HomePage::Impl
{
public:
	bool isDark = false;
	std::unique_ptr<IUiComponent> builtComponent;
	std::unique_ptr<CounterViewModel> counterVM; // 计数器ViewModel

	Impl()
	{
		counterVM = std::make_unique<CounterViewModel>();
	}

	[[nodiscard]] WidgetPtr buildUI() const
	{
		const auto mainContent = panel({
			// 欢迎标题
			text("欢迎使用方家")->fontSize(28),

			// 副标题
			text("中医方剂数据管理系统")->fontSize(16),

			spacer(15),

			// 新增：声明式绑定演示区域
			buildBindingDemo(),

			spacer(15),

			// 功能卡片网格
			grid()->columns({AUTO, AUTO})
				  ->rows({AUTO, AUTO})
				  ->colSpacing(32)
				  ->rowSpacing(32)
				  ->add(buildFeatureCard(":/icons/data_light.svg", ":/icons/data_dark.svg", "方剂数据", "查看和管理中医方剂"),
						0, 0, 1, 1, Grid::Center, Grid::Center)
				  ->add(buildFeatureCard(":/icons/explore_light.svg", ":/icons/explore_dark.svg", "探索发现",
										 "发现新的方剂组合"),
						0, 1, 1, 1, Grid::Center, Grid::Center)
				  ->add(buildFeatureCard(":/icons/fav_light.svg", ":/icons/fav_dark.svg", "我的收藏", "管理收藏的方剂"),
						1, 0, 1, 1, Grid::Center, Grid::Center)
				  ->add(buildFeatureCard(":/icons/settings_light.svg", ":/icons/settings_dark.svg", "系统设置",
										 "自定义应用偏好"),
						1, 1, 1, 1, Grid::Center, Grid::Center),

			// 留白
			spacer(8)
			})->vertical()
			->crossAxisAlignment(Alignment::Center)
			->spacing(20);

		// 使用声明式 ScrollView 包装主内容
		return scrollView(mainContent);
	}

private:
	// 新增：构建绑定演示区域
	[[nodiscard]] WidgetPtr buildBindingDemo() const
	{
		return card(panel({
					   text("声明式绑定演示")
					   ->fontSize(18)->fontWeight(QFont::Medium)
					   ->align(Qt::AlignHCenter),
					   spacer(10),

			// 使用 bindingHost 创建可重建的内容区域
			bindingHost([this]() -> WidgetPtr
			{
					// 这个lambda会在每次计数器变化时重新执行
					return panel({
							text(
								QString("当前计数: %1")
								.arg(counterVM->count())
							)->fontSize(16)
							 ->themeColor(QColor(50, 100, 150), QColor(200, 220, 255))
							 ->align(Qt::AlignHCenter),
							spacer(5),
							text(counterVM->count() % 2 == 0
									 ? "偶数 ✨"
									 : "奇数 🔥"
							)->fontSize(14)
							 ->themeColor(QColor(100, 150, 100), QColor(150, 255, 150))
							 ->align(Qt::AlignHCenter)
						})->vertical()
						  ->crossAxisAlignment(Alignment::Stretch);
				})->connect([this](UI::RebuildHost* host)
				{
						// 连接器：当 counterVM 的 count 变化时，自动重建UI
						UI::observe(counterVM.get(), &CounterViewModel::countChanged, [host]
						{
							host->requestRebuild();
						});
					}),

					spacer(10),

					// 按钮区域（不使用绑定，演示混合用法）
					grid()->columns({1_fr, 1_fr})
						  ->add(text("递增")
								->fontSize(14)
								->align(Qt::AlignHCenter)
								->onTap([this] { counterVM->increment(); })
								->padding(8, 4)
								->background(QColor(100, 160, 220), 4.0f),
								0, 0)
						  ->add(text("递减")
								->fontSize(14)
								->align(Qt::AlignHCenter)
								->onTap([this] { counterVM->decrement(); })
								->padding(8, 4)
								->background(QColor(220, 100, 100), 4.0f),
								0, 1)
						  ->colSpacing(0)
						  ->size(120, 30),

					spacer(5),
					text("点击按钮观察绑定效果 - UI会自动重建")
					->fontSize(12)
					->themeColor(QColor(120, 120, 120), QColor(160, 160, 160))
					->align(Qt::AlignCenter)
			})->vertical()
					->crossAxisAlignment(Alignment::Stretch)
					->padding(15))
			->elevation(1.0f)
			->backgroundTheme(QColor(250, 250, 255), QColor(20, 25, 35));
	}

	[[nodiscard]] WidgetPtr buildFeatureCard(const QString& iconLight, const QString& iconDark, const QString& title,
		const QString& desc) const
	{
		// 注意：将 size(200,180) 施加在 card 外层，而不是内部 panel 上
		return
			card(panel({
					icon(iconLight)->themePaths(iconLight, iconDark)
								   ->size(48)
								   ->color(isDark ? QColor(100, 160, 220) : QColor(60, 120, 180)),
					spacer(8),
					text(title)->fontSize(16)
							   ->fontWeight(QFont::Medium)
							   ->themeColor(QColor(30, 35, 40), QColor(210, 220, 215)),
					text(desc)->fontSize(13)
							  ->themeColor(QColor(100, 110, 120), QColor(150, 160, 155))
				})->vertical()
				->crossAxisAlignment(Alignment::Center)
				->spacing(10)
				->size(200, 140)
				->padding(10)
			)->elevation(2.0f)
			->backgroundTheme(QColor(240, 245, 255), QColor(10, 15, 25)); // 固定“卡片外层”尺寸，保证大小稳定
	}
};

HomePage::HomePage() : m_impl(std::make_unique<Impl>())
{
	try
	{
		setTitle("首页");
		HomePage::initializeContent();
	}
	catch (const std::exception& e)
	{
		qCritical() << "Exception in HomePage:" << e.what();
		throw;
	}
}

HomePage::~HomePage() = default;

void HomePage::initializeContent()
{
	try
	{
		if (const auto widget = m_impl->buildUI())
		{
			m_impl->builtComponent = widget->build();

			// 直接将声明式构建的组件（已包含 ScrollView）设置为页面内容
			setContent(m_impl->builtComponent.get());
		}
	}
	catch (const std::exception& e)
	{
		qCritical() << "Exception in initializeContent:" << e.what();
		throw;
	}
}

void HomePage::applyPageTheme(const bool isDark)
{
	m_impl->isDark = isDark;
	// 主题变化通过 UiPage/UiRoot 自动传播，无需手动处理
}

void HomePage::onAppear()
{
	qDebug() << "HomePage: onAppear() - 页面显示，可在此进行资源加载或埋点";
}

void HomePage::onDisappear()
{
	qDebug() << "HomePage: onDisappear() - 页面隐藏，可在此进行资源释放";
}

// CounterViewModel 实现已在此文件中，需要包含 MOC
