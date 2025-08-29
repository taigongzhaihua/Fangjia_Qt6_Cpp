cmake_minimum_required(VERSION 3.16)

# MSVC热重载支持：启用EditAndContinue调试信息格式
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(Fangjia_Qt6_Cpp LANGUAGES CXX)

# Qt构建配置
set(CMAKE_AUTOMOC ON)         # 自动运行MOC（元对象编译器）
set(CMAKE_AUTOUIC ON)         # 自动处理UI文件
set(CMAKE_AUTORCC ON)         # 自动处理资源文件
set(CMAKE_CXX_STANDARD 23)    # 使用C++23标准
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# 查找Qt6依赖组件
find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL Widgets Svg)

# 项目目录结构配置
set(PROJECT_ROOT ${CMAKE_CURRENT_SOURCE_DIR})
set(SRC_DIR ${PROJECT_ROOT}/src)

# ========================================
# 全局包含目录配置
# ========================================
# 设置源代码包含路径，支持分层架构的直接引用
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
# 核心渲染库：OpenGL渲染器和图标缓存
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
# 配置管理库：应用程序设置和主题持久化
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
# 平台特定库：Windows DWM集成等
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
# UI框架库：自绘组件、布局容器、声明式API
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

# ========================================
# 数据模型库：ViewModel和业务逻辑
# ========================================
file(GLOB MODEL_SOURCES
    "${SRC_DIR}/models/*.cpp"
    "${SRC_DIR}/models/*.h"
    "${SRC_DIR}/core/vm/*.cpp"
    "${SRC_DIR}/core/vm/*.hpp"
)

add_library(models STATIC ${MODEL_SOURCES})
target_link_libraries(models PUBLIC Qt6::Core Qt6::Gui)

# ========================================
# 业务视图库：具体页面和功能模块
# ========================================
file(GLOB VIEW_SOURCES
    "${SRC_DIR}/views/formula/*.cpp"
    "${SRC_DIR}/views/formula/*.h"
)
file(GLOB PAGE_SOURCES
    "${SRC_DIR}/views/pages/*.cpp"
    "${SRC_DIR}/views/pages/*.h"
)

# 合并视图源文件
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

# ========================================
# 主应用程序可执行文件
# ========================================
set(APP_SOURCES
    ${SRC_DIR}/app/main.cpp
    ${SRC_DIR}/app/MainOpenGlWindow.h
    ${SRC_DIR}/app/MainOpenGlWindow.cpp
)

# 嵌入Qt资源文件（图标、字体等）
if(EXISTS "${PROJECT_ROOT}/resources/resources.qrc")
    list(APPEND APP_SOURCES ${PROJECT_ROOT}/resources/resources.qrc)
endif()

add_executable(FangJia WIN32 ${APP_SOURCES})

# 链接所有依赖库
set(APP_LIBS
    business_views    # 业务视图层
    ui_framework     # UI框架层  
    models          # 数据模型层
    core_rendering  # 核心渲染层
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::Widgets
    Qt6::Svg
)

# 条件性链接平台特定库
if(TARGET core_config)
    list(APPEND APP_LIBS core_config)
endif()

if(WIN32 AND TARGET platform_windows)
    list(APPEND APP_LIBS platform_windows)
endif()

target_link_libraries(FangJia PRIVATE ${APP_LIBS})

# ========================================
# 测试子项目
# ========================================
add_subdirectory(tests)

# ========================================
# 示例子项目（可选）
# ========================================
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/examples")
    add_subdirectory(examples)
endif()

# ========================================
# 构建配置信息输出
# ========================================
message(STATUS "===========================================")
message(STATUS "Fangjia Qt6 C++项目配置完成")
message(STATUS "===========================================")
message(STATUS "Qt6版本: ${Qt6_VERSION}")
message(STATUS "源目录: ${SRC_DIR}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "==========================================")