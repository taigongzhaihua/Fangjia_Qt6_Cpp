#include "HomePage.h"
#include <AdvancedWidgets.h>
#include <BasicWidgets.h>
#include <Layouts.h>
#include <memory>
#include <qcolor.h>
#include <qfont.h>
#include <qlogging.h>

#include "UI.h"
#include <exception>
#include <qstring.h>
#include <UiComponent.hpp>
#include <Widget.h>
using namespace UI;

class HomePage::Impl
{
public:
	bool isDark = false;
	std::unique_ptr<IUiComponent> builtComponent;

	[[nodiscard]] WidgetPtr buildUI() const
	{
		auto main = panel({
			// 欢迎标题
			text("欢迎使用方家")->fontSize(28),

			// 副标题
			text("中医方剂数据管理系统")->fontSize(16),

			// 功能卡片网格
			panel({
				// 第一行
				panel({
					buildFeatureCard(":/icons/data_light.svg", "方剂数据", "查看和管理中医方剂"),
					spacer(16),
					buildFeatureCard(":/icons/explore_light.svg", "探索发现", "发现新的方剂组合")
				})->horizontal()
				  ->crossAxisAlignment(Alignment::Stretch),

				spacer(16),

					// 第二行
					panel({
						buildFeatureCard(":/icons/fav_light.svg", "我的收藏", "管理收藏的方剂"),
						spacer(16),
						buildFeatureCard(":/icons/settings_light.svg", "系统设置", "自定义应用偏好")
					})->horizontal()
					  ->crossAxisAlignment(Alignment::Stretch)
				}),

			// 留白
			spacer(8)
			})->vertical()
			->crossAxisAlignment(Alignment::Center)
			->spacing(15);

		return main;
	}

private:
	[[nodiscard]] WidgetPtr buildFeatureCard(const QString& iconPath, const QString& title, const QString& desc) const
	{
		return
			card(panel({
					icon(iconPath)->size(48)
								  ->color(isDark
											  ? QColor(100, 160, 220)
											  : QColor(60, 120, 180)),
					spacer(16),
					text(title)->fontSize(16)
							   ->fontWeight(QFont::DemiBold)
							   ->color(isDark
										   ? QColor(240, 245, 250)
										   : QColor(30, 35, 40)),
					spacer(8),
					text(desc)->fontSize(13)
							  ->color(isDark
										  ? QColor(180, 190, 200)
										  : QColor(100, 110, 120))
				})->vertical()
				->crossAxisAlignment(Alignment::Center)
				->spacing(10)
				->size(200, 180)
			)->elevation(2.0f)
			->border(QColor(0, 0, 0, 40), 1.0f, 8.0f);
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
