#ifndef socket_h
#define socket_h

#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <queue>
#include <functional>
#include <thread>
#include <stdlib.h>
#include <chrono>
#include <thread>
#include <mutex>
#include "../segment/segment.hpp"
#include "../segment/segment_handler.hpp"
#include "../message/message.hpp"
#include "../message/messageFilter.hpp"
#include "../config/Config.hpp"
#include "Timeout.hpp"

#define MAXPACKETBUFFERSIZE 1500
#define DEFAULTCOLLECTINGTIME 10
#define MINCOLLECTINGTIME 1

using namespace std;

// for references
// https://maxnilz.com/docs/004-network/003-tcp-connection-state/
// You can use a different state enum if you'd like
enum TCPStatusEnum
{
    LISTEN = 0,
    SYN_SENT = 1,
    SYN_RECEIVED = 2,
    ESTABLISHED = 3,
    FIN_WAIT_1 = 4,
    FIN_WAIT_2 = 5,
    CLOSE_WAIT = 6,
    CLOSING = 7,
    LAST_ACK = 8,
    TIME_WAIT = 9,
    CLOSED = 10,
};

class Socket
{
    // todo add tcp connection state?
protected:
    /**
     * The ip address and port for the socket instance
     * Not to be confused with ip address and port of the connected connection
     */
    string ip;
    int16_t port;
    /**
     * Socket descriptor
     */
    int32_t socket;

    SegmentHandler *segmentHandler;

    TCPStatusEnum status;

    vector<Message> packetBuffer;
    float cleanerTime;

    int timeoutDefault;
    bool isListening;
    thread listener;
    thread cleaner;

    std::mutex bufferMutex;
    std::condition_variable bufferCV;

public:
    Socket(const string ip, const int16_t &port);
    ~Socket();
    // void send(string ip, int32_t port, void *dataStream, uint32_t dataSize);
    // int32_t recv(void *buffer, uint32_t length);
    void close();

    //auxiliary
    struct sockaddr_in generateAddress(string, uint16_t);
    void bindAddress();
    void setBroadcast();

    void setCleanerTime(float);
    void clearPacketBuffer();
    void cleanerPacketThread();

    void listenerPacketThread();
    void start();
    void stop();
    Message listen(MessageFilter*, int);

    void sendSegment(Segment, string, uint16_t);
};

#endif