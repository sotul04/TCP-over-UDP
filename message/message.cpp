#include "message.hpp"

Message::Message(string ip, uint16_t port, Segment segment)
{
    this->ip = ip;
    this->port = port;
    this->segment = segment;
}

Message::~Message() {
    delete[] segment.payload;
}

bool Message::operator==(const Message &message) const
{
    return this->ip == message.ip && this->port == message.port && this->segment == message.segment;
}