#ifndef CLIENT_HPP
#define CLIENT_HPP

#include "node.hpp"
#include "../file/FileTransfer.hpp"

class Client: public Node
{
private:
    uint16_t serverPort;
public:
    Client(string ip, uint16_t port): Node(ip, port) {}
    ~Client() = default;
    void setServerPort(uint16_t port);
    void run();
};

#endif