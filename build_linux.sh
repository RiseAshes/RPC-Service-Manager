#!/bin/bash
# Linux 服务端构建脚本

set -e  # 遇到错误立即退出

# 颜色输出
RED='\033[0;31m'
GREEN='\033[0;32m'
YELLOW='\033[1;33m'
NC='\033[0m' # No Color

echo -e "${GREEN}=== Linux 服务端构建脚本 ===${NC}"

# 检查依赖
echo -e "${YELLOW}检查依赖...${NC}"
if ! command -v cmake &> /dev/null; then
    echo -e "${RED}错误: 未找到 cmake${NC}"
    exit 1
fi

if ! command -v protoc &> /dev/null; then
    echo -e "${RED}错误: 未找到 protoc (Protocol Buffers 编译器)${NC}"
    exit 1
fi

# 设置构建类型
BUILD_TYPE=${1:-Release}
echo -e "${YELLOW}构建类型: ${BUILD_TYPE}${NC}"

# 清理旧构建
if [ -d "build_linux" ]; then
    echo -e "${YELLOW}清理旧构建目录...${NC}"
    rm -rf build_linux
fi

# 创建构建目录
mkdir -p build_linux
cd build_linux

# 配置 CMake
echo -e "${YELLOW}配置 CMake...${NC}"
cmake .. \
    -DCMAKE_BUILD_TYPE=${BUILD_TYPE} \
    -DBUILD_CLIENT=OFF \
    -DBUILD_SERVER=ON \
    -DBUILD_TESTS=OFF

# 编译
echo -e "${YELLOW}开始编译...${NC}"
NPROC=$(nproc)
make -j${NPROC}

# 检查编译结果
if [ $? -eq 0 ]; then
    echo -e "${GREEN}=== 构建成功 ===${NC}"
    echo -e "${GREEN}可执行文件位置:${NC}"
    ls -lh bin/
else
    echo -e "${RED}=== 构建失败 ===${NC}"
    exit 1
fi
