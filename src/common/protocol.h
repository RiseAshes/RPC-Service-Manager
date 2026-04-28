#pragma once
#include "rpc_protocol.pb.h"

namespace rpc {
namespace protocol {

// 协议常量
constexpr uint32_t PROTOCOL_MAGIC = 0x52435054;  // "RCPT"
constexpr uint32_t PROTOCOL_VERSION = 1;
constexpr size_t MAX_MESSAGE_SIZE = 10 * 1024 * 1024;  // 10MB

// 消息头结构
struct MessageHeader {
    uint32_t magic;         // 魔数
    uint32_t version;       // 协议版本
    uint32_t length;        // 消息体长度
    uint32_t message_type;  // 消息类型
};

} // namespace protocol
} // namespace rpc