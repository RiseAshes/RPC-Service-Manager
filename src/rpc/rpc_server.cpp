#include "rpc_server.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>

namespace rpc {
namespace server {

RpcServer::RpcServer() : listen_fd_(-1), port_(0), running_(false) {
}

RpcServer::~RpcServer() {
    stop();
}

// 启动服务器
bool RpcServer::start(uint16_t port) {
    if (running_) {
        std::cerr << "Server is already running" << std::endl;
        return false;
    }

    port_ = port;

    // 创建监听套接字
    listen_fd_ = createListenSocket(port);
    if (listen_fd_ < 0) {
        std::cerr << "Failed to create listen socket" << std::endl;
        return false;
    }

    // 设置监听队列大小
    if (listen(listen_fd_, 128) < 0) {
        std::cerr << "Failed to listen on socket: " << strerror(errno) << std::endl;
        close(listen_fd_);
        listen_fd_ = -1;
        return false;
    }

    running_ = true;

    // 启动接受连接线程
    accept_thread_ = std::thread([this]() {
        while (running_) {
            acceptConnection();
            usleep(1000); // 1ms
        }
        std::cout << "Accept thread stopped" << std::endl;
    });

    // 启动 I/O 处理线程
    io_thread_ = std::thread([this]() {
        while (running_) {
            // 这里可以添加定时任务，如心跳检测等
            sleep(1);
        }
        std::cout << "IO thread stopped" << std::endl;
    });

    std::cout << "Server started on port " << port_ << std::endl;
    return true;
}

// 注册服务
void RpcServer::registerService(const std::string& service_name,
                               const std::string& method_name,
                               ServiceHandler handler) {
    std::lock_guard<std::mutex> lock(services_mutex_);
    services_[service_name][method_name] = handler;
    std::cout << "Registered service: " << service_name << "." << method_name << std::endl;
}

// 注销服务
void RpcServer::unregisterService(const std::string& service_name,
                                 const std::string& method_name) {
    std::lock_guard<std::mutex> lock(services_mutex_);
    auto service_it = services_.find(service_name);
    if (service_it != services_.end()) {
        service_it->second.erase(method_name);
        if (service_it->second.empty()) {
            services_.erase(service_it);
            std::cout << "Unregistered service: " << service_name << std::endl;
        }
    }
}

// 停止服务器
void RpcServer::stop() {
    if (!running_) {
        return;
    }

    running_ = false;

    // 停止接受线程
    if (accept_thread_.joinable()) {
        accept_thread_.join();
    }

    // 停止 I/O 线程
    if (io_thread_.joinable()) {
        io_thread_.join();
    }

    // 关闭监听套接字
    if (listen_fd_ >= 0) {
        close(listen_fd_);
        listen_fd_ = -1;
    }

    // 关闭所有客户端连接
    std::lock_guard<std::mutex> lock(clients_mutex_);
    for (auto& conn : clients_) {
        conn->stop();
    }
    clients_.clear();

    // 清理服务注册表
    {
        std::lock_guard<std::mutex> lock(services_mutex_);
        services_.clear();
    }

    std::cout << "Server stopped" << std::endl;
}

// 获取已连接的客户端数量
size_t RpcServer::getClientCount() const {
    std::lock_guard<std::mutex> lock(clients_mutex_);
    return clients_.size();
}

// 获取服务列表
std::vector<std::string> RpcServer::getServiceList() const {
    std::vector<std::string> services;
    std::lock_guard<std::mutex> lock(services_mutex_);

    for (const auto& service_pair : services_) {
        services.push_back(service_pair.first);
    }

    return services;
}

// 创建监听套接字
int RpcServer::createListenSocket(uint16_t port) {
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return -1;
    }

    // 设置套接字选项
    int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        std::cerr << "Failed to set socket option: " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }

    // 设置非阻塞
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    // 绑定地址
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_addr.s_addr = INADDR_ANY;
    addr.sin_port = htons(port);

    if (bind(fd, (struct sockaddr*)&addr, sizeof(addr)) < 0) {
        std::cerr << "Failed to bind socket: " << strerror(errno) << std::endl;
        close(fd);
        return -1;
    }

