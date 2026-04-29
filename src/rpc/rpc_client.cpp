#include "rpc_client.h"
#include <iostream>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <chrono>

namespace rpc {
namespace client {

RpcClient::RpcClient() : next_request_id_(1), running_(false) {
}

RpcClient::~RpcClient() {
    disconnect();
}

// 连接服务器
bool RpcClient::connect(const std::string& host, uint16_t port) {
    // 创建套接字
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd < 0) {
        std::cerr << "Failed to create socket: " << strerror(errno) << std::endl;
        return false;
    }

    // 设置套接字为非阻塞
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);

    // 连接服务器
    struct sockaddr_in server_addr;
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(port);
    server_addr.sin_addr.s_addr = inet_addr(host.c_str());

    int ret = connect(fd, (struct sockaddr*)&server_addr, sizeof(server_addr));
    if (ret < 0 && errno != EINPROGRESS) {
        std::cerr << "Failed to connect: " << strerror(errno) << std::endl;
        close(fd);
        return false;
    }

    // 创建连接对象
    connection_ = std::make_shared<net::TcpConnection>(fd);

    // 设置消息回调
    connection_->setMessageCallback([this](const std::string& data) {
        handleResponse(data);
    });

    // 设置关闭回调
    connection_->setCloseCallback([this]() {
        std::lock_guard<std::mutex> lock(mutex_);
        running_ = false;
        // 清理所有待处理请求
        for (auto& pair : pending_requests_) {
            pair.second->resolve("");
        }
        pending_requests_.clear();
        async_callbacks_.clear();
    });

    // 启动连接
    connection_->start();
    running_ = true;

    // 启动响应处理线程
    response_thread_ = std::thread([this]() {
        while (running_ && connection_ && connection_->isConnected()) {
            sleep(1); // 主循环，避免CPU占用过高
        }
        std::cout << "Client response thread stopped" << std::endl;
    });

    std::cout << "Connected to server at " << host << ":" << port << std::endl;
    return true;
}

// 同步调用
std::string RpcClient::callSync(const std::string& service_name,
                               const std::string& method_name,
                               const std::string& request_data,
                               int timeout_ms) {
    if (!connection_ || !connection_->isConnected()) {
        throw std::runtime_error("Client not connected");
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // 创建请求
    auto request = createRequest(service_name, method_name, request_data);
    std::string request_data_str;
    request.SerializeToString(&request_data_str);

    // 编码消息
    auto encoded = codec::MessageCodec::encodeMessage(request);

    // 创建承诺
    auto promise = std::make_shared<ResponsePromise>();
    uint64_t request_id = request.id();
    pending_requests_[request_id] = promise;

    // 发送请求
    if (!connection_->sendMessage(encoded)) {
        pending_requests_.erase(request_id);
        throw std::runtime_error("Failed to send request");
    }

    // 等待响应
    return promise->await(timeout_ms);
}

// 异步调用
void RpcClient::callAsync(const std::string& service_name,
                         const std::string& method_name,
                         const std::string& request_data,
                         ResponseCallback callback,
                         int timeout_ms) {
    if (!connection_ || !connection_->isConnected()) {
        if (callback) {
            callback("");
        }
        return;
    }

    std::lock_guard<std::mutex> lock(mutex_);

    // 创建请求
    auto request = createRequest(service_name, method_name, request_data);
    std::string request_data_str;
    request.SerializeToString(&request_data_str);

    // 编码消息
    auto encoded = codec::MessageCodec::encodeMessage(request);

    // 创建回调包装
    uint64_t request_id = request.id();
    auto promise = std::make_shared<ResponsePromise>();
    pending_requests_[request_id] = promise;
    async_callbacks_[request_id] = callback;

    // 设置超时处理
    std::thread timeout_thread([this, request_id, timeout_ms]() {
        std::this_thread::sleep_for(std::chrono::milliseconds(timeout_ms));
        {
            std::lock_guard<std::mutex> lock(mutex_);
            auto it = pending_requests_.find(request_id);
            if (it != pending_requests_.end()) {
                auto callback_it = async_callbacks_.find(request_id);
                if (callback_it != async_callbacks_.end() && callback_it->second) {
                    callback_it->second("Timeout");
                }
                pending_requests_.erase(it);
                async_callbacks_.erase(callback_it);
            }
        }
    });

    timeout_thread.detach();

    // 发送请求
    if (!connection_->sendMessage(encoded)) {
        std::lock_guard<std::mutex> lock(mutex_);
        pending_requests_.erase(request_id);
        async_callbacks_.erase(request_id);
        if (callback) {
            callback("Failed to send request");
        }
    }
}

// 断开连接
void RpcClient::disconnect() {
    running_ = false;

    if (response_thread_.joinable()) {
        response_thread_.join();
    }

    if (connection_) {
        connection_->stop();
        connection_.reset();
    }

    std::lock_guard<std::mutex> lock(mutex_);
    pending_requests_.clear();
    async_callbacks_.clear();

    std::cout << "Disconnected from server" << std::endl;
}

// 获取连接状态
bool RpcClient::isConnected() const {
    return connection_ && connection_->isConnected();
}

// 创建请求消息
protocol::RpcMessage RpcClient::createRequest(const std::string& service_name,
                                               const std::string& method_name,
                                               const std::string& request_data) {
    protocol::RpcMessage request;
    request.set_id(next_request_id_++);
    request.set_type(protocol::RpcMessage::REQUEST);
    request.set_service_name(service_name);
    request.set_method_name(method_name);
    request.set_request_data(request_data);
    return request;
}

// 处理响应消息
void RpcClient::handleResponse(const std::string& data) {
    protocol::RpcMessage response;
    size_t consumed = 0;

    if (!codec::MessageCodec::decodeMessage(data, response, consumed)) {
        std::cerr << "Failed to decode response message" << std::endl;
        return;
    }

    uint64_t request_id = response.id();

    std::lock_guard<std::mutex> lock(mutex_);

    // 查找对应的请求
    auto it = pending_requests_.find(request_id);
    if (it != pending_requests_.end()) {
        // 标记为已处理
        pending_requests_.erase(it);

        // 调用回调（如果有）
        auto callback_it = async_callbacks_.find(request_id);
        if (callback_it != async_callbacks_.end()) {
            if (callback_it->second) {
                callback_it->second(response.response_data());
            }
            async_callbacks_.erase(callback_it);
        } else {
            // 同步调用，解析结果
            it->second->resolve(response.response_data());
        }
    }
}

} // namespace client
} // namespace rpc