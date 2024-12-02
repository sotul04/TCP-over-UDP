#ifndef MessageFilter_HPP
#define MessageFilter_HPP

#include <string>
#include <stdint.h>
#include <cstring>
#include "message.hpp"

using namespace std;

class MessageFilter
{
private:
    string *ip;
    uint16_t *port;
    uint8_t *flags;
    uint32_t *seqNum;
    uint32_t *ackNum;
    uint8_t *payload;
    size_t payloadSize;

public:
    MessageFilter(string *ip, uint16_t *port, uint8_t *flags, uint32_t *seqNum, uint32_t *ackNum, uint8_t *payload, size_t payloadSize);
    ~MessageFilter();

    // query generator
    static MessageFilter ipNPortQuery(string *ip, uint16_t *port);
    static MessageFilter ipNPortNSeqNumQuery(string *ip, uint16_t *port, uint32_t *seqNum);
    static MessageFilter ipNPortNFlagsQuery(string *ip, uint16_t *port, uint8_t *flags);
    static MessageFilter ipNPortNAckNumQuery(string *ip, uint16_t *port, uint32_t *ackNum);
    static MessageFilter flagsQuery(uint8_t *flags);
    static MessageFilter ipNPortNAckNumNFlagsQuery(string *ip, uint16_t *port, uint32_t *ackNum, uint8_t *flags);
    static MessageFilter payloadsQuery(uint8_t *payload, size_t payloadSize);

    bool validate(Message message);
};

#endif
