cmake_minimum_required(VERSION 3.16)

# MSVC 热重载支持
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(Fangjia_Qt6_Cpp LANGUAGES CXX)

# ========================================
# 质量门禁选项
# ========================================
option(FANGJIA_WARNINGS_AS_ERRORS "将编译警告视为错误（默认 OFF，本地开发更友好；CI 传 ON）" OFF)
option(ENABLE_CLANG_TIDY "启用 clang-tidy 静态检查（默认 OFF；CI Linux 打开）" OFF)

# Qt 设置
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找 Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL Widgets Svg)

# 设置源目录
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${PROJECT_ROOT}/src)

# ========================================
# 编译器警告配置
# ========================================
if(MSVC)
    # MSVC 警告设置
    set(FANGJIA_WARNING_FLAGS "/W4")
    if(FANGJIA_WARNINGS_AS_ERRORS)
        list(APPEND FANGJIA_WARNING_FLAGS "/WX")
    endif()
    # 禁用一些过于严格的警告
    list(APPEND FANGJIA_WARNING_FLAGS "/wd4267" "/wd4244" "/wd4996")
elseif(CMAKE_CXX_COMPILER_ID MATCHES "GNU|Clang")
    # GCC/Clang 警告设置
    set(FANGJIA_WARNING_FLAGS
        "-Wall" "-Wextra" "-Wpedantic"
        "-Wcast-align" "-Wcast-qual" "-Wctor-dtor-privacy"
        "-Wdisabled-optimization" "-Wformat=2" "-Winit-self"
        "-Wmissing-declarations" "-Wmissing-include-dirs"
        "-Wold-style-cast" "-Woverloaded-virtual" "-Wredundant-decls"
        "-Wshadow" "-Wsign-conversion" "-Wsign-promo"
        "-Wstrict-overflow=5" "-Wswitch-default" "-Wundef"
    )
    if(FANGJIA_WARNINGS_AS_ERRORS)
        list(APPEND FANGJIA_WARNING_FLAGS "-Werror")
    endif()
    # 针对特定编译器的额外警告
    if(CMAKE_CXX_COMPILER_ID MATCHES "GNU")
        list(APPEND FANGJIA_WARNING_FLAGS "-Wlogical-op" "-Wnoexcept" "-Wstrict-null-sentinel")
    endif()
endif()

# ========================================
# clang-tidy 配置
# ========================================
if(ENABLE_CLANG_TIDY)
    find_program(CLANG_TIDY_EXE NAMES "clang-tidy")
    if(CLANG_TIDY_EXE)
        message(STATUS "clang-tidy found: ${CLANG_TIDY_EXE}")
        set(CMAKE_CXX_CLANG_TIDY "${CLANG_TIDY_EXE}")
    else()
        message(WARNING "clang-tidy requested but not found")
    endif()
endif()

# ========================================
# clang-format 目标（可选）
# ========================================
find_program(CLANG_FORMAT_EXE NAMES "clang-format")
if(CLANG_FORMAT_EXE)
    file(GLOB_RECURSE FORMAT_SOURCES 
        "${SRC_DIR}/*.h" 
        "${SRC_DIR}/*.hpp" 
        "${SRC_DIR}/*.cpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.h"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.hpp"
        "${CMAKE_CURRENT_SOURCE_DIR}/tests/*.cpp"
    )
    
    add_custom_target(format
        COMMAND ${CLANG_FORMAT_EXE} -i ${FORMAT_SOURCES}
        COMMENT "Running clang-format on source files"
        VERBATIM
    )
    
    add_custom_target(format-check
        COMMAND ${CLANG_FORMAT_EXE} --dry-run --Werror ${FORMAT_SOURCES}
        COMMENT "Checking code format with clang-format"
        VERBATIM
    )
endif()

# ========================================
# 警告标志应用函数
# ========================================
function(apply_fangjia_warnings target)
    if(FANGJIA_WARNING_FLAGS)
        target_compile_options(${target} PRIVATE ${FANGJIA_WARNING_FLAGS})
    endif()
endfunction()

# ========================================
# 全局包含目录
# ========================================
include_directories(
    ${SRC_DIR}
    ${SRC_DIR}/core
    ${SRC_DIR}/core/rendering
    ${SRC_DIR}/core/platform/windows
    ${SRC_DIR}/core/config
    ${SRC_DIR}/core/vm
    ${SRC_DIR}/framework
    ${SRC_DIR}/framework/base
    ${SRC_DIR}/framework/containers
    ${SRC_DIR}/framework/widgets
    ${SRC_DIR}/framework/declarative
    ${SRC_DIR}/models
    ${SRC_DIR}/views
    ${SRC_DIR}/views/pages
    ${SRC_DIR}/views/formula
    ${SRC_DIR}/app
)

# ========================================
# 核心渲染库
# ========================================
file(GLOB CORE_RENDERING_SOURCES
    "${SRC_DIR}/core/rendering/*.cpp"
    "${SRC_DIR}/core/rendering/*.h"
    "${SRC_DIR}/core/rendering/*.hpp"
)

