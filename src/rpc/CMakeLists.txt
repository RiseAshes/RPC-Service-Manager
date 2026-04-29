# RPC 通信模块
set(RPC_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(RPC_SOURCES
    ${RPC_SRC_DIR}/rpc_server.cpp
    ${RPC_SRC_DIR}/rpc_client.cpp
    ${RPC_SRC_DIR}/tcp_connection.cpp
    ${RPC_SRC_DIR}/message_codec.cpp
)

set(RPC_HEADERS
    ${RPC_SRC_DIR}/rpc_server.h
    ${RPC_SRC_DIR}/rpc_client.h
    ${RPC_SRC_DIR}/tcp_connection.h
    ${RPC_SRC_DIR}/message_codec.h
)

# 创建 RPC 客户端库
add_library(rpc_client STATIC ${RPC_SOURCES})

# 包含目录
target_include_directories(rpc_client PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_BINARY_DIR}
)

# 链接库
target_link_libraries(rpc_client PUBLIC
    common
    Threads::Threads
)


# 安装
install(TARGETS rpc_client
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)

# 创建测试程序
add_executable(rpc_test main_test.cpp)
target_link_libraries(rpc_test
    rpc_client
    common
    Threads::Threads
)
