#include "messageFilter.hpp"
#include <cstring>

MessageFilter::MessageFilter(
    const std::optional<std::string> &ip,
    const std::optional<uint16_t> &port,
    const std::optional<uint8_t> &flags,
    const std::optional<uint32_t> &seqNum,
    const std::optional<uint32_t> &ackNum,
    uint8_t *payload,
    size_t payloadSize) : ip(ip), port(port), flags(flags), seqNum(seqNum), ackNum(ackNum), payload(payload), payloadSize(payloadSize) {}

MessageFilter::~MessageFilter()
{
    delete[] payload;
}

MessageFilter MessageFilter::ipNPortQuery(const std::string &ip, uint16_t port)
{
    return MessageFilter(ip, port, std::nullopt, std::nullopt, std::nullopt, nullptr, 0);
}

MessageFilter MessageFilter::ipNPortNSeqNumQuery(const std::string &ip, uint16_t port, uint32_t seqNum)
{
    return MessageFilter(ip, port, std::nullopt, seqNum, std::nullopt, nullptr, 0);
}

MessageFilter MessageFilter::ipNPortNFlagsQuery(const std::string &ip, uint16_t port, uint8_t flags)
{
    return MessageFilter(ip, port, flags, std::nullopt, std::nullopt, nullptr, 0);
}

MessageFilter MessageFilter::ipNPortNAckNumQuery(const std::string &ip, uint16_t port, uint32_t ackNum)
{
    return MessageFilter(ip, port, std::nullopt, std::nullopt, ackNum, nullptr, 0);
}

MessageFilter MessageFilter::flagsQuery(uint8_t flags)
{
    return MessageFilter(std::nullopt, std::nullopt, flags, std::nullopt, std::nullopt, nullptr, 0);
}

MessageFilter MessageFilter::ipNPortNAckNumNFlagsQuery(const std::string &ip, uint16_t port, uint32_t ackNum, uint8_t flags)
{
    return MessageFilter(ip, port, flags, std::nullopt, ackNum, nullptr, 0);
}

MessageFilter MessageFilter::payloadsQuery(uint8_t *payload, size_t payloadSize)
{
    uint8_t *copiedPayload = new uint8_t[payloadSize];
    std::memcpy(copiedPayload, payload, payloadSize);
    return MessageFilter(std::nullopt, std::nullopt, std::nullopt, std::nullopt, std::nullopt, copiedPayload, payloadSize);
}

bool MessageFilter::validate(const Message &message)
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
            return false;

        if (std::memcmp(payload, messagePayload, payloadSize) != 0)
            return false;
    }

    return true;
}
