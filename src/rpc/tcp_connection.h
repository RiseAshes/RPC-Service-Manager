#pragma once
#include <string>
#include <memory>
#include <functional>
#include <mutex>
#include <atomic>
#include <condition_variable>

namespace rpc {
namespace net {

// TCP 连接类
class TcpConnection : public std::enable_shared_from_this<TcpConnection> {
public:
    // 回调函数类型
    using MessageCallback = std::function<void(const std::string&)>;
    using CloseCallback = std::function<void()>;

    TcpConnection(int fd);
    virtual ~TcpConnection();

    // 禁用拷贝构造和赋值
    TcpConnection(const TcpConnection&) = delete;
    TcpConnection& operator=(const TcpConnection&) = delete;

    // 发送消息
    bool sendMessage(const std::string& data);

    // 处理可读事件
    void handleRead();

    // 处理可写事件
    void handleWrite();

    // 关闭连接
    void handleClose();

    // 设置回调
    void setMessageCallback(const MessageCallback& cb);
    void setCloseCallback(const CloseCallback& cb);

    // 获取连接状态
    bool isConnected() const;
    std::string getPeerAddress() const;
    std::string getLocalAddress() const;

    // 启动/停止读写循环
    void start();
    void stop();

private:
    // 读取数据
    ssize_t readData(std::string& buffer);
    // 写入数据
    ssize_t writeData(const std::string& buffer);

    int fd_;
    std::string peer_address_;
    std::string local_address_;
    std::string read_buffer_;
    std::string write_buffer_;
    bool connected_;
    std::atomic<bool> running_;

    MessageCallback message_callback_;
    CloseCallback close_callback_;

    std::mutex mutex_;
    std::condition_variable write_cv_;
    std::thread read_thread_;
    std::thread write_thread_;
};

} // namespace net
} // namespace rpc