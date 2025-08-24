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

# 检查文件是否存在的函数
function(check_source_files)
    set(REQUIRED_FILES
        "${SRC_DIR}/core/rendering/RenderData.hpp"
        "${SRC_DIR}/core/rendering/Renderer.h"
        "${SRC_DIR}/framework/base/UiComponent.hpp"
        "${SRC_DIR}/app/main.cpp"
    )
    
    foreach(FILE ${REQUIRED_FILES})
        if(NOT EXISTS ${FILE})
            message(WARNING "Missing file: ${FILE}")
        endif()
    endforeach()
endfunction()

# 检查文件
check_source_files()

# ========================================
# 全局包含目录（重要！）
# ========================================
include_directories(
    ${SRC_DIR}
    ${SRC_DIR}/core
    ${SRC_DIR}/core/rendering
    ${SRC_DIR}/core/platform/windows
    ${SRC_DIR}/framework
    ${SRC_DIR}/framework/base
    ${SRC_DIR}/framework/containers
    ${SRC_DIR}/framework/widgets
    ${SRC_DIR}/models
    ${SRC_DIR}/views
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

if(CORE_RENDERING_SOURCES)
    add_library(core_rendering STATIC ${CORE_RENDERING_SOURCES})
else()
    # 如果文件还在旧位置，使用旧路径
    add_library(core_rendering STATIC
        RenderData.hpp
        Renderer.h Renderer.cpp
        IconLoader.h IconLoader.cpp
    )
    target_include_directories(core_rendering PUBLIC ${PROJECT_ROOT})
endif()

target_link_libraries(core_rendering PUBLIC 
    Qt6::Core 
    Qt6::Gui 
    Qt6::OpenGL 
    Qt6::Svg
)

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
    else()
        # 使用旧路径
        if(EXISTS "${PROJECT_ROOT}/WinWindowChrome.cpp")
            add_library(platform_windows STATIC
                WinWindowChrome.h
                WinWindowChrome.cpp
            )
            target_include_directories(platform_windows PUBLIC ${PROJECT_ROOT})
        endif()
    endif()
    
    if(TARGET platform_windows)
        target_link_libraries(platform_windows PUBLIC Qt6::Core Qt6::Gui Dwmapi)
        target_compile_definitions(platform_windows PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
    endif()
endif()

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

if(UI_FRAMEWORK_SOURCES)
    add_library(ui_framework STATIC ${UI_FRAMEWORK_SOURCES})
else()
    # 使用旧路径
    add_library(ui_framework STATIC
        components/base/UiComponent.hpp
        components/base/UiContent.hpp
        components/base/UiButton.hpp
        components/containers/UiRoot.h
        components/containers/UiRoot.cpp
        components/containers/UiBoxLayout.h
        components/containers/UiBoxLayout.cpp
        components/containers/UiPage.h
        components/containers/UiPage.cpp
        components/widgets/UiTopBar.h
        components/widgets/UiTopBar.cpp
        components/widgets/UiNav.h
        components/widgets/UiNav.cpp
        components/widgets/UiTabView.h
        components/widgets/UiTabView.cpp
        components/widgets/UiTreeList.h
        components/widgets/UiTreeList.cpp
    )
    target_include_directories(ui_framework PUBLIC 
        ${PROJECT_ROOT}
        ${PROJECT_ROOT}/components/base
        ${PROJECT_ROOT}/components/containers
        ${PROJECT_ROOT}/components/widgets
    )
endif()

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

if(MODEL_SOURCES)
    add_library(models STATIC ${MODEL_SOURCES})
else()
    # 使用旧路径
    add_library(models STATIC
        ThemeManager.h ThemeManager.cpp
        NavViewModel.h NavViewModel.cpp
        TabViewModel.h TabViewModel.cpp
        FormulaViewModel.h FormulaViewModel.cpp
    )
    target_include_directories(models PUBLIC ${PROJECT_ROOT})
endif()

target_link_libraries(models PUBLIC Qt6::Core Qt6::Gui)

# ========================================
# 业务视图库
# ========================================
file(GLOB VIEW_SOURCES
    "${SRC_DIR}/views/formula/*.cpp"
    "${SRC_DIR}/views/formula/*.h"
)

if(VIEW_SOURCES)
    add_library(business_views STATIC ${VIEW_SOURCES})
else()
    # 使用旧路径
    add_library(business_views STATIC
        views/UiFormulaView.h
        views/UiFormulaView.cpp
        views/UiFormulaDetail.h
        views/UiFormulaDetail.cpp
    )
    target_include_directories(business_views PUBLIC 
        ${PROJECT_ROOT}
        ${PROJECT_ROOT}/views
    )
endif()

target_link_libraries(business_views PUBLIC 
    ui_framework 
    models 
    core_rendering
)

# ========================================
# 主应用程序
# ========================================
# 检查新旧路径
if(EXISTS "${SRC_DIR}/app/main.cpp")
    set(APP_SOURCES
        ${SRC_DIR}/app/main.cpp
        ${SRC_DIR}/app/MainOpenGlWindow.h
        ${SRC_DIR}/app/MainOpenGlWindow.cpp
    )
else()
    set(APP_SOURCES
        main.cpp
        MainOpenGlWindow.h
        MainOpenGlWindow.cpp
    )
endif()

# 资源文件
if(EXISTS "${PROJECT_ROOT}/resources/resources.qrc")
    list(APPEND APP_SOURCES ${PROJECT_ROOT}/resources/resources.qrc)
else()
    list(APPEND APP_SOURCES resources.qrc)
endif()

add_executable(FangJia WIN32 ${APP_SOURCES})

target_link_libraries(FangJia PRIVATE
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

if(WIN32 AND TARGET platform_windows)
    target_link_libraries(FangJia PRIVATE platform_windows)
endif()

target_include_directories(FangJia PRIVATE ${PROJECT_ROOT})

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