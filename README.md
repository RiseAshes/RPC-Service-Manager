# 跨平台分布式 RPC 服务管理系统

## 项目简介

这是一个用 C++11 实现的跨平台分布式 RPC 服务管理系统，支持服务注册、发现和远程调用。

## 技术栈

- **语言**: C++11
- **网络**: TCP 套接字
- **序列化**: Protocol Buffers
- **数据库**: MySQL
- **客户端**: Qt 5/6
- **构建**: CMake 3.14+

## 目录结构

```
├── src/                    # 源代码目录
│   ├── common/            # 公共工具和协议
│   ├── registry/          # 服务注册中心
│   ├── rpc/               # RPC 通信模块
│   └── client/            # Qt 管理客户端
├── proto/                 # Protocol Buffers 定义
├── sql/                   # 数据库脚本
├── tests/                 # 测试代码
├── include/               # 公共头文件
├── ui/                   # Qt UI 文件
├── resources/            # 资源文件
└── CMakeLists.txt        # 构建配置
```

## 快速开始

### 环境要求

#### Linux 服务器端
- CMake 3.14+
- Protocol Buffers 编译器 (protoc)
- GCC 7+ 或 Clang 6+
- MySQL 开发库 (可选)
- spdlog (可选，会自动下载)

#### Windows 客户端
- Visual Studio 2017+
- Protocol Buffers 编译器 (protoc)
- Qt 5.12+ 或 Qt 6
- CMake 3.14+

### 构建步骤

#### Linux 服务端构建

```bash
# 1. 克隆代码
git clone <repository-url>
cd project1

# 2. 给构建脚本执行权限
chmod +x build_linux.sh

# 3. 构建
./build_linux.sh

# 或指定 Debug 模式
./build_linux.sh Debug
```

#### Windows 客户端构建

```bat
# 1. 打开 Visual Studio 开发者命令提示
# 2. 克隆代码
git clone <repository-url>
cd project1

# 3. 构建
build_windows.bat

# 或指定 Debug 模式
build_windows.bat Debug
```

### 运行服务

#### Linux 服务端

```bash
# 启动注册中心（默认端口 8080）
./bin/registry_server --port 8080

# 查看帮助
./bin/registry_server --help
```

#### Windows 客户端

```bat
# 启动 Qt 客户端
bin/qt_client.exe
```

## 数据库设置

系统需要 MySQL 进行数据持久化：

```sql
# 连接到 MySQL
mysql -u root -p

# 创建数据库和表
source sql/init_tables.sql
```

## 使用说明

### 服务注册

1. 服务启动时向注册中心注册
2. 定期发送心跳保持连接
3. 注册中心维护服务状态

### 服务发现

1. 客户端查询服务列表
2. 获取可用服务端点
3. 支持负载均衡选择

### RPC 调用

1. 客户端发起 RPC 请求
2. 注册中心路由到服务实例
3. 返回响应结果

## 开发指南

### 编码规范

- 使用 C++11 标准
- 使用智能指针管理内存
- 遵循 RAII 模式
- 使用互斥锁保护共享数据
- 使用 Protocol Buffers 定义消息格式

### 调试技巧

- 编译时开启 Debug 模式
- 使用 spdlog 输出详细日志
- 检查 MySQL 连接参数
- 验证网络端口可用性

### 常见问题

1. **CMake 配置失败**
   - 检查依赖是否安装
   - 确认编译器版本符合要求

2. **编译错误**
   - 检查文件路径是否正确
   - 验证 Protocol Buffers 是否生成

3. **运行时错误**
   - 确认端口未被占用
   - 检查数据库连接配置

## 版本信息

- 当前版本: 0.1.0
- 开发状态: 基础设施搭建阶段