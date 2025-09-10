# Application layer - 应用程序逻辑
# 基于PR #21的页面管理和视图模型设计

# 暂时创建空的库，等UI框架层实现后再构建
add_library(fangjia_app STATIC 
    stub.cpp
)

target_link_libraries(fangjia_app PUBLIC 
    fangjia_core
    fangjia_infrastructure  
    fangjia_presentation
)

target_include_directories(fangjia_app PUBLIC 
    .
)