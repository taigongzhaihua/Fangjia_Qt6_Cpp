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


# 核心渲染库
add_library(core_render STATIC
  RenderData.hpp
  Renderer.h Renderer.cpp
  IconLoader.h IconLoader.cpp)

target_link_libraries(core_render PUBLIC Qt6::OpenGL Qt6::Gui Qt6::Svg)
target_include_directories(core_render PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 通用UI组件库
add_library(ui_components STATIC
  # 基础接口
  components/base/UiComponent.hpp
  components/base/UiContent.hpp
  components/base/UiButton.hpp
  
  # 容器类
  components/containers/UiRoot.h
  components/containers/UiRoot.cpp
  components/containers/UiBoxLayout.h
  components/containers/UiBoxLayout.cpp
  components/containers/UiPage.h
  components/containers/UiPage.cpp
  
  # 复合控件
  components/widgets/UiTopBar.h
  components/widgets/UiTopBar.cpp
  components/widgets/UiNav.h
  components/widgets/UiNav.cpp
  components/widgets/UiTabView.h
  components/widgets/UiTabView.cpp
  components/widgets/UiTreeList.h
  components/widgets/UiTreeList.cpp
)

# 重要：设置包含目录，这样可以直接 #include "UiComponent.hpp" 而不需要路径
target_include_directories(ui_components PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/components/base
  ${CMAKE_CURRENT_SOURCE_DIR}/components/containers
  ${CMAKE_CURRENT_SOURCE_DIR}/components/widgets
)

target_link_libraries(ui_components PUBLIC core_render Qt6::Gui)

# 视图模型库
add_library(viewmodels STATIC
  ThemeManager.h ThemeManager.cpp
  NavViewModel.h NavViewModel.cpp
  TabViewModel.h TabViewModel.cpp
  FormulaViewModel.h FormulaViewModel.cpp)

target_link_libraries(viewmodels PUBLIC Qt6::Gui Qt6::Core)
target_include_directories(viewmodels PUBLIC ${CMAKE_CURRENT_SOURCE_DIR})

# 业务视图库
add_library(ui_views STATIC
  views/UiFormulaView.h
  views/UiFormulaView.cpp
  views/UiFormulaDetail.h
  views/UiFormulaDetail.cpp
)

target_link_libraries(ui_views PUBLIC ui_components viewmodels core_render)
target_include_directories(ui_views PUBLIC 
  ${CMAKE_CURRENT_SOURCE_DIR}/views
  ${CMAKE_CURRENT_SOURCE_DIR}  # 为了能找到 viewmodels
)

# 主程序
set(app_sources
  main.cpp
  MainOpenGlWindow.h MainOpenGlWindow.cpp
  resources.qrc)

# Windows 平台
if (WIN32)
  list(APPEND app_sources
    WinWindowChrome.h
    WinWindowChrome.cpp)
endif()

add_executable(FangJia WIN32 ${app_sources})
target_link_libraries(FangJia PRIVATE 
  ui_views        # 业务视图
  ui_components   # 通用组件
  viewmodels      # 视图模型
  core_render     # 渲染核心
  Qt6::Widgets
)

# 确保主程序能找到所有头文件
target_include_directories(FangJia PRIVATE
  ${CMAKE_CURRENT_SOURCE_DIR}
  ${CMAKE_CURRENT_SOURCE_DIR}/components/base
  ${CMAKE_CURRENT_SOURCE_DIR}/components/containers
  ${CMAKE_CURRENT_SOURCE_DIR}/components/widgets
  ${CMAKE_CURRENT_SOURCE_DIR}/views
)

# Windows 下链接 dwmapi
if (WIN32)
  target_link_libraries(FangJia PRIVATE Dwmapi)
  target_compile_definitions(FangJia PRIVATE WIN32_LEAN_AND_MEAN NOMINMAX)
endif()

# C++ 标准
set_property(TARGET FangJia PROPERTY CXX_STANDARD 23)
set_property(TARGET viewmodels PROPERTY CXX_STANDARD 23)
set_property(TARGET ui_components PROPERTY CXX_STANDARD 23)
set_property(TARGET ui_views PROPERTY CXX_STANDARD 23)
set_property(TARGET core_render PROPERTY CXX_STANDARD 23)