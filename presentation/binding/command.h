#pragma once
#include <QObject>
#include <QString>
#include <functional>

namespace fj::presentation::binding {

// ICommand interface that UI can trigger without knowing the VM type.
class ICommand : public QObject {
    Q_OBJECT
public:
    using CanExecuteFn = std::function<bool()>;

    explicit ICommand(QObject* parent = nullptr) : QObject(parent) {}
    ~ICommand() override = default;

    Q_INVOKABLE virtual void execute() = 0;
    Q_INVOKABLE virtual bool canExecute() const { return true; }
};

// A simple function-backed command for convenience in pages/adapters
class FunctionCommand final : public ICommand {
    Q_OBJECT
public:
    explicit FunctionCommand(std::function<void()> exec,
                             CanExecuteFn can = {},
                             QObject* parent = nullptr)
        : ICommand(parent), exec_(std::move(exec)), can_(std::move(can)) {}

    void execute() override { if (exec_) exec_(); }
    bool canExecute() const override { return can_ ? can_() : true; }

private:
    std::function<void()> exec_;
    CanExecuteFn can_;
};

} // namespace fj::presentation::binding