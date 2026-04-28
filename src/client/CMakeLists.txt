# Qt 管理客户端
set(CLIENT_SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR})
set(CLIENT_UI_DIR ${CMAKE_CURRENT_SOURCE_DIR}/ui)

set(CLIENT_SOURCES
    ${CLIENT_SRC_DIR}/main.cpp
    ${CLIENT_SRC_DIR}/main_window.cpp
    ${CLIENT_SRC_DIR}/service_model.cpp
    ${CLIENT_SRC_DIR}/rpc_tester.cpp
)

set(CLIENT_HEADERS
    ${CLIENT_SRC_DIR}/main_window.h
    ${CLIENT_SRC_DIR}/service_model.h
    ${CLIENT_SRC_DIR}/rpc_tester.h
)

set(CLIENT_UI_FILES
    ${CLIENT_UI_DIR}/main_window.ui
)

# 自动处理 UI 文件
if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_ui(UIS_SOURCES ${CLIENT_UI_FILES})
else()
    qt5_wrap_ui(UIS_SOURCES ${CLIENT_UI_FILES})
endif()

# 自动处理 MOC 文件
if(QT_VERSION_MAJOR EQUAL 6)
    qt_add_qml_module(qt_client
        URI rpc.client
        VERSION 1.0
        SOURCES ${CLIENT_SOURCES} ${CLIENT_HEADERS}
    )
    set(QT_LIBRARIES Qt6::Core Qt6::Widgets Qt6::Network)
else()
    qt5_add_resources(QRC_SOURCES resources.qrc)
    set(QT_LIBRARIES Qt5::Core Qt5::Widgets Qt5::Network)
endif()

# 创建可执行文件
add_executable(qt_client
    ${CLIENT_SOURCES}
    ${CLIENT_HEADERS}
    ${UIS_SOURCES}
)

# 启用 Qt 的 MOC、RCC、UIC
set_target_properties(qt_client PROPERTIES
    AUTOMOC ON
    AUTORCC ON
    AUTOUIC ON
)

# 包含目录
target_include_directories(qt_client PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/src/common
    ${CMAKE_CURRENT_BINARY_DIR}
)

# 链接库
target_link_libraries(qt_client PRIVATE
    common
    ${QT_LIBRARIES}
)

# Windows 特定设置
if(WIN32)
    set_target_properties(qt_client PROPERTIES
        WIN32_EXECUTABLE ON
    )
endif()

# 安装
if(WIN32)
    install(TARGETS qt_client DESTINATION .)
else()
    install(TARGETS qt_client DESTINATION bin)
endif()
