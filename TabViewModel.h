#pragma once
#include <qcontainerfwd.h>
#include <qobject.h>
#include <qstring.h>
#include <qtmetamacros.h>

// 通用 Tab ViewModel：承载标签页数据与选中状态
class TabViewModel final : public QObject
{
	Q_OBJECT
public:
	struct TabItem {
		QString id;      // 唯一标识
		QString label;   // 显示文本
		QString tooltip; // 工具提示（可选）
	};

	explicit TabViewModel(QObject* parent = nullptr);

	// 标签页数据
	void setItems(QVector<TabItem> items);
	[[nodiscard]] const QVector<TabItem>& items() const noexcept { return m_items; }
	[[nodiscard]] int count() const noexcept { return m_items.size(); }

	// 选中项
	[[nodiscard]] int selectedIndex() const noexcept { return m_selected; }
	void setSelectedIndex(int idx);

	// 查找项
	[[nodiscard]] int findById(const QString& id) const;
	[[nodiscard]] QString selectedId() const;

signals:
	void itemsChanged();
	void selectedIndexChanged(int index);

private:
	QVector<TabItem> m_items;
	int m_selected{ 0 };
};