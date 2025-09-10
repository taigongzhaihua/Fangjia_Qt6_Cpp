# Infrastructure layer - Qt相关的底层功能
# 基于PR #21的图形渲染和平台集成设计

# 暂时创建空的库，等核心测试通过后实现
add_library(fangjia_infrastructure STATIC 
    stub.cpp
)

target_link_libraries(fangjia_infrastructure PUBLIC 
    fangjia_core
)

if(ENABLE_QT_BUILD)
    target_link_libraries(fangjia_infrastructure PUBLIC
        Qt6::Core 
        Qt6::Gui 
        Qt6::OpenGL 
        Qt6::OpenGLWidgets
        Qt6::Widgets
        Qt6::Svg
    )
endif()

target_include_directories(fangjia_infrastructure PUBLIC 
    .
    ${boost_di_SOURCE_DIR}/include
)