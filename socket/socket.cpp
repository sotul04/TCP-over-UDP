#include "socket.hpp"
#include "../config/Config.hpp"
#include "../segment/serialize.hpp"
#include "../segment/segment.hpp"
#include "../message/messageQuery.hpp"

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

Message TCPSocket::listen(MessageQuery* message, int timeout) {
    Message *retval = nullptr;
    chrono::steady_clock::time_point limit;
    if (timeout != NULL) {
        auto start = chrono::steady_clock::now();
        limit = start + chrono::seconds(timeout);
    }
    if (message == nullptr) {
        while (retval == nullptr) {
            try {
                if (packetBuffer.empty()) {
                    *retval = packetBuffer.front();
                    packetBuffer.erase(packetBuffer.begin());
                }
            } catch (exception e) {

            }

            if ((timeout != NULL) && chrono::steady_clock::now() > limit) {
                throw runtime_error("Error");
            }
        }
    } else {
        while (retval == nullptr) {
            try {
                for (int i = 0; i < packetBuffer.size(); i++) {
                    if (message->validateMessageQuery(packetBuffer[i])) {
                        *retval = packetBuffer[i];
                        packetBuffer.erase(packetBuffer.begin() + i);
                        break;
                    }
                }
            } catch (exception e) {
                
            }

            if ((timeout != NULL) && chrono::steady_clock::now() > limit) {
                throw std::runtime_error("Error");
            }
        }
    }
    return *retval;
}