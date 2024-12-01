#include "socket.hpp"

TCPSocket::TCPSocket(const string ip, const int32_t &port)
{
    this->ip = ip;
    this->port = port;

    this->packetCollectingTime = DEFAULTCOLLECTINGTIME;
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
        try 
        {
            Message firstMessage = packetBuffer.front();

            std::this_thread::sleep_for(std::chrono::seconds(this->packetCollectingTime));

            if (packetBuffer.front() == firstMessage) {
                packetBuffer.erase(packetBuffer.begin());
            }

            if (packetBuffer.size() > MAXPACKETBUFFERSIZE) {
                setPacketCollectingTime(MINCOLLECTINGTIME);
            } else {
                setPacketCollectingTime(DEFAULTCOLLECTINGTIME);
            }
        } 
        catch (exception &e) {}
    }
}