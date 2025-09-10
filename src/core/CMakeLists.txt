# Core library - 不依赖任何外部UI库，纯C++标准库实现
# 基于PR #21的设计原则：清晰的接口、生命周期管理、状态管理

file(GLOB FANGJIA_CORE_SOURCES
    "interfaces/*.h"
    "interfaces/*.hpp"
    "base/*.cpp"
    "base/*.h"
    "lifecycle/*.cpp"
    "lifecycle/*.h"
    "events/*.cpp"
    "events/*.h"
    "di/*.hpp"
    "di/*.cpp"
)

add_library(fangjia_core STATIC ${FANGJIA_CORE_SOURCES})

# 只依赖标准库和boost-ext/di
target_include_directories(fangjia_core PUBLIC 
    .
    interfaces
    base
    lifecycle
    events
    di
    ${boost_di_SOURCE_DIR}/include
)

# 设置编译选项
target_compile_features(fangjia_core PUBLIC cxx_std_23)