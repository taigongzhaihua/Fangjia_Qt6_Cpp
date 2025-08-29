#pragma once
#include <QObject>
#include <functional>

namespace fj::presentation::binding {

// Base interface without template to work with Qt's MOC
class IValueAdapterBase : public QObject {
    Q_OBJECT
public:
    explicit IValueAdapterBase(QObject* parent = nullptr) : QObject(parent) {}
    ~IValueAdapterBase() override = default;

signals:
    void changed();
};

template <typename T>
class IValueAdapter : public IValueAdapterBase {
public:
    explicit IValueAdapter(QObject* parent = nullptr) : IValueAdapterBase(parent) {}
    ~IValueAdapter() override = default;

    virtual T get() const = 0;
    virtual void set(const T& value) = 0;
};

// A light-weight adapter that wraps getter/setter/signaller.
template <typename T>
class FunctionValueAdapter final : public IValueAdapter<T> {
public:
    using Getter = std::function<T()>;
    using Setter = std::function<void(const T&)>;

    FunctionValueAdapter(Getter g, Setter s, QObject* owner, const char* signal)
        : IValueAdapter<T>(owner), get_(std::move(g)), set_(std::move(s)) {
        // Re-emit owner's change signal as adapter's changed()
        QObject::connect(owner, signal, this, &IValueAdapterBase::changed);
    }

    T get() const override { return get_(); }
    void set(const T& v) override { set_(v); }

private:
    Getter get_;
    Setter set_;
};

} // namespace fj::presentation::binding