#include "message.hpp"

Message::Message(string ipAddress, uint16_t port)
{
    this->ipAddress = ipAddress;
    this->port = port;
}

Message::~Message() {}