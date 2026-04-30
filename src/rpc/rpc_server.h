#pragma once
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <thread>
#include <mutex>
#include <set>
#include "tcp_connection.h"
#include "protocol.h"

namespace rpc {
namespace server {

// 服务处理函数类型
using ServiceHandler = std::function<std::string(const std::string&)>;

// RPC 服务器
class RpcServer {
public:
    RpcServer();
    ~RpcServer();

    // 禁用拷贝构造和赋值
    RpcServer(const RpcServer&) = delete;
    RpcServer& operator=(const RpcServer&) = delete;

    // 启动服务器
    bool start(uint16_t port);

    // 注册服务
    void registerService(const std::string& service_name,
                        const std::string& method_name,
                        ServiceHandler handler);

    // 注销服务
    void unregisterService(const std::string& service_name,
                          const std::string& method_name);

    // 停止服务器
    void stop();

    // 获取已连接的客户端数量
    size_t getClientCount() const;

    // 获取服务列表
    std::vector<std::string> getServiceList() const;

private:
    // 创建监听套接字
    int createListenSocket(uint16_t port);

    // 接受连接
    void acceptConnection();

    // 处理连接
    void handleConnection(std::shared_ptr<net::TcpConnection> conn);

    // 处理请求消息
    void handleRequest(std::shared_ptr<net::TcpConnection> conn,
                      const rpc::RpcMessage& request);

    int listen_fd_;
    uint16_t port_;
    bool running_;
    std::thread accept_thread_;
    std::thread io_thread_;

    // 服务注册表: service_name -> method_name -> handler
    std::map<std::string, std::map<std::string, ServiceHandler>> services_;
    std::mutex services_mutex_;

    // 已连接的客户端
    std::set<std::shared_ptr<net::TcpConnection>> clients_;
    std::mutex clients_mutex_;
};

} // namespace server
} // namespace rpc