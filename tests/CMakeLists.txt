# 测试配置
enable_testing()

# 查找 Qt Test
find_package(Qt6 REQUIRED COMPONENTS Test Core Gui OpenGL Widgets)

# 收集所有测试源文件
set(TEST_SOURCES
    test_main.cpp
)

# 创建测试可执行文件
add_executable(FangJia_Tests ${TEST_SOURCES})

# 包含目录
target_include_directories(FangJia_Tests PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src
    ${CMAKE_SOURCE_DIR}/src/core
    ${CMAKE_SOURCE_DIR}/src/framework
    ${CMAKE_SOURCE_DIR}/src/models
)

# 链接库
target_link_libraries(FangJia_Tests PRIVATE
    Qt6::Test
    Qt6::Core
    Qt6::Gui
    Qt6::OpenGL
    Qt6::Widgets
    core_rendering
    ui_framework
    models
)

# 条件链接
if(TARGET core_config)
    target_link_libraries(FangJia_Tests PRIVATE core_config)
endif()

if(WIN32 AND TARGET platform_windows)
    target_link_libraries(FangJia_Tests PRIVATE platform_windows)
endif()

# 应用警告标志（如果可用）
if(COMMAND apply_fangjia_warnings)
    apply_fangjia_warnings(FangJia_Tests)
endif()

# 添加测试
add_test(NAME AllTests COMMAND FangJia_Tests)

# 设置测试环境
set_tests_properties(AllTests PROPERTIES
    ENVIRONMENT "QT_QPA_PLATFORM=offscreen;QT_LOGGING_RULES=qt.qpa.gl=false"
    TIMEOUT 60
)

# 代码覆盖率（可选）
if(CMAKE_BUILD_TYPE STREQUAL "Debug" AND CMAKE_COMPILER_IS_GNUCXX)
    target_compile_options(FangJia_Tests PRIVATE --coverage)
    target_link_options(FangJia_Tests PRIVATE --coverage)
endif()

# 自定义测试目标
add_custom_target(run_tests
    COMMAND ${CMAKE_CTEST_COMMAND} --output-on-failure
    DEPENDS FangJia_Tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running all tests..."
)

# 性能测试目标
add_custom_target(perf_tests
    COMMAND FangJia_Tests -o perf_report.txt,txt -maxwarnings 0 TestIntegration::testPerformance
    DEPENDS FangJia_Tests
    WORKING_DIRECTORY ${CMAKE_BINARY_DIR}
    COMMENT "Running performance tests..."
)