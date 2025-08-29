#pragma once
#include "UiNav.h"
#include <memory>

// Forward declarations
class NavViewModel;

namespace Ui {

/// NavWrapper: Adapter that connects NavRail UI component with NavViewModel
/// This moves the ViewModel dependency out of the pure UI layer
class NavWrapper {
public:
    explicit NavWrapper();
    ~NavWrapper();

    // UI component access
    NavRail* component() const { return m_navRail.get(); }

    // ViewModel integration
    void setViewModel(NavViewModel* vm);
    NavViewModel* viewModel() const { return m_vm; }

private:
    void connectSignals();
    void disconnectSignals();
    void syncFromViewModel();

private:
    std::unique_ptr<NavRail> m_navRail;
    NavViewModel* m_vm{nullptr};
};

} // namespace Ui