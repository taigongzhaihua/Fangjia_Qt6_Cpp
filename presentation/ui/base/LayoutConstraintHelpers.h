#pragma once

/*
 * Layout Constraints Helpers
 * 
 * This header provides utilities to handle layout constraints properly
 * and prevent lambda conversion errors with setLayoutConstraints.
 */

#include <functional>
#include <qsize.h>

namespace UI {

// Layout constraints structure (prevent lambda conversion errors)
struct LayoutConstraints {
    int minWidth{-1};
    int minHeight{-1};
    int maxWidth{-1};
    int maxHeight{-1};
    int prefWidth{-1};
    int prefHeight{-1};
    
    // Factory methods for common constraint patterns
    static LayoutConstraints minSize(int width, int height) {
        LayoutConstraints c;
        c.minWidth = width;
        c.minHeight = height;
        return c;
    }
    
    static LayoutConstraints maxSize(int width, int height) {
        LayoutConstraints c;
        c.maxWidth = width;
        c.maxHeight = height;
        return c;
    }
    
    static LayoutConstraints fixedSize(int width, int height) {
        LayoutConstraints c;
        c.minWidth = c.maxWidth = c.prefWidth = width;
        c.minHeight = c.maxHeight = c.prefHeight = height;
        return c;
    }
    
    static LayoutConstraints fillWidth(int height = -1) {
        LayoutConstraints c;
        c.minWidth = 0;
        c.maxWidth = std::numeric_limits<int>::max();
        if (height > 0) {
            c.minHeight = c.maxHeight = c.prefHeight = height;
        }
        return c;
    }
    
    static LayoutConstraints fillHeight(int width = -1) {
        LayoutConstraints c;
        c.minHeight = 0;
        c.maxHeight = std::numeric_limits<int>::max();
        if (width > 0) {
            c.minWidth = c.maxWidth = c.prefWidth = width;
        }
        return c;
    }
};

// Interface for layoutable components
class ILayoutable {
public:
    virtual ~ILayoutable() = default;
    
    // Proper signature to prevent lambda conversion errors
    virtual void setLayoutConstraints(const LayoutConstraints& constraints) = 0;
    virtual LayoutConstraints getLayoutConstraints() const = 0;
};

// Builder class to safely create layout constraints
class LayoutConstraintBuilder {
private:
    LayoutConstraints m_constraints;
    
public:
    LayoutConstraintBuilder& minSize(int width, int height) {
        m_constraints.minWidth = width;
        m_constraints.minHeight = height;
        return *this;
    }
    
    LayoutConstraintBuilder& maxSize(int width, int height) {
        m_constraints.maxWidth = width;
        m_constraints.maxHeight = height;
        return *this;
    }
    
    LayoutConstraintBuilder& fixedSize(int width, int height) {
        m_constraints.minWidth = m_constraints.maxWidth = m_constraints.prefWidth = width;
        m_constraints.minHeight = m_constraints.maxHeight = m_constraints.prefHeight = height;
        return *this;
    }
    
    LayoutConstraintBuilder& fillWidth() {
        m_constraints.minWidth = 0;
        m_constraints.maxWidth = std::numeric_limits<int>::max();
        return *this;
    }
    
    LayoutConstraintBuilder& fillHeight() {
        m_constraints.minHeight = 0;
        m_constraints.maxHeight = std::numeric_limits<int>::max();
        return *this;
    }
    
    // Convert to constraints object (prevents lambda conversion errors)
    LayoutConstraints build() const {
        return m_constraints;
    }
    
    // Implicit conversion operator for convenience
    operator LayoutConstraints() const {
        return m_constraints;
    }
};

// Safe constraint setting for any layoutable component
template<typename T>
void setConstraints(T* component, const LayoutConstraints& constraints) {
    static_assert(std::is_base_of_v<ILayoutable, T>, "T must implement ILayoutable");
    component->setLayoutConstraints(constraints);
}

// Example usage patterns that prevent lambda conversion errors:
/*
// Correct usage with builder:
auto constraints = LayoutConstraintBuilder()
    .minSize(100, 50)
    .maxSize(200, 100)
    .build();
component->setLayoutConstraints(constraints);

// Correct usage with factory methods:
component->setLayoutConstraints(LayoutConstraints::minSize(100, 50));

// Wrong usage that this prevents:
// auto lambda = [](int w, int h) { return QSize(w, h); };
// component->setLayoutConstraints(lambda); // Compilation error prevented
*/

} // namespace UI