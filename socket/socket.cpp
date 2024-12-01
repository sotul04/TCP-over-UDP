#include "socket.hpp"

TCPSocket::TCPSocket(const string ip, const int32_t &port)
{
    this->ip = ip;
    this->port = port;
    if ((socket = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    segmentHandler = new SegmentHandler();
    status = CLOSED;
}