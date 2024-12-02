#include "messageFilter.hpp"

MessageFilter::MessageFilter(string *ip, uint16_t *port, uint8_t *flags, uint32_t *seqNum, uint32_t *ackNum, uint8_t *payload, size_t payloadSize)
{
    this->ip = ip;
    this->port = port;
    this->flags = flags;
    this->seqNum = seqNum;
    this->ackNum = ackNum;
    this->payload = payload;
    this->payloadSize = payloadSize;
}

MessageFilter::~MessageFilter()
{
    delete ip;
    delete port;
    delete flags;
    delete seqNum;
    delete ackNum;
    delete[] payload;
}

MessageFilter MessageFilter::ipNPortQuery(string *ip, uint16_t *port)
{
    return MessageFilter(ip, port, nullptr, nullptr, nullptr, nullptr, 0);
}

MessageFilter MessageFilter::ipNPortNSeqNumQuery(string *ip, uint16_t *port, uint32_t *seqNum)
{
    return MessageFilter(ip, port, nullptr, seqNum, nullptr, nullptr, 0);
}

MessageFilter MessageFilter::ipNPortNFlagsQuery(string *ip, uint16_t *port, uint8_t *flags)
{
    return MessageFilter(ip, port, flags, nullptr, nullptr, nullptr, 0);
}

MessageFilter MessageFilter::ipNPortNAckNumQuery(string *ip, uint16_t *port, uint32_t *ackNum)
{
    return MessageFilter(ip, port, nullptr, nullptr, ackNum, nullptr, 0);
}

MessageFilter MessageFilter::flagsQuery(uint8_t *flags)
{
    return MessageFilter(nullptr, nullptr, flags, nullptr, nullptr, nullptr, 0);
}

MessageFilter MessageFilter::ipNPortNAckNumNFlagsQuery(string *ip, uint16_t *port, uint32_t *ackNum, uint8_t *flags)
{
    return MessageFilter(ip, port, flags, nullptr, ackNum, nullptr, 0);
}

MessageFilter MessageFilter::payloadsQuery(uint8_t *payload, size_t payloadSize)
{
    return MessageFilter(nullptr, nullptr, nullptr, nullptr, nullptr, payload, payloadSize);
}

bool MessageFilter::validate(Message message)
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

        if (memcmp(payload, messagePayload, payloadSize) != 0)
            return false;
    }

    return true;
}
