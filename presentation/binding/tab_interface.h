#pragma once
#include <QObject>
#include <QString>
#include <QVector>

namespace fj::presentation::binding {

// Tab item structure that UI layer can use
struct TabItem {
    QString id;      // 唯一标识
    QString label;   // 显示文本
    QString tooltip; // 工具提示（可选）
};

// Interface for tab data that UI can bind to
class ITabDataProvider : public QObject {
    Q_OBJECT
public:
    explicit ITabDataProvider(QObject* parent = nullptr) : QObject(parent) {}
    ~ITabDataProvider() override = default;

    virtual QVector<TabItem> items() const = 0;
    virtual int count() const = 0;
    virtual int selectedIndex() const = 0;
    virtual void setSelectedIndex(int idx) = 0;
    virtual QString selectedId() const = 0;
    virtual int findById(const QString& id) const = 0;

signals:
    void itemsChanged();
    void selectedIndexChanged(int index);
};

} // namespace fj::presentation::binding