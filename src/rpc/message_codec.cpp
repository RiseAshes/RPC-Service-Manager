#include "message_codec.h"
#include <iostream>
#include <cstring>
#include <arpa/inet.h>

namespace rpc {
namespace codec {

// 从字节流读取固定大小的整数（网络字节序）
uint32_t MessageCodec::readInt32(const std::string& data, size_t pos) {
    if (pos + 4 > data.size()) {
        return 0;
    }

    uint32_t value;
    memcpy(&value, data.data() + pos, 4);
    return ntohl(value);
}

// 将整数转换为网络字节序并写入字节流
void MessageCodec::writeInt32(std::string& data, uint32_t value) {
    uint32_t network_value = htonl(value);
    data.append(reinterpret_cast<const char*>(&network_value), 4);
}

// 编码消息（添加消息头）
std::string MessageCodec::encodeMessage(const protocol::RpcMessage& message) {
    std::string encoded_data;

    // 准备消息数据（先序列化 protobuf 消息）
    std::string message_data;
    message.SerializeToString(&message_data);

    // 创建消息头
    encoded_data.reserve(16 + message_data.size()); // 头16字节 + 消息体

    // 写入魔数
    writeInt32(encoded_data, protocol::PROTOCOL_MAGIC);

    // 写入版本
    writeInt32(encoded_data, protocol::PROTOCOL_VERSION);

    // 写入消息长度（不包括头）
    writeInt32(encoded_data, static_cast<uint32_t>(message_data.size()));

    // 写入消息类型
    writeInt32(encoded_data, static_cast<uint32_t>(message.type()));

    // 写入消息体
    encoded_data += message_data;

    return encoded_data;
}

// 解码消息（验证消息头并解析）
bool MessageCodec::decodeMessage(const std::string& data,
                                protocol::RpcMessage& message,
                                size_t& consumed) {
    // 至少需要头部数据
    if (data.size() < 16) {
        return false;
    }

    // 读取魔数
    uint32_t magic = readInt32(data, 0);
    if (magic != protocol::PROTOCOL_MAGIC) {
        std::cerr << "Invalid protocol magic: 0x" << std::hex << magic << std::endl;
        return false;
    }

    // 读取版本
    uint32_t version = readInt32(data, 4);
    if (version != protocol::PROTOCOL_VERSION) {
        std::cerr << "Unsupported protocol version: " << version << std::endl;
        return false;
    }

    // 读取消息长度
    uint32_t length = readInt32(data, 8);

    // 检查消息长度是否超过最大限制
    if (length > protocol::MAX_MESSAGE_SIZE) {
        std::cerr << "Message too large: " << length << " bytes" << std::endl;
        return false;
    }

    // 检查是否有完整的消息数据
    if (data.size() < 16 + length) {
        return false;
    }

    // 读取消息类型
    uint32_t type = readInt32(data, 12);

    // 解析消息体
    std::string message_data = data.substr(16, length);
    if (!message.ParseFromString(message_data)) {
        std::cerr << "Failed to parse message body" << std::endl;
        return false;
    }

    // 设置消息类型
    message.set_type(static_cast<protocol::RpcMessage::MessageType>(type));

    // 记录已消耗的字节数
    consumed = 16 + length;

    return true;
}

// 检查数据是否包含完整消息
bool MessageCodec::hasCompleteMessage(const std::string& buffer) {
    if (buffer.size() < 16) {
        return false;
    }

    // 检查魔数
    uint32_t magic = readInt32(buffer, 0);
    if (magic != protocol::PROTOCOL_MAGIC) {
        // 如果魔数错误，可能是错误数据，返回 true 让调用者处理
        return true;
    }

    // 读取消息长度
    uint32_t length = readInt32(buffer, 8);

    // 检查是否有完整的消息
    return buffer.size() >= 16 + length;
}

} // namespace codec
} // namespace rpc