    return fd;
}

// 接受连接
void RpcServer::acceptConnection() {
    struct sockaddr_in client_addr;
    socklen_t client_len = sizeof(client_addr);

    int client_fd = accept(listen_fd_, (struct sockaddr*)&client_addr, &client_len);
    if (client_fd < 0) {
        if (errno != EAGAIN && errno != EWOULDBLOCK) {
            std::cerr << "Failed to accept connection: " << strerror(errno) << std::endl;
        }
        return;
    }

    // 创建连接对象
    auto conn = std::make_shared<net::TcpConnection>(client_fd);

    // 设置消息回调
    conn->setMessageCallback([this, conn](const std::string& data) {
        protocol::RpcMessage message;
        size_t consumed = 0;

        if (codec::MessageCodec::decodeMessage(data, message, consumed)) {
            if (message.type() == protocol::RpcMessage::REQUEST) {
                handleRequest(conn, message);
            }
            // 可以添加对其他消息类型的处理
        }
    });

    // 设置关闭回调
    conn->setCloseCallback([this, conn]() {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_.erase(conn);
    });

    // 添加到客户端列表
    {
        std::lock_guard<std::mutex> lock(clients_mutex_);
        clients_.insert(conn);
    }

    // 启动连接
    conn->start();

    std::cout << "New client connected: " << conn->getPeerAddress()
              << " (Total clients: " << clients_.size() << ")" << std::endl;
}

// 处理连接
void RpcServer::handleConnection(std::shared_ptr<net::TcpConnection> conn) {
    // 这里可以添加连接相关的处理逻辑
    // 目前消息处理在连接的消息回调中完成
}

// 处理请求消息
void RpcServer::handleRequest(std::shared_ptr<net::TcpConnection> conn,
                             const protocol::RpcMessage& request) {
    std::lock_guard<std::mutex> lock(services_mutex_);

    // 查找服务
    auto service_it = services_.find(request.service_name());
    if (service_it == services_.end()) {
        // 服务不存在
        protocol::RpcMessage response;
        response.set_id(request.id());
        response.set_type(protocol::RpcMessage::RESPONSE);
        response.set_response_data("Service not found: " + request.service_name());

        std::string response_data;
        response.SerializeToString(&response_data);

        auto encoded = codec::MessageCodec::encodeMessage(response);
        conn->sendMessage(encoded);
        return;
    }

    // 查找方法
    auto method_it = service_it->second.find(request.method_name());
    if (method_it == service_it->second.end()) {
        // 方法不存在
        protocol::RpcMessage response;
        response.set_id(request.id());
        response.set_type(protocol::RpcMessage::RESPONSE);
        response.set_response_data("Method not found: " + request.method_name());

        std::string response_data;
        response.SerializeToString(&response_data);

        auto encoded = codec::MessageCodec::encodeMessage(response);
        conn->sendMessage(encoded);
        return;
    }

    // 调用处理函数
    try {
        std::string result = method_it->second(request.request_data());

        // 返回响应
        protocol::RpcMessage response;
        response.set_id(request.id());
        response.set_type(protocol::RpcMessage::RESPONSE);
        response.set_response_data(result);

        std::string response_data;
        response.SerializeToString(&response_data);

        auto encoded = codec::MessageCodec::encodeMessage(response);
        conn->sendMessage(encoded);

        std::cout << "Handled request: " << request.service_name() << "."
                  << request.method_name() << " from " << conn->getPeerAddress() << std::endl;
    } catch (const std::exception& e) {
        // 处理异常
        protocol::RpcMessage response;
        response.set_id(request.id());
        response.set_type(protocol::RpcMessage::RESPONSE);
        response.set_response_data("Error: " + std::string(e.what()));

        std::string response_data;
        response.SerializeToString(&response_data);

        auto encoded = codec::MessageCodec::encodeMessage(response);
        conn->sendMessage(encoded);

        std::cerr << "Error handling request: " << e.what() << std::endl;
    }
}

} // namespace server
} // namespace rpc