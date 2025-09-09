# Compilation Errors Fix Guide

This document provides solutions for common C++ compilation errors that may occur in Qt/OpenGL projects.

## Current Status
The current repository builds successfully without any compilation errors. The errors mentioned in the original problem statement appear to reference files and code patterns that don't exist in the current codebase.

## Common Error Patterns and Solutions

### 1. Template Specialization Errors

**Error Pattern:** 
```
error C2912: 显式专用化；"fx::WindowBuilder fx::create<fx::Window>(fx::WindowType)"不是函数模板的专用化
```

**Solution:**
Ensure template functions are properly declared before specialization:

```cpp
// Wrong: Specialization without proper template declaration
template<>
WindowBuilder create<Window>(WindowType type) { ... }

// Correct: Proper template function declaration first
template<typename T>
Builder create(typename T::Type type);

// Then specialize
template<>
WindowBuilder create<Window>(WindowType type) { ... }
```

### 2. OpenGL Header Conflicts

**Error Pattern:**
```
fatal error C1189: #error: OpenGL header already included, remove this include, glad already provides it
```

**Solution:**
Ensure proper order of OpenGL header inclusions:

```cpp
// Wrong: Including both OpenGL headers and glad
#include <GL/gl.h>
#include <glad/glad.h>

// Correct: Only include glad, which provides OpenGL functions
#include <glad/glad.h>
// Remove other OpenGL includes
```

### 3. Missing Header Files

**Error Pattern:**
```
fatal error C1083: 无法打开包括文件: "stb_image.h": No such file or directory
```

**Solution:**
Add missing third-party libraries to CMakeLists.txt:

```cmake
# Add third-party library directory
target_include_directories(target_name PRIVATE 
    ${CMAKE_SOURCE_DIR}/thirdParty/include
    ${CMAKE_SOURCE_DIR}/thirdParty/stb
)

# Or use FetchContent for automatic downloading
include(FetchContent)
FetchContent_Declare(
    stb
    GIT_REPOSITORY https://github.com/nothings/stb.git
    GIT_TAG master
)
FetchContent_MakeAvailable(stb)
```

### 4. Member Function Call Syntax Errors

**Error Pattern:**
```
error C3867: "fx::Button::onClick": 非标准语法；请使用 "&" 来创建指向成员的指针
error C2660: "fx::Button::onClick": 函数不接受 0 个参数
```

**Solution:**
Use proper member function pointer syntax:

```cpp
// Wrong: Direct member function access
button.onClick();
auto callback = button.onClick;

// Correct: Proper member function calls
button.onClick([]() { /* handler */ });  // If onClick is a setter
auto callback = &Button::onClick;         // If getting function pointer
```

### 5. Lambda Conversion Issues

**Error Pattern:**
```
error C2664: "void fx::ILayoutable::setLayoutConstraints(const fx::ILayoutable::LayoutConstraints &)": 
无法将参数 1 从"fx::UIBuilder::minSize::<lambda_1>"转换为"const fx::ILayoutable::LayoutConstraints &"
```

**Solution:**
Use proper lambda-to-struct conversions:

```cpp
// Wrong: Direct lambda to struct conversion
auto lambda = [](int w, int h) { return QSize(w, h); };
setLayoutConstraints(lambda);

// Correct: Create proper constraint object
LayoutConstraints constraints;
constraints.minWidth = width;
constraints.minHeight = height;
setLayoutConstraints(constraints);

// Or use lambda with proper signature
auto constraintFactory = [](int w, int h) -> LayoutConstraints {
    LayoutConstraints c;
    c.minWidth = w;
    c.minHeight = h;
    return c;
};
setLayoutConstraints(constraintFactory(width, height));
```

### 6. Const-Correctness Issues

**Error Pattern:**
```
error C2662: "std::shared_ptr<fx::UIElement> fx::UIBuilder::build(void)": 
不能将"this"指针从"const _Elem"转换为"fx::UIBuilder &"
```

**Solution:**
Add const-correctness to member functions:

```cpp
// Wrong: Non-const method called on const object
class UIBuilder {
public:
    std::shared_ptr<UIElement> build();  // Non-const
};

// Correct: Make method const-correct
class UIBuilder {
public:
    std::shared_ptr<UIElement> build() const;  // const method
};
```

## Prevention Strategies

1. **Use proper include guards and forward declarations**
2. **Ensure template declarations precede specializations**
3. **Use CMake FetchContent for third-party dependencies**
4. **Apply const-correctness consistently**
5. **Use proper lambda signatures and conversions**
6. **Test compilation on different compilers (GCC, Clang, MSVC)**

## Build Verification

The current project can be built successfully with:

```bash
mkdir -p build
cd build
cmake ..
make -j$(nproc)
```

All targets build without errors on the current codebase.