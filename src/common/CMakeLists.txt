# 公共模块
set(COMMON_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(COMMON_PROTO_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../../proto)

# Protocol Buffers 定义
set(PROTO_FILES ${COMMON_PROTO_DIR}/rpc_protocol.proto)

# 生成 Protocol Buffers 源文件
protobuf_generate_cpp(PROTO_SRCS PROTO_HDRS ${PROTO_FILES})

# 创建公共库
add_library(common STATIC
    ${PROTO_SRCS}
)

# 包含目录
target_include_directories(common PUBLIC
    ${CMAKE_CURRENT_BINARY_DIR}
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${Protobuf_INCLUDE_DIRS}
)

# 链接 Protobuf
target_link_libraries(common PUBLIC
    protobuf::libprotobuf
)

# 安装
install(TARGETS common
    ARCHIVE DESTINATION lib
    LIBRARY DESTINATION lib
)
install(FILES ${PROTO_HDRS} DESTINATION include/common)
