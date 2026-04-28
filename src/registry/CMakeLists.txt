# 服务注册中心
set(REGISTRY_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})

set(REGISTRY_SOURCES
    ${REGISTRY_SRC_DIR}/registry_server.cpp
    ${REGISTRY_SRC_DIR}/service_node.cpp
    ${REGISTRY_SRC_DIR}/mysql_storage.cpp
    ${REGISTRY_SRC_DIR}/main.cpp
)

# 创建可执行文件
add_executable(registry_server ${REGISTRY_SOURCES})

# 包含目录
target_include_directories(registry_server PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_BINARY_DIR}  # Protocol Buffers 生成文件
)

# 链接库
target_link_libraries(registry_server PRIVATE
    common
    Threads::Threads
)

# 链接 spdlog
if(TARGET spdlog::spdlog)
    target_link_libraries(registry_server PRIVATE spdlog::spdlog)
else()
    target_link_libraries(registry_server PRIVATE spdlog)
endif()

# 链接 MySQL
if(MySQL_FOUND)
    target_include_directories(registry_server PRIVATE ${MYSQL_INCLUDE_DIR})
    target_link_libraries(registry_server PRIVATE ${MYSQL_LIBRARY})
endif()

# 安装
install(TARGETS registry_server DESTINATION bin)
