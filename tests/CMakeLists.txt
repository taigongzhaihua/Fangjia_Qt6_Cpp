# Core-only demo without Qt dependencies
# 演示基于PR #21设计原则的核心架构

add_executable(fangjia_core_demo
    ../src/main.cpp
)

target_link_libraries(fangjia_core_demo PRIVATE
    fangjia_core
)

target_include_directories(fangjia_core_demo PRIVATE
    ../src
    ${boost_di_SOURCE_DIR}/include
)

# Core tests (不依赖Qt)
add_executable(fangjia_core_tests
    core_tests.cpp
)

target_link_libraries(fangjia_core_tests PRIVATE
    fangjia_core
)

target_include_directories(fangjia_core_tests PRIVATE
    ../src
    ${boost_di_SOURCE_DIR}/include
)

# 添加测试
enable_testing()
add_test(NAME CoreTests COMMAND fangjia_core_tests)