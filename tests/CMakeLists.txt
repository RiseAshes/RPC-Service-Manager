# 测试模块
set(TESTS_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(TEST_SOURCES
    ${TESTS_SRC_DIR}/test_common.cpp
    ${TESTS_SRC_DIR}/test_rpc.cpp
)

# 为每个测试创建可执行文件
add_executable(test_common ${TESTS_SRC_DIR}/test_common.cpp)
target_link_libraries(test_common PRIVATE
    common
    gtest
    gtest_main
)

add_executable(test_rpc ${TESTS_SRC_DIR}/test_rpc.cpp)
target_link_libraries(test_rpc PRIVATE
    common
    rpc_client
    gtest
    gtest_main
)

# 添加测试
add_test(NAME TestCommon COMMAND test_common)
add_test(NAME TestRpc COMMAND test_rpc)

# 包含目录
target_include_directories(test_common PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/common
)

target_include_directories(test_rpc PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_SOURCE_DIR}/src/rpc
)
