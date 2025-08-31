#include "TabViewModel.h"
#include <utility>

TabViewModel::TabViewModel(QObject* parent)
	: ITabDataProvider(parent)
{
}

void TabViewModel::setItems(QVector<fj::presentation::binding::TabItem> items)
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

void TabViewModel::setSelectedIndex(const int idx)
{
	if (idx < -1 || idx >= m_items.size()) return;
	if (m_selected == idx) return;
	m_selected = idx;
	emit selectedIndexChanged(m_selected);
}

int TabViewModel::findById(const QString& id) const
{
	for (int i = 0; i < m_items.size(); ++i) {
		if (m_items[i].id == id) return i;
	}
	return -1;
}

QVector<fj::presentation::binding::TabItem> TabViewModel::items() const
{
	return m_items;
}

int TabViewModel::count() const
{
	return m_items.size();
}

int TabViewModel::selectedIndex() const
{
	return m_selected;
}

QString TabViewModel::selectedId() const
{
	if (m_selected >= 0 && m_selected < m_items.size()) {
		return m_items[m_selected].id;
	}
	return QString();
}