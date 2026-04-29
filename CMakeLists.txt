# 跨平台分布式 RPC 服务管理系统
cmake_minimum_required(VERSION 3.14)
project(RPC-Service-Manager VERSION 0.1.0 LANGUAGES CXX)

# 设置 C++ 标准
set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# 跨平台编译选项
if(MSVC)
    add_compile_options(/W4 /utf-8)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /MP")
else()
    add_compile_options(-Wall -Wextra -Wpedantic)
    set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -g -O0")
    set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -O3")
endif()

# 输出目录配置
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)

# 查找依赖
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

# 公共模块（所有平台都需要）
add_subdirectory(src/common)

# RPC 通信模块（已实现）
add_subdirectory(src/rpc)

# 打印配置信息
message(STATUS "=== 构建配置 ===")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "Protobuf 版本: ${Protobuf_VERSION}")
message(STATUS "===============")