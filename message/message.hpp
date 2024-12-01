#ifndef MESSAGE_HPP
#define MESSAGE_HPP

#include <string>
#include <cstdint>

using namespace std;

class Message {
    public:
        string ipAddress;
        uint16_t port;

        Message(string ipAddress, uint16_t port);
        ~Message();
};

#endif