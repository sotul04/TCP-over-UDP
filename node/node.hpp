#ifndef node_h
#define node_h

#include "../socket/TCPSocket.hpp"

/**
 * Abstract class.
 *
 * This is the base class for Server and Client class.
 */
class Node
{
protected:
    TCPSocket *connection;

public:
    Node(string ip, uint16_t port);
    void run();
    virtual ~Node();
    virtual void handleMessage(void *buffer) = 0;
};

#endif