add_library(core_rendering STATIC ${CORE_RENDERING_SOURCES})
target_link_libraries(core_rendering PUBLIC 
    Qt6::Core 
    Qt6::Gui 
    Qt6::OpenGL 
    Qt6::Svg
)
apply_fangjia_warnings(core_rendering)

# ========================================
# 配置管理库（如果存在）
# ========================================
if(EXISTS "${SRC_DIR}/core/config/AppConfig.h")
    file(GLOB CONFIG_SOURCES
        "${SRC_DIR}/core/config/*.cpp"
        "${SRC_DIR}/core/config/*.h"
    )
    
    add_library(core_config STATIC ${CONFIG_SOURCES})
    target_link_libraries(core_config PUBLIC 
        Qt6::Core
        Qt6::Widgets
    )
    apply_fangjia_warnings(core_config)
endif()

# ========================================
# 平台特定库
# ========================================
if(WIN32)
    file(GLOB PLATFORM_SOURCES
        "${SRC_DIR}/core/platform/windows/*.cpp"
        "${SRC_DIR}/core/platform/windows/*.h"
    )
    
    if(PLATFORM_SOURCES)
        add_library(platform_windows STATIC ${PLATFORM_SOURCES})
        target_link_libraries(platform_windows PUBLIC Qt6::Core Qt6::Gui Dwmapi)
        target_compile_definitions(platform_windows PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
        apply_fangjia_warnings(platform_windows)
    endif()
endif()

# ========================================
# UI 框架库
# ========================================
file(GLOB UI_FRAMEWORK_SOURCES
    "${SRC_DIR}/framework/base/*.hpp"
    "${SRC_DIR}/framework/base/*.cpp"
    "${SRC_DIR}/framework/base/*.h"
    "${SRC_DIR}/framework/containers/*.cpp"
    "${SRC_DIR}/framework/containers/*.h"
    "${SRC_DIR}/framework/widgets/*.cpp"
    "${SRC_DIR}/framework/widgets/*.h"
    "${SRC_DIR}/framework/declarative/*.cpp"
    "${SRC_DIR}/framework/declarative/*.h"
)

add_library(ui_framework STATIC ${UI_FRAMEWORK_SOURCES})
target_link_libraries(ui_framework PUBLIC 
    core_rendering 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
)
apply_fangjia_warnings(ui_framework)

# ========================================
# 数据模型库
# ========================================
file(GLOB MODEL_SOURCES
    "${SRC_DIR}/models/*.cpp"
    "${SRC_DIR}/models/*.h"
    "${SRC_DIR}/core/vm/*.cpp"
    "${SRC_DIR}/core/vm/*.hpp"
)

add_library(models STATIC ${MODEL_SOURCES})
target_link_libraries(models PUBLIC Qt6::Core Qt6::Gui)
apply_fangjia_warnings(models)

# ========================================
# 业务视图库
# ========================================
file(GLOB VIEW_SOURCES
    "${SRC_DIR}/views/formula/*.cpp"
    "${SRC_DIR}/views/formula/*.h"
)
file(GLOB PAGE_SOURCES
    "${SRC_DIR}/views/pages/*.cpp"
    "${SRC_DIR}/views/pages/*.h"
)

# 将页面源文件添加到 business_views
set(VIEW_SOURCES
    ${VIEW_SOURCES}
    ${PAGE_SOURCES}
)

add_library(business_views STATIC ${VIEW_SOURCES})
target_link_libraries(business_views PUBLIC 
    ui_framework 
    models 
    core_rendering
)
apply_fangjia_warnings(business_views)

# ========================================
# 主应用程序
# ========================================
set(APP_SOURCES
    ${SRC_DIR}/app/main.cpp
    ${SRC_DIR}/app/MainOpenGlWindow.h
    ${SRC_DIR}/app/MainOpenGlWindow.cpp
)

# 资源文件
if(EXISTS "${PROJECT_ROOT}/resources/resources.qrc")
    list(APPEND APP_SOURCES ${PROJECT_ROOT}/resources/resources.qrc)
endif()

add_executable(FangJia WIN32 ${APP_SOURCES})

# 链接库
set(APP_LIBS
    business_views
    ui_framework
    models
    core_rendering
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::Widgets
    Qt6::Svg
)

# 条件链接
if(TARGET core_config)
    list(APPEND APP_LIBS core_config)
endif()

if(WIN32 AND TARGET platform_windows)
    list(APPEND APP_LIBS platform_windows)
endif()

target_link_libraries(FangJia PRIVATE ${APP_LIBS})
apply_fangjia_warnings(FangJia)

# ========================================
# 测试子项目
# ========================================
add_subdirectory(tests)

# ========================================
# 示例子项目（如果存在）
# ========================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    add_subdirectory(examples)
endif()

# ========================================
# 输出配置信息
# ========================================
message(STATUS "===========================================")
message(STATUS "Fangjia Qt6 项目配置")
message(STATUS "===========================================")
message(STATUS "Qt6 版本: ${Qt6_VERSION}")
message(STATUS "源目录: ${SRC_DIR}")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "===========================================")