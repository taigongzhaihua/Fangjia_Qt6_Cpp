#pragma once
#include <QObject>

namespace fj::presentation::binding {

// Minimal Qt-friendly change notification contract to be implemented by VMs or adapters.
class INotifyPropertyChanged {
public:
    virtual ~INotifyPropertyChanged() = default;
signals:
    // Emitted when any relevant property changed; name can be empty if not specified
    virtual void propertyChanged(const QString& name) = 0;
};

} // namespace fj::presentation::binding