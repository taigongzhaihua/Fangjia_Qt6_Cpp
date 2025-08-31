#include "NavViewModel.h"

#include <nav_interface.h>
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qtmetamacros.h>
#include <utility>

NavViewModel::NavViewModel(QObject* parent)
	: INavDataProvider(parent)
{
	initItems();
}

void NavViewModel::setItems(QVector<Item> items)
{
	m_items = std::move(items);
	emit itemsChanged();

	// 修正 selectedIndex 合法性
	if (m_selected < 0 && !m_items.isEmpty()) {
		setSelectedIndex(0);
	}
	else if (m_selected >= m_items.size()) {
		setSelectedIndex(m_items.isEmpty() ? -1 : 0);
	}
}

QVector<fj::presentation::binding::NavItem> NavViewModel::items() const
{
	QVector<fj::presentation::binding::NavItem> result;
	result.reserve(m_items.size());
	for (const auto& [id, svgLight, svgDark, label] : m_items) {
		result.append({
			.id = id,
			.svgLight = svgLight,
			.svgDark = svgDark,
			.label = label
			});
	}
	return result;
}

void NavViewModel::setSelectedIndex(const int idx)
{
	if (idx < -1 || idx >= m_items.size()) return;
	if (m_selected == idx) return;
	m_selected = idx;
	emit selectedIndexChanged(m_selected);
}

void NavViewModel::setExpanded(const bool on)
{
	if (m_expanded == on) return;
	m_expanded = on;
	emit expandedChanged(m_expanded);
}

void NavViewModel::toggleExpanded()
{
	setExpanded(!m_expanded);
}

void NavViewModel::initItems()
{
	setItems(QVector<Item>{
		{.id = "home", .svgLight = ":/icons/home_light.svg", .svgDark = ":/icons/home_dark.svg", .label = "首页"},
		{ .id = "data", .svgLight = ":/icons/data_light.svg", .svgDark = ":/icons/data_dark.svg", .label = "数据" },
		{
			.id = "explore", .svgLight = ":/icons/explore_light.svg", .svgDark = ":/icons/explore_dark.svg",
			.label = "探索"
		},
		{ .id = "favorites", .svgLight = ":/icons/fav_light.svg", .svgDark = ":/icons/fav_dark.svg", .label = "收藏" },
		{
			.id = "settings", .svgLight = ":/icons/settings_light.svg", .svgDark = ":/icons/settings_dark.svg",
			.label = "设置"
		}
	});
}
