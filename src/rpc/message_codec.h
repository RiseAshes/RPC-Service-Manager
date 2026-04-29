#pragma once
#include <string>
#include <cstdint>
#include "protocol.h"

namespace rpc {
namespace codec {

// 消息编解码器
class MessageCodec {
public:
    // 编码消息（添加消息头）
    static std::string encodeMessage(const protocol::RpcMessage& message);

    // 解码消息（验证消息头并解析）
    static bool decodeMessage(const std::string& data,
                            protocol::RpcMessage& message,
                            size_t& consumed);

    // 检查数据是否包含完整消息
    static bool hasCompleteMessage(const std::string& buffer);

private:
    // 从字节流读取固定大小的整数（网络字节序）
    static uint32_t readInt32(const std::string& data, size_t pos);

    // 将整数转换为网络字节序并写入字节流
    static void writeInt32(std::string& data, uint32_t value);
};

} // namespace codec
} // namespace rpc