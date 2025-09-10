# Presentation layer - UI框架
# 基于PR #21的组件和事件处理设计

# 暂时创建空的库，等核心和基础设施层实现后再构建
add_library(fangjia_presentation STATIC 
    stub.cpp
)

target_link_libraries(fangjia_presentation PUBLIC 
    fangjia_core
    fangjia_infrastructure
)

target_include_directories(fangjia_presentation PUBLIC 
    .
)