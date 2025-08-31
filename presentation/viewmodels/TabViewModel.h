#pragma once
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qstring.h>
#include <qtmetamacros.h>
#include "tab_interface.h"

// 通用 Tab ViewModel：承载标签页数据与选中状态
class TabViewModel final : public fj::presentation::binding::ITabDataProvider
{
	Q_OBJECT
public:

	explicit TabViewModel(QObject* parent = nullptr);

	// 标签页数据
	void setItems(QVector<fj::presentation::binding::TabItem> items);
	[[nodiscard]] QVector<fj::presentation::binding::TabItem> items() const override;
	[[nodiscard]] int count() const override;

	// 选中项
	[[nodiscard]] int selectedIndex() const override;
	void setSelectedIndex(int idx) override;

	// 查找项
	[[nodiscard]] int findById(const QString& id) const override;
	[[nodiscard]] QString selectedId() const override;

signals:
	void itemsChanged();
	void selectedIndexChanged(int index);

private:
	QVector<fj::presentation::binding::TabItem> m_items;
	int m_selected{ 0 };
};