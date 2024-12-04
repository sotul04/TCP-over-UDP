#ifndef TCPSOCKET_HPP
#define TCPSOCKET_HPP

#include "socket.hpp"
#include "connection.hpp"

struct receivedData {
    Connection conn;
    vector<Segment> data;

    // Constructor to initialize the Connection and the data vector
    receivedData(const Connection& c, const vector<Segment>& d) : conn(c), data(d) {}
};

class TCPSocket: public Socket {
private:
public:
    TCPSocket(const string ip, const uint16_t &port): Socket(ip, port) {}

    Connection seekBroadcast(string destIP, uint16_t destPort);
    Connection listenBroadcast();

    Connection reqHandShake(string destIP, uint16_t destPort);
    Connection accHandShake(string destIP, uint16_t destPort);

    Connection reqClosing(string destIP, uint16_t destPort, uint32_t finSeqNum);
    Connection accClosing(string destIp, uint16_t destPort, uint32_t finSeqNum);

    void senderThread(const Message& message, std::atomic<int>& lastAck, std::atomic<bool> &abort);
    //sliding algorithm
    Connection sendData(string destIP, uint16_t desPort, uint32_t seqNum, vector<Segment> data);
    pair<vector<Segment>, Connection> receiveData(string destIP, uint16_t destPort, uint32_t seqNum);
};

#endif
