#pragma once
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qstring.h>
#include <qtmetamacros.h>

// 轻量导航 ViewModel：承载业务真值（items / selected / expanded）与变更信号
class NavViewModel final : public QObject
{
	Q_OBJECT
public:
	struct Item {
		QString id;
		QString svgLight;
		QString svgDark;
		QString label;
	};

	explicit NavViewModel(QObject* parent = nullptr);

	// 列表数据
	void setItems(QVector<Item> items);
	[[nodiscard]] const QVector<Item>& items() const noexcept { return m_items; }
	[[nodiscard]] int count() const noexcept { return m_items.size(); }

	// 选中项
	[[nodiscard]] int selectedIndex() const noexcept { return m_selected; }
	void setSelectedIndex(int idx);

	// 展开/收起
	[[nodiscard]] bool expanded() const noexcept { return m_expanded; }
	void setExpanded(bool on);
	void toggleExpanded();

signals:
	void itemsChanged();
	void selectedIndexChanged(int index);
	void expandedChanged(bool expanded);

private:
	QVector<Item> m_items;
	int  m_selected{ -1 };
	bool m_expanded{ false };
};