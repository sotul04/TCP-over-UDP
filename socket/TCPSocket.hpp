#ifndef TCPSOCKET_HPP
#define TCPSOCKET_HPP

#include "socket.hpp"
#include "connection.hpp"

class TCPSocket: public Socket {
private:
public:
    TCPSocket(const string ip, const uint16_t &port): Socket(ip, port) {}

    Connection seekBroadcast(string destIP, uint16_t destPort);
    Connection listenBroadcast();

    Connection reqHandShake(string destIP, uint16_t destPort);
    Connection accHandShake(string destIP, uint16_t destPort);
    
};

#endif
