#ifndef SERVER_HPP
#define SERVER_HPP

#include "node.hpp"

class Server: public Node 
{
public:
    Server(uint16_t port): Node("0.0.0.0", port) {}
    ~Server() = default;
    void updateSeqNum(uint32_t seqNum);
    void run();
};

#endif