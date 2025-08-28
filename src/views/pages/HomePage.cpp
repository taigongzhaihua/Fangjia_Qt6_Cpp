#include "HomePage.h"
#include <AdvancedWidgets.h>
#include <BasicWidgets.h>
#include <Layouts.h>
#include <Binding.h>  // 新增：引入绑定功能
#include <memory>
#include <qcolor.h>
#include <qlogging.h>

#include "UI.h"
#include <exception>
#include <qfont.h>
#include <qstring.h>
#include <UiComponent.hpp>
#include <Widget.h>
using namespace UI;

// CounterViewModel 实现
CounterViewModel::CounterViewModel(QObject* parent) : QObject(parent) {}

void CounterViewModel::increment()
{
    m_count++;
    emit countChanged();
}

void CounterViewModel::decrement()
{
    if (m_count > 0) {
        m_count--;
        emit countChanged();
    }
}

class HomePage::Impl
{
public:
	bool isDark = false;
	std::unique_ptr<IUiComponent> builtComponent;
	std::unique_ptr<CounterViewModel> counterVM;  // 新增：计数器ViewModel

	Impl() {
		counterVM = std::make_unique<CounterViewModel>();
	}

	[[nodiscard]] WidgetPtr buildUI() const
	{
		auto main = panel({
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
				  ->add(buildFeatureCard(":/icons/explore_light.svg", ":/icons/explore_dark.svg", "探索发现", "发现新的方剂组合"),
						0, 1, 1, 1, Grid::Center, Grid::Center)
				  ->add(buildFeatureCard(":/icons/fav_light.svg", ":/icons/fav_dark.svg", "我的收藏", "管理收藏的方剂"),
						1, 0, 1, 1, Grid::Center, Grid::Center)
				  ->add(buildFeatureCard(":/icons/settings_light.svg", ":/icons/settings_dark.svg", "系统设置", "自定义应用偏好"),
						1, 1, 1, 1, Grid::Center, Grid::Center),

			// 留白
			spacer(8)
			})->vertical()
			->crossAxisAlignment(Alignment::Center)
			->spacing(20);

		return main;
	}

private:
	// 新增：构建绑定演示区域
	[[nodiscard]] WidgetPtr buildBindingDemo() const
	{
		return card(panel({
			text("声明式绑定演示")->fontSize(18)->fontWeight(QFont::Medium),
			spacer(10),
			
			// 使用 bindingHost 创建可重建的内容区域
			bindingHost([this]() -> WidgetPtr {
				// 这个lambda会在每次计数器变化时重新执行
				return panel({
					text(QString("当前计数: %1").arg(counterVM->count()))
						->fontSize(16)
						->themeColor(QColor(50, 100, 150), QColor(200, 220, 255)),
					spacer(5),
					text(counterVM->count() % 2 == 0 ? "偶数 ✨" : "奇数 🔥")
						->fontSize(14)
						->themeColor(QColor(100, 150, 100), QColor(150, 255, 150))
				})->vertical()->crossAxisAlignment(Alignment::Center);
			})->connect([this](UI::RebuildHost* host) {
				// 连接器：当 counterVM 的 count 变化时，自动重建UI
				UI::observe(counterVM.get(), &CounterViewModel::countChanged, [host]() {
					host->requestRebuild();
				});
			}),

			spacer(10),
			
			// 按钮区域（不使用绑定，演示混合用法）
			panel({
				text("递增")->fontSize(14)
					->onTap([this]() { counterVM->increment(); })
					->padding(8, 4)
					->background(QColor(100, 160, 220), 4.0f),
				spacer(10),
				text("递减")->fontSize(14)
					->onTap([this]() { counterVM->decrement(); })
					->padding(8, 4)
					->background(QColor(220, 100, 100), 4.0f)
			})->horizontal()->crossAxisAlignment(Alignment::Center),

			spacer(5),
			text("点击按钮观察绑定效果 - UI会自动重建")->fontSize(12)
				->themeColor(QColor(120, 120, 120), QColor(160, 160, 160))
				->align(Qt::AlignCenter)
				
		})->vertical()->crossAxisAlignment(Alignment::Center)->padding(15))
		->elevation(1.0f)
		->backgroundTheme(QColor(250, 250, 255), QColor(20, 25, 35));
	}
	[[nodiscard]] WidgetPtr buildFeatureCard(const QString& iconLight, const QString& iconDark, const QString& title, const QString& desc) const
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
			setContent(m_impl->builtComponent.get());
		}
	}
	catch (const std::exception& e)
	{
		qCritical() << "Exception in initializeContent:" << e.what();
		throw;
	}
}

void HomePage::applyPageTheme(bool isDark)
{
	m_impl->isDark = isDark;
	if (m_impl->builtComponent) m_impl->builtComponent->onThemeChanged(isDark);
}

// CounterViewModel 实现已在此文件中，需要包含 MOC
#include "HomePage.moc"
