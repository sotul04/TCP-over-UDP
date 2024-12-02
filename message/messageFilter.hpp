#ifndef MESSAGEFILTER_HPP
#define MESSAGEFILTER_HPP

#include <string>
#include <optional>
#include "message.hpp"

class MessageFilter {
private:
    std::optional<std::string> ip;
    std::optional<uint16_t> port;
    std::optional<uint8_t> flags;
    std::optional<uint32_t> seqNum;
    std::optional<uint32_t> ackNum;
    uint8_t* payload;
    size_t payloadSize;

public:
    MessageFilter(
        const std::optional<std::string>& ip,
        const std::optional<uint16_t>& port,
        const std::optional<uint8_t>& flags,
        const std::optional<uint32_t>& seqNum,
        const std::optional<uint32_t>& ackNum,
        uint8_t* payload,
        size_t payloadSize
    );
    ~MessageFilter();

    // Query generator
    static MessageFilter ipNPortQuery(const std::string& ip, uint16_t port);
    static MessageFilter ipNPortNSeqNumQuery(const std::string& ip, uint16_t port, uint32_t seqNum);
    static MessageFilter ipNPortNFlagsQuery(const std::string& ip, uint16_t port, uint8_t flags);
    static MessageFilter ipNPortNAckNumQuery(const std::string& ip, uint16_t port, uint32_t ackNum);
    static MessageFilter flagsQuery(uint8_t flags);
    static MessageFilter ipNPortNAckNumNFlagsQuery(const std::string& ip, uint16_t port, uint32_t ackNum, uint8_t flags);
    static MessageFilter payloadsQuery(uint8_t* payload, size_t payloadSize);

    bool validate(const Message& message);
};

#endif
