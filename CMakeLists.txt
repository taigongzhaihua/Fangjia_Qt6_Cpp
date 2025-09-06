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
find_package(Qt6 REQUIRED COMPONENTS Core Gui OpenGL OpenGLWidgets Widgets Svg Sql)

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
# 模块化目标构建
# ========================================

# Infrastructure Layer - Graphics/Rendering
file(GLOB FJ_INFRA_GFX_SOURCES
    "infrastructure/gfx/*.cpp"
    "infrastructure/gfx/*.h"
    "infrastructure/gfx/*.hpp"
)

add_library(fj_infra_gfx STATIC ${FJ_INFRA_GFX_SOURCES})
target_link_libraries(fj_infra_gfx PUBLIC 
    Qt6::Core 
    Qt6::Gui 
    Qt6::OpenGL 
    Qt6::Svg
)
target_include_directories(fj_infra_gfx PUBLIC infrastructure/gfx)

# Infrastructure Layer - Platform (Windows-specific)
if(WIN32)
    file(GLOB FJ_INFRA_PLATFORM_SOURCES
        "infrastructure/platform/windows/*.cpp"
        "infrastructure/platform/windows/*.h"
    )
    
    if(FJ_INFRA_PLATFORM_SOURCES)
        add_library(fj_infra_platform STATIC ${FJ_INFRA_PLATFORM_SOURCES})
        target_link_libraries(fj_infra_platform PUBLIC Qt6::Core Qt6::Gui Dwmapi)
        target_compile_definitions(fj_infra_platform PUBLIC WIN32_LEAN_AND_MEAN NOMINMAX)
        target_include_directories(fj_infra_platform PUBLIC 
            infrastructure/platform
            infrastructure/platform/windows
        )
    endif()
endif()

# Data Layer - Configuration and data sources
file(GLOB FJ_DATA_SOURCES
    "data/sources/local/*.cpp"
    "data/sources/local/*.h"
    "data/repositories/*.cpp"
    "data/repositories/*.h"
    "data/utils/*.cpp"
    "data/utils/*.h"
)

add_library(fj_data STATIC ${FJ_DATA_SOURCES})
target_link_libraries(fj_data PUBLIC 
    fj_domain
    Qt6::Core
    Qt6::Widgets
    Qt6::Sql
)
target_include_directories(fj_data PUBLIC 
    data/sources
    data/sources/local
    data/repositories
    data/utils
    domain
    ${boost_di_SOURCE_DIR}/include
)

# Presentation Layer - Binding interfaces (new)
file(GLOB FJ_PRESENTATION_BINDING_SOURCES
    "presentation/binding/*.h"
    "presentation/binding/*.hpp"
    "presentation/binding/*.cpp"
)
add_library(fj_presentation_binding STATIC ${FJ_PRESENTATION_BINDING_SOURCES})
target_link_libraries(fj_presentation_binding PUBLIC Qt6::Core)
target_include_directories(fj_presentation_binding PUBLIC presentation/binding)

# Presentation Layer - UI Framework (base, containers, widgets, declarative)
file(GLOB FJ_PRESENTATION_UI_SOURCES
    "presentation/ui/base/*.hpp"
    "presentation/ui/base/*.cpp"
    "presentation/ui/base/*.h"
    "presentation/ui/containers/*.cpp"
    "presentation/ui/containers/*.h"
    "presentation/ui/widgets/*.cpp"
    "presentation/ui/widgets/*.h"
    "presentation/ui/declarative/*.cpp"
    "presentation/ui/declarative/*.h"
)

add_library(fj_presentation_ui STATIC ${FJ_PRESENTATION_UI_SOURCES})
# Link to binding to access interfaces only
target_link_libraries(fj_presentation_ui PUBLIC 
    fj_infra_gfx 
    fj_presentation_binding
    Qt6::Core 
    Qt6::Gui 
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Widgets
)
# Remove dependency on presentation/viewmodels (Stage 5)
target_include_directories(fj_presentation_ui PUBLIC 
    presentation/ui
    presentation/ui/base
    presentation/ui/containers
    presentation/ui/widgets
    presentation/ui/declarative
)

# Presentation Layer - ViewModels
file(GLOB FJ_PRESENTATION_VM_SOURCES
    "presentation/viewmodels/*.cpp"
    "presentation/viewmodels/*.h"
    "presentation/viewmodels/*.hpp"
)

add_library(fj_presentation_vm STATIC ${FJ_PRESENTATION_VM_SOURCES})
# VMs implement binding interfaces
target_link_libraries(fj_presentation_vm PUBLIC 
    fj_domain
    fj_presentation_binding
    Qt6::Core 
    Qt6::Gui
)
target_include_directories(fj_presentation_vm PUBLIC 
    presentation/viewmodels
    domain
    ${boost_di_SOURCE_DIR}/include
)

# Presentation Layer - Pages and Views
file(GLOB FJ_PRESENTATION_PAGES_SOURCES
    "presentation/pages/*.cpp"
    "presentation/pages/*.h"
    "presentation/views/formula/*.cpp"
    "presentation/views/formula/*.h"
)

add_library(fj_presentation_pages STATIC ${FJ_PRESENTATION_PAGES_SOURCES})
target_link_libraries(fj_presentation_pages PUBLIC 
    fj_presentation_ui 
    fj_presentation_vm 
    fj_infra_gfx
    fj_data
)
target_include_directories(fj_presentation_pages PUBLIC 
    presentation/pages
    presentation/views
    presentation/views/formula
    data/sources/local
    data/repositories
    domain
)

# Domain Layer - Core business logic and abstractions (Pure C++)
file(GLOB FJ_DOMAIN_SOURCES
    "domain/entities/*.h"
    "domain/repositories/*.h" 
    "domain/services/*.cpp"
    "domain/services/*.h"
    "domain/usecases/*.cpp"
    "domain/usecases/*.h"
    "domain/ServiceRegistry.cpp"
    "domain/ServiceRegistry.h"
)

add_library(fj_domain STATIC ${FJ_DOMAIN_SOURCES})
target_include_directories(fj_domain PUBLIC 
    domain
    domain/entities
    domain/repositories
    domain/services
    domain/usecases
    ${boost_di_SOURCE_DIR}/include
)

# Application - Main executable
file(GLOB APP_SOURCES
    "apps/fangjia/*.cpp"
    "apps/fangjia/*.h"
)

# 嵌入Qt资源文件（图标、字体等）
if(EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc")
    list(APPEND APP_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/resources/resources.qrc")
endif()

add_executable(FangJia WIN32 ${APP_SOURCES})

# Link all dependencies to main application
set(APP_LIBS
    fj_presentation_pages
    fj_presentation_ui
    fj_presentation_vm
    fj_infra_gfx
    fj_domain
    fj_data
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::OpenGLWidgets
    Qt6::Widgets
    Qt6::Svg
)

# Conditionally link platform-specific libraries
if(WIN32 AND TARGET fj_infra_platform)
    list(APPEND APP_LIBS fj_infra_platform)
endif()

target_link_libraries(FangJia PRIVATE ${APP_LIBS})
target_include_directories(FangJia PRIVATE 
    apps/fangjia
    data/sources/local
    data/repositories
    domain
    ${boost_di_SOURCE_DIR}/include
)

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
message(STATUS "源目录: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "C++标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "==========================================")
