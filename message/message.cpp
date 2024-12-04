#include "message.hpp"

Message::Message(string ip, uint16_t port, Segment segment)
{
    this->ip = ip;
    this->port = port;
    this->segment = copySegment(segment);

}

Message::Message(const Message& other) 
{
    this->ip= other.ip;
    this->port = other.port;
    this->segment = copySegment(other.segment);
}

Message &Message::operator=(Message&& other) noexcept
{
    if (this != &other)
    {
        ip = std::move(other.ip);
        port = std::move(other.port);
        segment = copySegment(other.segment);
    }

    return *this;
}

Message::~Message() {
    delete[] segment.payload;
}

bool Message::operator==(const Message &message) const
{
    return this->ip == message.ip && this->port == message.port && this->segment == message.segment;
}