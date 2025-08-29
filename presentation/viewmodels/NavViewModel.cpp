#include "NavViewModel.h"

#include <utility>

NavViewModel::NavViewModel(QObject* parent)
	: INavDataProvider(parent)
{
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
	for (const auto& item : m_items) {
		result.append({
			.id = item.id,
			.svgLight = item.svgLight,
			.svgDark = item.svgDark,
			.label = item.label
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