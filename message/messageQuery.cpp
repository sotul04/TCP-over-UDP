#include "messageQuery.hpp"

using namespace std;

MessageQuery::MessageQuery(string ipAddress, uint16_t sourcePort, uint8_t flag, uint32_t seqNum, uint32_t ackNum, uint8_t *payload)
{
    this->ipAddress = ipAddress;
    this->sourcePort = sourcePort;
    this->flag = flag;
    this->seqNum = seqNum;
    this->ackNum = ackNum;
    this->payload = payload;
}

MessageQuery::~MessageQuery() {}

bool MessageQuery::validateMessageQuery(Message message)
{
    return this->ipAddress == message.ipAddress && this->sourcePort == message.port && this->flag == message.segment.flags.syn && this->seqNum == message.segment.seqNum && this->ackNum == message.segment.ackNum && this->payload == message.segment.payload;
}