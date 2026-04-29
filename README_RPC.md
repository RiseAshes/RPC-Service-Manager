# RPC 通信模块测试说明

## 编译和运行

### 1. 编译项目

```bash
# Linux
mkdir build && cd build
cmake .. -DCMAKE_BUILD_TYPE=Debug
make -j$(nproc)

# Windows
mkdir build && cd build
cmake .. -G "Visual Studio 16 2019" -A x64
cmake --build . --config Debug
```

### 2. 运行测试

#### 服务器模式
```bash
# 在终端 1 启动服务器（默认端口 8080）
./bin/rpc_test

# 指定端口
./bin/rpc_test 8080 server
```

#### 客户端模式
```bash
# 在终端 2 启动客户端（连接到 localhost:8080）
./bin/rpc_test

# 连接到指定端口
./bin/rpc_test 8080 client
```

## 功能测试

### 服务器功能
- [x] 监听指定端口
- [x] 接受客户端连接
- [x] 注册服务（TestEchoService）
- [x] 处理 RPC 请求
- [x] 统计请求信息

### 客户端功能
- [x] 连接到服务器
- [x] 同步 RPC 调用
- [x] 异步 RPC 调用
- [x] 批量 RPC 调用
- [x] 请求 ID 管理

### 测试服务
1. **echo** - 回显消息
   ```cpp
   Request: "Hello, RPC!"
   Response: "Echo: Hello, RPC!"
   ```

2. **echoNTimes** - 多次回显
   ```cpp
   Request: "Test,3"
   Response: "Echo[1]: Test\nEcho[2]: Test\nEcho[3]: Test"
   ```

3. **getServerInfo** - 获取服务器信息
   ```cpp
   Response: "RPC-Service-Manager v0.1.0 Running"
   ```

## 消息编解码测试

测试程序包含对消息编解码器的测试：
- [x] 编码消息（添加消息头）
- [x] 解码消息（验证消息头）
- [x] 缓冲区处理
- [x] 多消息提取

## 故障排查

### 常见问题
1. **编译错误：找不到 rpc_protocol.pb.h**
   - 确保已安装 Protocol Buffers
   - 运行 `cmake` 时会自动生成头文件

2. **连接失败**
   - 检查端口是否被占用
   - 确保服务器已启动
   - 检查防火墙设置

3. **超时错误**
   - 增加超时时间参数
   - 检查网络延迟

### 调试建议
- 使用 `strace` (Linux) 或 `netstat` 检查网络连接
- 启用详细日志查看消息传输过程
- 使用 Wireshark 分析网络包

## 扩展功能

可以在此基础上添加：
1. **服务注册中心** - 实现服务发现
2. **连接池** - 优化连接管理
3. **负载均衡** - 实现请求分发
4. **安全机制** - 添加认证和加密