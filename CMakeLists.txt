cmake_minimum_required(VERSION 3.16)

# MSVC 热重载支持
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(Fangjia_Qt6_Cpp LANGUAGES CXX)

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
# 全局包含目录
# ========================================
include_directories(
    ${SRC_DIR}
    ${SRC_DIR}/core
    ${SRC_DIR}/core/rendering
    ${SRC_DIR}/core/platform/windows
    ${SRC_DIR}/core/config
    ${SRC_DIR}/core/di
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
endif()

# ========================================
# 依赖注入库
# ========================================
file(GLOB DI_SOURCES
    "${SRC_DIR}/core/di/*.cpp"
    "${SRC_DIR}/core/di/*.h"
    "${SRC_DIR}/core/di/*.hpp"
)

if(DI_SOURCES)
    add_library(core_di STATIC ${DI_SOURCES})
    target_link_libraries(core_di PUBLIC 
        Qt6::Core
        models  # 需要链接 models 因为 ServiceRegistry 使用了 ViewModels
        core_config  # 需要链接 config
    )
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
    endif()
endif()

# ========================================
# 添加声明式UI库
# ========================================
file(GLOB DECLARATIVE_SOURCES
    "${SRC_DIR}/framework/declarative/*.cpp"
    "${SRC_DIR}/framework/declarative/*.h"
)

add_library(declarative_ui STATIC ${DECLARATIVE_SOURCES})
target_link_libraries(declarative_ui PUBLIC 
    ui_framework
    core_rendering
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
)

# ========================================
# UI 框架库
# ========================================
file(GLOB UI_FRAMEWORK_SOURCES
    "${SRC_DIR}/framework/base/*.hpp"
    "${SRC_DIR}/framework/containers/*.cpp"
    "${SRC_DIR}/framework/containers/*.h"
    "${SRC_DIR}/framework/widgets/*.cpp"
    "${SRC_DIR}/framework/widgets/*.h"
)

add_library(ui_framework STATIC ${UI_FRAMEWORK_SOURCES})
target_link_libraries(ui_framework PUBLIC 
    core_rendering 
    Qt6::Core 
    Qt6::Gui 
    Qt6::Widgets
)

# ========================================
# 数据模型库
# ========================================
file(GLOB MODEL_SOURCES
    "${SRC_DIR}/models/*.cpp"
    "${SRC_DIR}/models/*.h"
)

add_library(models STATIC ${MODEL_SOURCES})
target_link_libraries(models PUBLIC Qt6::Core Qt6::Gui)

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
    declarative_ui
    ui_framework 
    models 
    core_rendering
)

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

if(TARGET core_di)
    list(APPEND APP_LIBS core_di)
endif()

if(WIN32 AND TARGET platform_windows)
    list(APPEND APP_LIBS platform_windows)
endif()

target_link_libraries(FangJia PRIVATE ${APP_LIBS})

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