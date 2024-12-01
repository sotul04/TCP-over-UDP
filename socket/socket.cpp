#include "socket.hpp"
#include "../config/Config.hpp"
#include "../segment/serialize.hpp"
#include "../segment/segment.hpp"

TCPSocket::TCPSocket(const string ip, const int32_t &port)
{
    this->ip = ip;
    this->port = port;

    this->packetCollectingTime = 0.1;
    this->packetBuffer = vector<Message>();

    if ((socket = ::socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }
    segmentHandler = new SegmentHandler();
    status = CLOSED;
}

TCPSocket::~TCPSocket()
{
    delete segmentHandler;
}

void TCPSocket::setPacketCollectingTime(int time)
{
    this->packetCollectingTime = time;
}

void TCPSocket::cleanPacketBuffer()
{
    packetBuffer.clear();
}

void TCPSocket::managePacketGarbage()
{
    while (listenStatus) {
        try {
        }

        catch (exception &e) {}
    }
}

void TCPSocket::listenThreadWorker() {
    uint8_t *buffer = new uint8_t[MAXLINE];
    struct sockaddr_in cliaddr;
    socklen_t len = sizeof(cliaddr);
    
    while (listenStatus) {
        try {
            int n = recvfrom(socket, buffer, 
                            MAXLINE, MSG_WAITALL, 
                            (struct sockaddr*) &cliaddr, 
                            &len);
            Segment received = deserializeSegment(buffer, n);
            if (!isValidChecksum(received)) {
                throw runtime_error("Segment not valid");
            }
            try {
                Message newMessage(inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port, received);
                packetBuffer.push_back(newMessage);
            } catch (exception e) {
                cout << "Bad packet" << "\n";
            }
        } catch (exception e) {
            // do nothing
        }
    }
}

void TCPSocket::listenThreadStart() {
    listenStatus = true;
    listener = thread(listenThreadWorker);
    collectGarbage = thread(managePacketGarbage);
}

void TCPSocket::listenThreadStop() {
    listenStatus = false;
    listener.join();
}

void TCPSocket::listen() {
    
}