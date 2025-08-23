cmake_minimum_required(VERSION 3.16)

# 如果支持，请为 MSVC 编译器启用热重载。
if (POLICY CMP0141)
  cmake_policy(SET CMP0141 NEW)
  set(CMAKE_MSVC_DEBUG_INFORMATION_FORMAT "$<IF:$<AND:$<C_COMPILER_ID:MSVC>,$<CXX_COMPILER_ID:MSVC>>,$<$<CONFIG:Debug,RelWithDebInfo>:EditAndContinue>,$<$<CONFIG:Debug,RelWithDebInfo>:ProgramDatabase>>")
endif()

project(Fangjia_Qt6_Cpp LANGUAGES CXX)

# 启用 Qt 自动处理
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

# 查找 Qt6 组件
find_package(Qt6 REQUIRED COMPONENTS Gui OpenGL Widgets Svg)


add_library(core_render STATIC
  RenderData.hpp
  Renderer.h Renderer.cpp
  IconLoader.h IconLoader.cpp)

target_link_libraries(core_render PUBLIC Qt6::OpenGL Qt6::Gui Qt6::Svg)
target_include_directories(core_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(ui_components STATIC
  UiComponent.hpp
  UiButton.hpp
  UiRoot.h UiRoot.cpp
  UiTopBar.h UiTopBar.cpp
  UiNav.h UiNav.cpp
  UiPage.h UiPage.cpp)  # 新增页面组件
target_link_libraries(ui_components PUBLIC core_render Qt6::Gui)
target_include_directories(ui_components PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

add_library(viewmodels STATIC
  ThemeManager.h ThemeManager.cpp
  NavViewModel.h NavViewModel.cpp)
target_link_libraries(viewmodels PUBLIC Qt6::Gui Qt6::Core)
target_include_directories(viewmodels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

set(app_sources
  main.cpp
  MainOpenGlWindow.h MainOpenGlWindow.cpp
  resources.qrc)

# Windows 平台：加入自定义 WindowChrome
if (WIN32)
  list(APPEND app_sources
    WinWindowChrome.h
    WinWindowChrome.cpp)
endif()

add_executable(Main WIN32 ${app_sources})
target_link_libraries(Main PRIVATE ui_components viewmodels core_render Qt6::Widgets)

# Windows 下链接 dwmapi（DWM 扩展）
if (WIN32)
  target_link_libraries(Main PRIVATE Dwmapi)
  target_compile_definitions(Main PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

# C++ 标准
set_property(TARGET Main PROPERTY CXX_STANDARD 23)
set_property(TARGET viewmodels PROPERTY CXX_STANDARD 23)
set_property(TARGET ui_components PROPERTY CXX_STANDARD 23)
set_property(TARGET core_render PROPERTY CXX_STANDARD 23)