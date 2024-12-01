#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <cstdint>
#include "segment/segment.hpp"

using namespace std;

class Message {
    public:
        string ipAddress;
        uint16_t port;
        Segment segment;

        Message(string ipAddress, uint16_t port, Segment segment);
        ~Message();

        bool operator==(const Message &message) const;
};

#endif