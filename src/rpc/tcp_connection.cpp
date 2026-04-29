#include "tcp_connection.h"
#include <iostream>
#include <unistd.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <cstring>
#include <sstream>

namespace rpc {
namespace net {

TcpConnection::TcpConnection(int fd) : fd_(fd), connected_(true), running_(false) {
    // 设置套接字为非阻塞
    int flags = fcntl(fd_, F_GETFL, 0);
    fcntl(fd_, F_SETFL, flags | O_NONBLOCK);

    // 获取对端地址
    struct sockaddr_in peer_addr;
    socklen_t peer_len = sizeof(peer_addr);
    if (getpeername(fd_, (struct sockaddr*)&peer_addr, &peer_len) == 0) {
        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &peer_addr.sin_addr, addr_str, INET_ADDRSTRLEN);
        peer_address_ = std::string(addr_str) + ":" + std::to_string(ntohs(peer_addr.sin_port));
    }

    // 获取本地地址
    struct sockaddr_in local_addr;
    socklen_t local_len = sizeof(local_addr);
    if (getsockname(fd_, (struct sockaddr*)&local_addr, &local_len) == 0) {
        char addr_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &local_addr.sin_addr, addr_str, INET_ADDRSTRLEN);
        local_address_ = std::string(addr_str) + ":" + std::to_string(ntohs(local_addr.sin_port));
    }

    std::cout << "New connection: " << peer_address_ << " -> " << local_address_ << std::endl;
}

TcpConnection::~TcpConnection() {
    if (fd_ >= 0) {
        close(fd_);
        std::cout << "Connection closed: " << peer_address_ << std::endl;
    }
}

// 发送消息
bool TcpConnection::sendMessage(const std::string& data) {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_) {
        return false;
    }

    // 将数据添加到写缓冲区
    write_buffer_ += data;

    // 通知写线程有新数据
    write_cv_.notify_one();

    return true;
}

// 处理可读事件
void TcpConnection::handleRead() {
    // 这个方法在外部事件循环中调用
    // 触发内部读取线程
    if (running_ && connected_) {
        // 读取线程已经在运行，无需额外处理
    }
}

// 处理可写事件
void TcpConnection::handleWrite() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_ || write_buffer_.empty()) {
        return;
    }

    // 尝试发送写缓冲区中的数据
    while (!write_buffer_.empty()) {
        ssize_t bytes_written = writeData(write_buffer_);
        if (bytes_written < 0) {
            if (errno == EAGAIN || errno == EWOULDBLOCK) {
                // 暂时无法写入，等待下次事件
                break;
            } else {
                // 错误发生，关闭连接
                handleClose();
                return;
            }
        } else if (bytes_written == 0) {
            // 对端关闭连接
            handleClose();
            return;
        } else {
            // 移除已写入的数据
            write_buffer_.erase(0, bytes_written);
        }
    }
}

// 关闭连接
void TcpConnection::handleClose() {
    std::lock_guard<std::mutex> lock(mutex_);
    if (!connected_) {
        return;
    }

    connected_ = false;
    if (close_callback_) {
        close_callback_();
    }

    // 停止读写线程
    running_ = false;
    write_cv_.notify_all();

    if (read_thread_.joinable()) {
        read_thread_.join();
    }
    if (write_thread_.joinable()) {
        write_thread_.join();
    }

    close(fd_);
    fd_ = -1;
}

// 设置回调
void TcpConnection::setMessageCallback(const MessageCallback& cb) {
    message_callback_ = cb;
}

void TcpConnection::setCloseCallback(const CloseCallback& cb) {
    close_callback_ = cb;
}

// 获取连接状态
bool TcpConnection::isConnected() const {
    return connected_;
}

std::string TcpConnection::getPeerAddress() const {
    return peer_address_;
}

std::string TcpConnection::getLocalAddress() const {
    return local_address_;
}

// 启动读写循环
void TcpConnection::start() {
    running_ = true;

    // 启动读取线程
    read_thread_ = std::thread([this]() {
        while (running_ && connected_) {
            std::string buffer;
            ssize_t bytes_read = readData(buffer);

            if (bytes_read < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 暂时无法读取，稍后重试
                    usleep(1000); // 1ms
                    continue;
                } else {
                    // 错误发生，关闭连接
                    break;
                }
            } else if (bytes_read == 0) {
                // 对端关闭连接
                break;
            } else {
                // 收到数据，调用回调
                if (message_callback_) {
                    message_callback_(buffer);
                }
            }
        }

        // 连接断开，触发关闭回调
        if (connected_) {
            handleClose();
        }
    });

    // 启动写入线程
    write_thread_ = std::thread([this]() {
        while (running_ && connected_) {
            std::unique_lock<std::mutex> lock(mutex_);

            // 等待写缓冲区有数据
            write_cv_.wait(lock, [this]() {
                return !write_buffer_.empty() || !running_ || !connected_;
            });

            if (!connected_ || !running_) {
                break;
            }

            // 复制写缓冲区，避免长时间持有锁
            std::string to_write = write_buffer_;
            lock.unlock();

            // 如果缓冲区为空（可能刚被清空），继续等待
            if (to_write.empty()) {
                continue;
            }

            // 尝试写入
            ssize_t bytes_written = writeData(to_write);
            if (bytes_written < 0) {
                if (errno == EAGAIN || errno == EWOULDBLOCK) {
                    // 写入缓冲区，稍后重试
                    lock.lock();
                    write_buffer_.insert(0, to_write);
                } else {
                    // 错误发生，关闭连接
                    break;
                }
            } else if (bytes_written == 0) {
                // 对端关闭连接
                break;
            }
        }
    });
}

// 停止读写循环
void TcpConnection::stop() {
    handleClose();
}

// 读取数据
ssize_t TcpConnection::readData(std::string& buffer) {
    const size_t chunk_size = 4096;
    char chunk[chunk_size];

    ssize_t bytes_read = recv(fd_, chunk, chunk_size - 1, 0);
    if (bytes_read > 0) {
        chunk[bytes_read] = '\0';
        buffer += chunk;
    }

    return bytes_read;
}

// 写入数据
ssize_t TcpConnection::writeData(const std::string& buffer) {
    return send(fd_, buffer.c_str(), buffer.size(), MSG_NOSIGNAL);
}

} // namespace net
} // namespace rpc