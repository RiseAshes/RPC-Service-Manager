# RPC 通信模块
set(RPC_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(RPC_SOURCES
    ${RPC_SRC_DIR}/rpc_server.cpp
    ${RPC_SRC_DIR}/rpc_client.cpp
    ${RPC_SRC_DIR}/connection_pool.cpp
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

# 链接 spdlog
if(TARGET spdlog::spdlog)
    target_link_libraries(rpc_client PRIVATE spdlog::spdlog)
else()
    target_link_libraries(rpc_client PRIVATE spdlog)
endif()

# 安装
install(TARGETS rpc_client
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
