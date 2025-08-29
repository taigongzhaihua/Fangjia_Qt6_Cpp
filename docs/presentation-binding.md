# Presentation Binding Layer

This layer provides tiny interfaces for UI-agnostic bindings:
- `ICommand` for actions
- `IValueAdapter<T>` for simple property get/set + change notifications
- `INavDataProvider` for navigation data abstraction  
- `ITabDataProvider` for tab data abstraction

## Rules
- `presentation/ui` must not include or link against `presentation/viewmodels`.
- Pages own concrete ViewModel instances and inject adapters/commands into UI controls.

## Benefits
- UI is a reusable framework without business dependencies.
- ViewModels remain testable and can be used with different UIs.

## Usage Example

In the presentation/pages layer, create adapters that implement the binding interfaces:

```cpp
// Create a concrete ViewModel
auto tabViewModel = std::make_shared<TabViewModel>();

// Create an adapter that implements ITabDataProvider
class TabViewModelAdapter : public fj::presentation::binding::ITabDataProvider {
public:
    explicit TabViewModelAdapter(TabViewModel* vm) : m_vm(vm) {
        connect(m_vm, &TabViewModel::itemsChanged, this, &ITabDataProvider::itemsChanged);
        connect(m_vm, &TabViewModel::selectedIndexChanged, this, &ITabDataProvider::selectedIndexChanged);
    }
    
    QVector<fj::presentation::binding::TabItem> items() const override {
        // Convert TabViewModel::TabItem to binding::TabItem
        QVector<fj::presentation::binding::TabItem> result;
        for (const auto& item : m_vm->items()) {
            result.append({item.id, item.label, item.tooltip});
        }
        return result;
    }
    
    int count() const override { return m_vm->count(); }
    int selectedIndex() const override { return m_vm->selectedIndex(); }
    void setSelectedIndex(int idx) override { m_vm->setSelectedIndex(idx); }
    // ... other methods
    
private:
    TabViewModel* m_vm;
};

// In the page's buildUI method:
auto adapter = new TabViewModelAdapter(tabViewModel.get());
return UI::tabView()
    ->dataProvider(adapter)
    ->content(/* ... */);
```

This approach maintains clean separation while allowing the UI to remain framework-agnostic.