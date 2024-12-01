#ifndef MESSAGEQUERY_HPP
#define MESSAGEQUERY_HPP

#include <string>
#include <stdlib.h>
#include "message.hpp"

using namespace std;

class MessageQuery
{
private:
    string ipAddress;
    uint16_t sourcePort;
    uint8_t flag;
    uint32_t seqNum;
    uint32_t ackNum;
    uint8_t *payload; 

public:
    MessageQuery(string ipAddress, uint16_t sourcePort, uint8_t flag, uint32_t seqNum, uint32_t ackNum, uint8_t *payload);
    ~MessageQuery();

    bool validateMessageQuery(Message message);
};

#endif