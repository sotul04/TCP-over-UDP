#include "segment.hpp"
// #include <iostream>
// using namespace std;

Segment syn(uint32_t seqNum) {
    Segment s;
    s.seqNum = seqNum;
    s.ackNum = 0;
    s.data_offset = 20;
    s.reserved = 0;
    s.flags.cwr = 0;
    s.flags.ece = 0;
    s.flags.urg = 0;
    s.flags.ack = 0;
    s.flags.psh = 0;
    s.flags.rst = 0;
    s.flags.syn = 1;
    s.flags.fin = 0;
    s.window = 65535;
    return s;
}

Segment ack(uint32_t seqNum, uint32_t ackNum) {
    Segment s;
    s.seqNum = seqNum;
    s.ackNum = ackNum;
    s.data_offset = 20;
    s.reserved = 0;
    s.flags.cwr = 0;
    s.flags.ece = 0;
    s.flags.urg = 0;
    s.flags.ack = 1;
    s.flags.psh = 0;
    s.flags.rst = 0;
    s.flags.syn = 0;
    s.flags.fin = 0;
    s.window = 65535;
    return s;
}

Segment synAck(uint32_t seqNum) {
    Segment s;
    s.data_offset = 20;
    s.reserved = 0;
    s.flags.ack = 1;
    s.flags.syn = 1;
    s.window = 65535;
    return s;
}

Segment fin() {
    Segment s;
    s.data_offset = 20;
    s.reserved = 0;
    s.flags.fin = 1;
    s.window = 65535;
    return s;
}

Segment finAck() {
    Segment s;
    s.data_offset = 20;
    s.reserved = 0;
    s.flags.fin = 1;
    s.flags.ack = 1;
    s.window = 65535;
    return s;
}

// int main () {
//     syn(12345);
//     return 0;
// }