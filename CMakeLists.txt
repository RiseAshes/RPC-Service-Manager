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

# 平台特定配置
if(WIN32)
    message(STATUS "构建目标: Windows 客户端")
    set(BUILD_CLIENT ON CACHE BOOL "构建 Qt 客户端" FORCE)
    set(BUILD_SERVER OFF CACHE BOOL "构建服务端" FORCE)
elseif(UNIX AND NOT APPLE)
    message(STATUS "构建目标: Linux 服务端")
    set(BUILD_CLIENT OFF CACHE BOOL "构建 Qt 客户端" FORCE)
    set(BUILD_SERVER ON CACHE BOOL "构建服务端" FORCE)
else()
    message(STATUS "构建目标: 全平台")
    set(BUILD_CLIENT ON CACHE BOOL "构建 Qt 客户端")
    set(BUILD_SERVER ON CACHE BOOL "构建服务端")
endif()

# 查找依赖
find_package(Threads REQUIRED)
find_package(Protobuf REQUIRED)

# 公共模块（所有平台都需要）
add_subdirectory(src/common)

# 服务端（Linux）
if(BUILD_SERVER)
    message(STATUS "配置服务端组件")
    # MySQL 连接库（服务端需要）
    find_package(MySQL QUIET)
    if(NOT MySQL_FOUND)
        message(WARNING "未找到 MySQL，将使用头文件模式")
        # 设置 MySQL 路径（如需要手动指定）
        # set(MYSQL_INCLUDE_DIR "path/to/mysql/include")
        # set(MYSQL_LIBRARY "path/to/mysql/lib/libmysqlclient.so")
    endif()

    # spdlog 日志库
    find_package(spdlog QUIET)
    if(NOT spdlog_FOUND)
        message(STATUS "未找到 spdlog，使用 FetchContent 下载")
        include(FetchContent)
        FetchContent_Declare(
            spdlog
            GIT_REPOSITORY https://github.com/gabime/spdlog.git
            GIT_TAG v1.12.0
        )
        FetchContent_MakeAvailable(spdlog)
    endif()

    add_subdirectory(src/registry)
    add_subdirectory(src/rpc)
endif()

# 客户端（Windows）
if(BUILD_CLIENT)
    message(STATUS "配置客户端组件")
    # 查找 Qt（自动检测 5 或 6）
    find_package(Qt6 QUIET COMPONENTS Core Widgets Network)
    if(Qt6_FOUND)
        message(STATUS "使用 Qt 6")
        set(QT_VERSION_MAJOR 6)
    else()
        find_package(Qt5 QUIET REQUIRED COMPONENTS Core Widgets Network)
        if(Qt5_FOUND)
            message(STATUS "使用 Qt 5")
            set(QT_VERSION_MAJOR 5)
        else()
            message(FATAL_ERROR "未找到 Qt 5 或 Qt 6")
        endif()
    endif()

    add_subdirectory(src/client)
endif()

# 测试（可选）
option(BUILD_TESTS "构建测试" OFF)
if(BUILD_TESTS)
    message(STATUS "配置测试组件")
    enable_testing()
    include(FetchContent)
    FetchContent_Declare(
        googletest
        GIT_REPOSITORY https://github.com/google/googletest.git
        GIT_TAG release-1.12.1
    )
    FetchContent_MakeAvailable(googletest)
    add_subdirectory(tests)
endif()

# 打印配置信息
message(STATUS "=== 构建配置 ===")
message(STATUS "C++ 标准: ${CMAKE_CXX_STANDARD}")
message(STATUS "构建类型: ${CMAKE_BUILD_TYPE}")
message(STATUS "构建客户端: ${BUILD_CLIENT}")
message(STATUS "构建服务端: ${BUILD_SERVER}")
message(STATUS "构建测试: ${BUILD_TESTS}")
message(STATUS "Protobuf 版本: ${Protobuf_VERSION}")
if(BUILD_CLIENT)
    message(STATUS "Qt 版本: ${QT_VERSION_MAJOR}")
endif()
message(STATUS "===============")
