#include "messageFilter.hpp"
#include <cstring>

void MessageFilter::copyPayload(const uint8_t *source, size_t size)
{
    if (source && size > 0)
    {
        payload = std::make_unique<uint8_t[]>(size);
        std::memcpy(payload.get(), source, size);
        payloadSize = size;
    }
    else
    {
        payload.reset();
        payloadSize = 0;
    }
}

MessageFilter::MessageFilter(
    const std::optional<std::string> &ip,
    const std::optional<uint16_t> &port,
    const std::optional<uint8_t> &flags,
    const std::optional<uint32_t> &seqNum,
    const std::optional<uint32_t> &ackNum,
    const uint8_t *payload,
    size_t payloadSize)
    : ip(ip), port(port), flags(flags), seqNum(seqNum), ackNum(ackNum), payload(nullptr), payloadSize(0)
{
    copyPayload(payload, payloadSize);
}

MessageFilter::MessageFilter(const MessageFilter &other)
    : ip(other.ip), port(other.port), flags(other.flags), seqNum(other.seqNum), ackNum(other.ackNum)
{
    copyPayload(other.payload.get(), other.payloadSize);
}

MessageFilter::MessageFilter(MessageFilter &&other) noexcept
    : ip(std::move(other.ip)), port(std::move(other.port)), flags(std::move(other.flags)),
      seqNum(std::move(other.seqNum)), ackNum(std::move(other.ackNum)),
      payload(std::move(other.payload)), payloadSize(other.payloadSize)
{
    other.payloadSize = 0;
}

MessageFilter &MessageFilter::operator=(const MessageFilter &other)
{
    if (this != &other)
    {
        ip = other.ip;
        port = other.port;
        flags = other.flags;
        seqNum = other.seqNum;
        ackNum = other.ackNum;
        copyPayload(other.payload.get(), other.payloadSize);
    }
    return *this;
}

MessageFilter &MessageFilter::operator=(MessageFilter &&other) noexcept
{
    if (this != &other)
    {
        ip = std::move(other.ip);
        port = std::move(other.port);
        flags = std::move(other.flags);
        seqNum = std::move(other.seqNum);
        ackNum = std::move(other.ackNum);
        payload = std::move(other.payload);
        payloadSize = other.payloadSize;
        other.payloadSize = 0;
    }
    return *this;
}

MessageFilter::~MessageFilter() = default;

MessageFilter MessageFilter::withIP(const std::string &ip) const
{
    return MessageFilter(ip, port, flags, seqNum, ackNum, payload.get(), payloadSize);
}

MessageFilter MessageFilter::withPort(uint16_t port) const
{
    return MessageFilter(ip, port, flags, seqNum, ackNum, payload.get(), payloadSize);
}

MessageFilter MessageFilter::withSeqNum(uint32_t seqNum) const
{
    return MessageFilter(ip, port, flags, seqNum, ackNum, payload.get(), payloadSize);
}

MessageFilter MessageFilter::withFlags(uint8_t flags) const
{
    return MessageFilter(ip, port, flags, seqNum, ackNum, payload.get(), payloadSize);
}

MessageFilter MessageFilter::withAckNum(uint32_t ackNum) const
{
    return MessageFilter(ip, port, flags, seqNum, ackNum, payload.get(), payloadSize);
}

MessageFilter MessageFilter::withPayloads(const uint8_t *payload, size_t payloadSize) const
{
    return MessageFilter(ip, port, flags, seqNum, ackNum, payload, payloadSize);
}

// MessageFilter MessageFilter::ipNPortQuery(const std::string &ip, uint16_t port)
// {
//     return MessageFilter(ip, port, std::nullopt, std::nullopt, std::nullopt, nullptr, 0);
// }

// MessageFilter MessageFilter::ipNPortNSeqNumQuery(const std::string &ip, uint16_t port, uint32_t seqNum)
// {
//     return MessageFilter(ip, port, std::nullopt, seqNum, std::nullopt, nullptr, 0);
// }

// MessageFilter MessageFilter::ipNPortNFlagsQuery(const std::string &ip, uint16_t port, uint8_t flags)
// {
//     return MessageFilter(ip, port, flags, std::nullopt, std::nullopt, nullptr, 0);
// }

// MessageFilter MessageFilter::ipNPortNAckNumQuery(const std::string &ip, uint16_t port, uint32_t ackNum)
// {
//     return MessageFilter(ip, port, std::nullopt, std::nullopt, ackNum, nullptr, 0);
// }

// MessageFilter MessageFilter::flagsQuery(uint8_t flags)
// {
//     return MessageFilter(std::nullopt, std::nullopt, flags, std::nullopt, std::nullopt, nullptr, 0);
// }

// MessageFilter MessageFilter::ipNPortNAckNumNFlagsQuery(const std::string &ip, uint16_t port, uint32_t ackNum, uint8_t flags)
// {
//     return MessageFilter(ip, port, flags, std::nullopt, ackNum, nullptr, 0);
// }

// MessageFilter MessageFilter::payloadsQuery(const uint8_t* payload, size_t payloadSize)
// {
//     uint8_t *copiedPayload = new uint8_t[payloadSize];
//     std::memcpy(copiedPayload, payload, payloadSize);
//     return MessageFilter(std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, copiedPayload, payloadSize);
// }

bool MessageFilter::validate(const Message &message) const
{
    if (ip && *ip != message.ip)
        return false;

    if (port && *port != message.port)
        return false;

    if (flags)
    {
        uint8_t messageFlags =
            (message.segment.flags.cwr << 7) |
            (message.segment.flags.ece << 6) |
            (message.segment.flags.urg << 5) |
            (message.segment.flags.ack << 4) |
            (message.segment.flags.psh << 3) |
            (message.segment.flags.rst << 2) |
            (message.segment.flags.syn << 1) |
            (message.segment.flags.fin);

        if (*flags != messageFlags)
            return false;
    }

    if (seqNum && *seqNum != message.segment.seqNum)
        return false;

    if (ackNum && *ackNum != message.segment.ackNum)
        return false;

    if (payload)
    {
        const uint8_t *messagePayload = message.segment.payload;
        size_t messagePayloadSize = message.segment.payloadSize;

        if (payloadSize != messagePayloadSize)
        {
            return false;
        }

        if (std::memcmp(payload.get(), messagePayload, payloadSize) != 0)
            return false;
    }

    return true;
}
