#include "NavWrapper.h"
#include "NavViewModel.h"

namespace Ui {

NavWrapper::NavWrapper()
    : m_navRail(std::make_unique<NavRail>())
{
}

NavWrapper::~NavWrapper() = default;

void NavWrapper::setViewModel(NavViewModel* vm)
{
    if (m_vm == vm) return;
    
    disconnectSignals();
    m_vm = vm;
    
    if (m_vm) {
        // Set the ViewModel on the UI component
        m_navRail->setViewModel(m_vm);
        connectSignals();
        syncFromViewModel();
    } else {
        m_navRail->setViewModel(nullptr);
    }
}

void NavWrapper::connectSignals()
{
    // Connect ViewModel signals to UI updates if needed
    // This is where the ViewModel-UI binding logic would go
}

void NavWrapper::disconnectSignals()
{
    // Disconnect any existing signal connections
}

void NavWrapper::syncFromViewModel()
{
    if (!m_vm) return;
    
    // Synchronize UI state with ViewModel state
    // This replaces the syncFromVmInstant() logic that was in the UI component
}

} // namespace Ui