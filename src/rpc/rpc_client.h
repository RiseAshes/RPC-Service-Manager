#pragma once
#include <string>
#include <memory>
#include <functional>
#include <map>
#include <mutex>
#include <thread>
#include <condition_variable>
#include "tcp_connection.h"
#include "protocol.h"

namespace rpc {
namespace client {

// RPC 响应承诺类
class ResponsePromise {
public:
    std::string result;
    bool resolved;
    std::condition_variable cv;
    std::mutex mutex;

    void resolve(const std::string& data) {
        std::lock_guard<std::mutex> lock(mutex);
        result = data;
        resolved = true;
        cv.notify_one();
    }

    std::string await(int timeout_ms = 5000) {
        std::unique_lock<std::mutex> lock(mutex);
        if (!cv.wait_for(lock, std::chrono::milliseconds(timeout_ms),
                        [this]() { return resolved; })) {
            throw std::runtime_error("Timeout waiting for response");
        }
        return result;
    }
};

// RPC 客户端
class RpcClient {
public:
    RpcClient();
    ~RpcClient();

    // 禁用拷贝构造和赋值
    RpcClient(const RpcClient&) = delete;
    RpcClient& operator=(const RpcClient&) = delete;

    // 连接服务器
    bool connect(const std::string& host, uint16_t port);

    // 同步调用
    std::string callSync(const std::string& service_name,
                         const std::string& method_name,
                         const std::string& request_data,
                         int timeout_ms = 5000);

    // 异步调用
    using ResponseCallback = std::function<void(const std::string&)>;
    void callAsync(const std::string& service_name,
                  const std::string& method_name,
                  const std::string& request_data,
                  ResponseCallback callback,
                  int timeout_ms = 5000);

    // 断开连接
    void disconnect();

    // 获取连接状态
    bool isConnected() const;

private:
    // 创建请求消息
    rpc::RpcMessage createRequest(const std::string& service_name,
                                  const std::string& method_name,
                                  const std::string& request_data);

    // 处理响应消息
    void handleResponse(const std::string& data);

    std::shared_ptr<net::TcpConnection> connection_;
    uint64_t next_request_id_;
    std::map<uint64_t, std::shared_ptr<ResponsePromise>> pending_requests_;
    std::map<uint64_t, ResponseCallback> async_callbacks_;
    std::mutex mutex_;
    std::thread response_thread_;
    bool running_;
};

} // namespace client
} // namespace rpc