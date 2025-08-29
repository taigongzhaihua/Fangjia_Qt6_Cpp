# MVVM Binding Example
cmake_minimum_required(VERSION 3.16)

# 查找 Qt6
find_package(Qt6 REQUIRED COMPONENTS Core Gui)

# 源文件
set(EXAMPLE_SOURCES
    main.cpp
)

# 创建可执行文件
add_executable(mvvm_binding_example ${EXAMPLE_SOURCES})

# 包含目录
target_include_directories(mvvm_binding_example PRIVATE
    ${CMAKE_SOURCE_DIR}
)

# 链接库
target_link_libraries(mvvm_binding_example PRIVATE
    Qt6::Core
    Qt6::Gui
    fj_presentation_ui
    fj_infra_gfx
)

# 设置输出目录
set_target_properties(mvvm_binding_example PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/examples
)