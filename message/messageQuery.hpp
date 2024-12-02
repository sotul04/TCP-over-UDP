#ifndef MESSAGEQUERY_HPP
#define MESSAGEQUERY_HPP

#include <string>
#include <stdint.h>
#include <cstring>
#include "message.hpp"

using namespace std;

class MessageQuery
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
    MessageQuery(string *ip, uint16_t *port, uint8_t *flags, uint32_t *seqNum, uint32_t *ackNum, uint8_t *payload, size_t payloadSize);
    ~MessageQuery();

    // query generator
    static MessageQuery ipNPortQuery(string *ip, uint16_t *port);
    static MessageQuery ipNPortNSeqNumQuery(string *ip, uint16_t *port, uint32_t *seqNum);
    static MessageQuery ipNPortNFlagsQuery(string *ip, uint16_t *port, uint8_t *flags);
    static MessageQuery ipNPortNAckNumQuery(string *ip, uint16_t *port, uint32_t *ackNum);
    static MessageQuery flagsQuery(uint8_t *flags);
    static MessageQuery ipNPortNAckNumNFlagsQuery(string *ip, uint16_t *port, uint32_t *ackNum, uint8_t *flags);
    static MessageQuery payloadsQuery(uint8_t *payload, size_t payloadSize);

    bool validateMessageQuery(Message message);
};

#endif
