@echo off
REM Windows 客户端构建脚本

setlocal enabledelayedexpansion

echo === Windows 客户端构建脚本 ===
echo.

REM 检查依赖
echo 检查依赖...
where cmake >nul 2>nul
if %errorlevel% neq 0 (
    echo 错误: 未找到 cmake
    echo 请从 https://cmake.org/download/ 下载安装
    exit /b 1
)

where protoc >nul 2>nul
if %errorlevel% neq 0 (
    echo 警告: 未找到 protoc (Protocol Buffers 编译器)
    echo 请从 https://github.com/protocolbuffers/protobuf/releases 下载安装
    exit /b 1
)

REM 设置构建类型（默认 Release）
set BUILD_TYPE=Release
if "%~1" neq "" set BUILD_TYPE=%~1

echo 构建类型: %BUILD_TYPE%
echo.

REM 清理旧构建
if exist build_windows (
    echo 清理旧构建目录...
    rmdir /s /q build_windows
)

REM 创建构建目录
mkdir build_windows
cd build_windows

REM 检测 Visual Studio 版本
set CMAKE_GENERATOR=
if defined VisualStudioVersion (
    echo 检测到 Visual Studio 环境
) else (
    echo 检测可用的 Visual Studio 版本...
    if exist "C:\Program Files\Microsoft Visual Studio\2022\Community\MSBuild\Current\Bin\MSBuild.exe" (
        set CMAKE_GENERATOR="Visual Studio 17 2022"
        set CMAKE_ARCH=x64
    ) else if exist "C:\Program Files\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
        set CMAKE_GENERATOR="Visual Studio 16 2019"
        set CMAKE_ARCH=x64
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2019\Community\MSBuild\Current\Bin\MSBuild.exe" (
        set CMAKE_GENERATOR="Visual Studio 16 2019"
        set CMAKE_ARCH=x64
    ) else if exist "C:\Program Files (x86)\Microsoft Visual Studio\2017\Community\MSBuild\Current\Bin\MSBuild.exe" (
        set CMAKE_GENERATOR="Visual Studio 15 2017"
        set CMAKE_ARCH=x64
    ) else (
        echo 错误: 未找到 Visual Studio
        echo 请安装 Visual Studio 2017 或更高版本
        exit /b 1
    )
)

REM 配置 CMake
echo 配置 CMake (生成器: %CMAKE_GENERATOR%)...
cmake .. -G %CMAKE_GENERATOR% -A %CMAKE_ARCH% ^
    -DCMAKE_BUILD_TYPE=%BUILD_TYPE% ^
    -DBUILD_CLIENT=ON ^
    -DBUILD_SERVER=OFF ^
    -DBUILD_TESTS=OFF

if %errorlevel% neq 0 (
    echo.
    echo === CMake 配置失败 ===
    exit /b 1
)

REM 编译
echo.
echo 开始编译...
cmake --build . --config %BUILD_TYPE% --parallel

if %errorlevel% equ 0 (
    echo.
    echo === 构建成功 ===
    echo 可执行文件位置:
    dir /b bin\*.exe
) else (
    echo.
    echo === 构建失败 ===
    exit /b 1
)

cd ..
