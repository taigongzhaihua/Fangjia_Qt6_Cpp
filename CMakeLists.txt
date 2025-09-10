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

# 查找Qt6依赖组件 - 如果不可用则跳过Qt相关构建
find_package(Qt6 QUIET COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets Svg Sql)

if(Qt6_FOUND)
    message(STATUS "Qt6 found, version: ${Qt6_VERSION}")
    set(ENABLE_QT_BUILD ON)
else()
    message(STATUS "Qt6 not found, skipping Qt-dependent builds")
    set(ENABLE_QT_BUILD OFF)
endif()

# ========================================
# 外部依赖 - 依赖注入库
# ========================================

include(FetchContent)

# Fetch boost-ext/di (header-only dependency injection library)
FetchContent_Declare(
    boost_di
    GIT_REPOSITORY https://github.com/boost-ext/di.git
    GIT_TAG v1.3.0
    GIT_SHALLOW TRUE
)

FetchContent_MakeAvailable(boost_di)

# ========================================
# 新架构：基于PR #21设计原则的重新实现
# ========================================

# Core Infrastructure (不依赖Qt)
add_subdirectory(src/core)

# Core-only demo (不依赖Qt)  
add_subdirectory(tests)

if(ENABLE_QT_BUILD)
    # Infrastructure Layer - Graphics/Rendering (依赖Qt)
    add_subdirectory(src/infrastructure)

    # Presentation Layer - UI Framework
    add_subdirectory(src/presentation)

    # Application Layer
    add_subdirectory(src/app)

    # 主应用程序
    set(APP_SOURCES src/main.cpp)

    # 嵌入Qt资源文件（图标、字体等）
    if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc")
        list(APPEND APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc")
    endif()

    add_executable(FangJia WIN32 ${APP_SOURCES})

    # Link all dependencies to main application
    target_link_libraries(FangJia PRIVATE 
        fangjia_core
        fangjia_infrastructure
        fangjia_presentation
        fangjia_app
    )
    
    target_include_directories(FangJia PRIVATE 
        src
        ${boost_di_SOURCE_DIR}/include
    )
endif()

# ========================================
# 构建配置信息输出
# ========================================
message(STATUS "===========================================")
message(STATUS "Fangjia Qt6 C++ 项目配置完成")
message(STATUS "===========================================")
if(ENABLE_QT_BUILD)
    message(STATUS "Qt6版本: ${Qt6_VERSION}")
endif()
message(STATUS "源目录: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "==========================================")
