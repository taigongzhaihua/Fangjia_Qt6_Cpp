#pragma once
#include "HomePage.h"
#include <AdvancedWidgets.h>
#include <BasicWidgets.h>
#include <exception>
#include <Layouts.h>
#include <memory>
#include <qcolor.h>
#include <qfont.h>
#include <qlogging.h>
#include <qstring.h>
#include <UiComponent.hpp>
#include <UiPanel.h>
#include <Widget.h>
using namespace UI;

class HomePage::Impl
{
public:
	bool isDark = false;
	std::unique_ptr<IUiComponent> builtComponent;

	WidgetPtr buildUI()
	{
		// 使用 make_widget 确保对象被 shared_ptr 管理
		auto mainColumn = UI::make_widget<Panel>(WidgetList{
			// 欢迎标题
			UI::make_widget<Container>(
				UI::make_widget<Text>("欢迎使用方家")
			)->padding(0, 40, 0, 20),

				// 副标题
				UI::make_widget<Container>(
					UI::make_widget<Text>("中医方剂数据管理系统")
				)->padding(0, 0, 0, 40),

				// 功能卡片网格
				buildFeatureGrid(),

				// 留白（不再使用 Expanded）
				make_widget<Spacer>(8)
			});

		// Panel 纵向排列，交叉轴居中
		mainColumn->orientation(UiPanel::Orientation::Vertical);
		mainColumn->crossAxisAlignment(Alignment::Center);

		return mainColumn;
	}

private:
	WidgetPtr buildFeatureGrid()
	{
		return make_widget<Container>(
			make_widget<Panel>(WidgetList{
				make_widget<Panel>(WidgetList{
					buildFeatureCard(
						":/icons/data_light.svg",
						"方剂数据",
						"查看和管理中医方剂"
					),
					make_widget<Spacer>(16),
					buildFeatureCard(
						":/icons/explore_light.svg",
						"探索发现",
						"发现新的方剂组合"
					)
				})->orientation(UiPanel::Orientation::Horizontal)->crossAxisAlignment(Alignment::Center),

				make_widget<Spacer>(16),

				make_widget<Panel>(WidgetList{
					buildFeatureCard(
						":/icons/fav_light.svg",
						"我的收藏",
						"管理收藏的方剂"
					),
					make_widget<Spacer>(16),
					buildFeatureCard(
						":/icons/settings_light.svg",
						"系统设置",
						"自定义应用偏好"
					)
				})
				->orientation(UiPanel::Orientation::Horizontal)
				->crossAxisAlignment(Alignment::Center)
				})
		)->padding(40);
	}

	WidgetPtr buildFeatureCard(const QString& iconPath, const QString& title, const QString& desc)
	{
		auto icon = make_widget<Icon>(iconPath);
		icon->size(48);
		icon->color(isDark ? QColor(100, 160, 220) : QColor(60, 120, 180));

		auto titleText = make_widget<Text>(title);
		titleText->fontSize(16);
		titleText->fontWeight(QFont::DemiBold);
		titleText->color(isDark ? QColor(240, 245, 250) : QColor(30, 35, 40));

		auto descText = UI::make_widget<Text>(desc);
		descText->fontSize(13);
		descText->color(isDark ? QColor(180, 190, 200) : QColor(100, 110, 120));

		auto column = UI::make_widget<Panel>(WidgetList{
			icon,
			UI::make_widget<Spacer>(16),
			titleText,
			UI::make_widget<Spacer>(8),
			descText
			});
		column->orientation(UiPanel::Orientation::Vertical);
		column->crossAxisAlignment(Alignment::Center);

		auto card = UI::make_widget<Card>(column);
		card->elevation(2.0f);
		card->size(200, 180);

		return card;
	}
};

HomePage::HomePage() : m_impl(std::make_unique<Impl>())
{
	try
	{
		qDebug() << "setting title...\n";
		setTitle("首页");

		qDebug() << "initializing Content...\n";
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
		qDebug() << "building UI...\n";
		auto widget = m_impl->buildUI();
		qDebug() << (widget != nullptr ? "UI built successfully." : "Failed to build UI.");
		if (widget)
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

	// 重新构建UI
	auto widget = m_impl->buildUI();
	if (widget)
	{
		m_impl->builtComponent = widget->build();
		setContent(m_impl->builtComponent.get());
	}
}
