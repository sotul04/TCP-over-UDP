#ifndef CONNECTION_HPP
#define CONNECTION_HPP

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

using namespace std;

class Connection {
public:
    bool cont;
    string ip;
    uint16_t port;
    uint32_t seqNum;
    uint32_t ackNum;

    Connection(bool cont, string ip, uint16_t port, uint32_t seqNum, uint32_t ackNum):
        cont(cont), ip(ip), port(port), seqNum(seqNum), ackNum(ackNum) {}
};

#endif