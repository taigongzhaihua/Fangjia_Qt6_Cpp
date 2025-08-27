#include "HomePage.h"
#include <AdvancedWidgets.h>
#include <BasicWidgets.h>
#include <Layouts.h>
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

			spacer(15),
			// 功能卡片网格
			grid()->columns({Grid::Star(1), Grid::Star(1)})
				  ->rows({Grid::Auto(), Grid::Auto()})
				  ->colSpacing(16)
				  ->rowSpacing(16)
				  ->add(buildFeatureCard(":/icons/data_light.svg", "方剂数据", "查看和管理中医方剂"),
						0, 0, 1, 1, Grid::CellAlign::Center, Grid::CellAlign::Center)
				  ->add(buildFeatureCard(":/icons/explore_light.svg", "探索发现", "发现新的方剂组合"),
						0, 1, 1, 1, Grid::CellAlign::Center, Grid::CellAlign::Center)
				  ->add(buildFeatureCard(":/icons/fav_light.svg", "我的收藏", "管理收藏的方剂"),
						1, 0, 1, 1, Grid::CellAlign::Center, Grid::CellAlign::Center)
				  ->add(buildFeatureCard(":/icons/settings_light.svg", "系统设置", "自定义应用偏好"),
						1, 1, 1, 1, Grid::CellAlign::Center, Grid::CellAlign::Center),

			// 留白
			spacer(8)
			})->vertical()
			->crossAxisAlignment(Alignment::Center)
			->spacing(20);

		return main;
	}

private:
	[[nodiscard]] WidgetPtr buildFeatureCard(const QString& iconPath, const QString& title, const QString& desc) const
	{
		// 注意：将 size(200,180) 施加在 card 外层，而不是内部 panel 上
		return
			card(panel({
					icon(iconPath)->size(48)
								  ->color(isDark
											  ? QColor(100, 160, 220)
											  : QColor(60, 120, 180)),
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
