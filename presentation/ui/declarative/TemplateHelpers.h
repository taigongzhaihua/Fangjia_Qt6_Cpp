#pragma once

/*
 * Template Specialization Helpers
 * 
 * This header provides utilities to prevent common template specialization errors
 * and ensure proper template function declarations.
 */

#include <memory>
#include <type_traits>

namespace UI {

// Forward declarations to prevent specialization errors
template<typename T>
struct Builder;

template<typename T>
struct ComponentTraits;

// Base template function declaration
template<typename T, typename... Args>
std::shared_ptr<Builder<T>> create(Args&&... args) {
    static_assert(std::is_constructible_v<Builder<T>, Args...>, 
                  "Builder<T> must be constructible with given arguments");
    return std::make_shared<Builder<T>>(std::forward<Args>(args)...);
}

// Example proper specialization pattern:
// First declare the specialized types
struct Window {
    enum class Type { Normal, Dialog, Popup };
};

struct Layout {
    enum class Type { Vertical, Horizontal, Grid };
};

// Then declare specialized builders
template<>
struct Builder<Window> {
    Builder(Window::Type type) : windowType(type) {}
    Window::Type windowType;
    // Add builder methods here
};

template<>
struct Builder<Layout> {
    Builder(Layout::Type type) : layoutType(type) {}
    Layout::Type layoutType;
    // Add builder methods here
};

// Template function specializations (if needed)
// These should come after the template declaration above
template<>
std::shared_ptr<Builder<Window>> create<Window>(Window::Type type) {
    return std::make_shared<Builder<Window>>(type);
}

template<>
std::shared_ptr<Builder<Layout>> create<Layout>(Layout::Type type) {
    return std::make_shared<Builder<Layout>>(type);
}

// Utility to check if a type has proper builder support
template<typename T>
constexpr bool has_builder_v = std::is_constructible_v<Builder<T>>;

// Safe create function that checks for builder support
template<typename T, typename... Args>
auto safe_create(Args&&... args) -> std::enable_if_t<has_builder_v<T>, std::shared_ptr<Builder<T>>> {
    return create<T>(std::forward<Args>(args)...);
}

} // namespace UI