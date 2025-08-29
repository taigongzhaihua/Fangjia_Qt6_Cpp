#pragma once
#include <QObject>
#include <QAbstractButton>
#include <QCheckBox>

#include "value_adapter.h"
#include "command.h"

namespace fj::presentation::binding {

// Helpers that UI widgets (or pages) can use to connect to adapters/commands without VM headers.

inline void bindCheckBox(QCheckBox* checkbox, IValueAdapter<bool>* adapter) {
    QObject::connect(checkbox, &QCheckBox::toggled, adapter, [adapter](bool v){ adapter->set(v); });
    QObject::connect(adapter, &IValueAdapterBase::changed, checkbox, [checkbox, adapter]{
        const QSignalBlocker b(checkbox);
        checkbox->setChecked(adapter->get());
    });
    // Initialize UI from model
    checkbox->setChecked(adapter->get());
}

inline void bindCommand(QAbstractButton* button, ICommand* cmd) {
    QObject::connect(button, &QAbstractButton::clicked, cmd, [cmd]{ if (cmd->canExecute()) cmd->execute(); });
    // If cmd exposes canExecute change, we can later add enable/disable binding.
}

} // namespace fj::presentation::binding