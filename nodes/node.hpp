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
    vector<Segment> data;
    string ip;
    uint16_t port;

public:
    Node(string ip, uint16_t port);
    virtual void run() = 0;
    virtual ~Node();
    void setData(vector<Segment> segments);
    vector<Segment> getData() const;
};

#endif