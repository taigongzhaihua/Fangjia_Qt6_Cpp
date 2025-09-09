#pragma once

/*
 * Button Callback Helpers
 * 
 * This header provides utilities to handle button callbacks properly
 * and prevent common onClick syntax errors.
 */

#include <functional>
#include <memory>

namespace UI {

// Forward declaration
class Button;

// Callback type aliases for clarity
using ClickCallback = std::function<void()>;
using HoverCallback = std::function<void(bool)>;

// Button callback interface to prevent direct member function access errors
class IButtonCallbacks {
public:
    virtual ~IButtonCallbacks() = default;
    virtual void setOnClick(ClickCallback callback) = 0;
    virtual void setOnHover(HoverCallback callback) = 0;
    virtual void click() = 0; // Programmatic click
};

// Helper class for safe button callback handling
class ButtonCallbackHandler {
private:
    ClickCallback m_onClick;
    HoverCallback m_onHover;

public:
    // Safe setter methods (prevent direct function access)
    void setClickHandler(ClickCallback callback) {
        m_onClick = std::move(callback);
    }
    
    void setHoverHandler(HoverCallback callback) {
        m_onHover = std::move(callback);
    }
    
    // Safe invocation methods
    void invokeClick() const {
        if (m_onClick) {
            m_onClick();
        }
    }
    
    void invokeHover(bool hovered) const {
        if (m_onHover) {
            m_onHover(hovered);
        }
    }
    
    // Check if handlers are set
    bool hasClickHandler() const { return static_cast<bool>(m_onClick); }
    bool hasHoverHandler() const { return static_cast<bool>(m_onHover); }
};

// Builder pattern for button callbacks to prevent syntax errors
class ButtonBuilder {
private:
    ButtonCallbackHandler m_callbacks;
    
public:
    ButtonBuilder& onClick(ClickCallback callback) {
        m_callbacks.setClickHandler(std::move(callback));
        return *this;
    }
    
    ButtonBuilder& onHover(HoverCallback callback) {
        m_callbacks.setHoverHandler(std::move(callback));
        return *this;
    }
    
    // Method to get the callback handler
    const ButtonCallbackHandler& getCallbacks() const {
        return m_callbacks;
    }
};

// Example usage patterns:
/*
// Correct usage:
auto button = ButtonBuilder()
    .onClick([]() { /* handle click */ })
    .onHover([](bool hovered) { /* handle hover */ });

// Wrong usage that this prevents:
// button.onClick(); // Error: function doesn't take 0 parameters  
// auto func = button.onClick; // Error: non-standard syntax
*/

// Utility function to create button with callbacks
template<typename... CallbackArgs>
ButtonBuilder createButton(CallbackArgs&&... args) {
    ButtonBuilder builder;
    // Apply callbacks using fold expression (C++17)
    (builder.onClick(std::forward<CallbackArgs>(args)), ...);
    return builder;
}

} // namespace UI