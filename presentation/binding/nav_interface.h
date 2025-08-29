#pragma once
#include <QObject>
#include <QString>
#include <QVector>

namespace fj::presentation::binding {

// Navigation item structure that UI layer can use
struct NavItem {
    QString id;
    QString svgLight;
    QString svgDark;
    QString label;
};

// Interface for navigation data that UI can bind to
class INavDataProvider : public QObject {
    Q_OBJECT
public:
    explicit INavDataProvider(QObject* parent = nullptr) : QObject(parent) {}
    ~INavDataProvider() override = default;

    virtual QVector<NavItem> items() const = 0;
    virtual int count() const = 0;
    virtual int selectedIndex() const = 0;
    virtual void setSelectedIndex(int idx) = 0;
    virtual bool expanded() const = 0;
    virtual void setExpanded(bool expanded) = 0;

signals:
    void itemsChanged();
    void selectedIndexChanged(int index);
    void expandedChanged(bool expanded);
};

} // namespace fj::presentation::binding