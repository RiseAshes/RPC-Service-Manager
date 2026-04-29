#include <iostream>
#include <thread>
#include <chrono>
#include <signal.h>
#include "rpc_server.h"
#include "rpc_client.h"

using namespace rpc;
using namespace rpc::server;
using namespace rpc::client;

// 全局变量控制程序运行
bool running = true;

// 信号处理函数
void signal_handler(int signal) {
    std::cout << "\nReceived signal " << signal << ", shutting down..." << std::endl;
    running = false;
}

// Echo 服务处理函数
std::string echoService(const std::string& request) {
    std::cout << "Echo service received: " << request << std::endl;
    return "Echo: " + request;
}

// Add 服务处理函数
std::string addService(const std::string& request) {
    try {
        size_t pos = 0;
        int a = std::stoi(request, &pos);
        int b = std::stoi(request.substr(pos + 1));

        std::cout << "Add service: " << a << " + " << b << " = " << (a + b) << std::endl;
        return std::to_string(a + b);
    } catch (...) {
        return "Invalid add format: use 'a,b'";
    }
}

// 服务器主函数
void runServer() {
    RpcServer server;

    // 注册服务
    server.registerService("echo", "echo", echoService);
    server.registerService("math", "add", addService);

    // 启动服务器
    if (!server.start(8080)) {
        std::cerr << "Failed to start server" << std::endl;
        return;
    }

    std::cout << "Server running... (Press Ctrl+C to stop)" << std::endl;

    // 主循环
    while (running) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
        std::cout << "Active clients: " << server.getClientCount() << std::endl;
    }

    // 停止服务器
    server.stop();
}

// 客户端测试函数
void runClient() {
    RpcClient client;

    // 等待服务器启动
    std::this_thread::sleep_for(std::chrono::seconds(2));

    // 连接服务器
    if (!client.connect("127.0.0.1", 8080)) {
        std::cerr << "Failed to connect to server" << std::endl;
        return;
    }

    std::cout << "Connected to server" << std::endl;

    // 同步调用测试
    try {
        std::cout << "\n=== 同步调用测试 ===" << std::endl;

        // Echo 服务
        std::string result = client.callSync("echo", "echo", "Hello from sync client");
        std::cout << "Echo result: " << result << std::endl;

        // Add 服务
        result = client.callSync("math", "add", "10,20");
        std::cout << "Add result: " << result << std::endl;

        // 超时测试
        try {
            result = client.callSync("echo", "echo", "Test timeout", 100);
            std::cout << "Timeout result: " << result << std::endl;
        } catch (const std::exception& e) {
            std::cout << "Timeout test caught: " << e.what() << std::endl;
        }

    } catch (const std::exception& e) {
        std::cerr << "Sync call error: " << e.what() << std::endl;
    }

    // 异步调用测试
    try {
        std::cout << "\n=== 异步调用测试 ===" << std::endl;

        // Echo 服务
        client.callAsync("echo", "echo", "Hello from async client",
                         [](const std::string& response) {
                             std::cout << "Async echo result: " << response << std::endl;
                         });

        // Add 服务
        client.callAsync("math", "add", "30,40",
                         [](const std::string& response) {
                             std::cout << "Async add result: " << response << std::endl;
                         });

        // 等待异步回调
        std::this_thread::sleep_for(std::chrono::seconds(1));

    } catch (const std::exception& e) {
        std::cerr << "Async call error: " << e.what() << std::endl;
    }

    // 断开连接
    client.disconnect();
    std::cout << "Disconnected from server" << std::endl;
}

int main(int argc, char* argv[]) {
    // 注册信号处理
    signal(SIGINT, signal_handler);
    signal(SIGTERM, signal_handler);

    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " [server|client]" << std::endl;
        return 1;
    }

    std::string mode = argv[1];

    if (mode == "server") {
        runServer();
    } else if (mode == "client") {
        runClient();
    } else {
        std::cout << "Invalid mode. Use 'server' or 'client'" << std::endl;
        return 1;
    }

    return 0;
}