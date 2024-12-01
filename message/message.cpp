#include "message.hpp"

Message::Message(string ipAddress, uint16_t port, Segment segment)
{
    this->ipAddress = ipAddress;
    this->port = port;
    this->segment = segment;
}

Message::~Message() {}