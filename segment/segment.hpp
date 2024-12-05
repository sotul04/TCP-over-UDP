#ifndef segment_h
#define segment_h

#include <cstdint>
#include <iostream>
#include <string>
#include <iomanip>
#include <cstring>
#include <utility>
#include <vector>
using namespace std;

struct Segment
{
    uint16_t sourcePort;
    uint16_t destPort;
    uint32_t seqNum;
    uint32_t ackNum;

    struct
    {
        unsigned int data_offset : 4;
        unsigned int reserved : 4;
    };

    struct
    {
        unsigned int cwr : 1;
        unsigned int ece : 1;
        unsigned int urg : 1;
        unsigned int ack : 1;
        unsigned int psh : 1;
        unsigned int rst : 1;
        unsigned int syn : 1;
        unsigned int fin : 1;
    } flags;

    uint16_t window;
    uint16_t checksum;
    uint16_t urgentPointer;

    uint8_t *payload;
    uint16_t payloadSize;
} __attribute__((packed));

const uint8_t FIN_FLAG = 1;
const uint8_t SYN_FLAG = 2;
const uint8_t ACK_FLAG = 16;
const uint8_t SYN_ACK_FLAG = SYN_FLAG | ACK_FLAG;
const uint8_t FIN_ACK_FLAG = FIN_FLAG | ACK_FLAG;
const uint8_t FIN_PSH_FLAG = 1 | 8;

/**
 * Generate Segment that contain SYN packet
 */
Segment syn(uint32_t seqNum);

/**
 * Generate Segment that contain ACK packet
 */
Segment ack(uint32_t ackNum);

/**
 * Generate Segment that contain SYN-ACK packet
 */
Segment synAck(uint32_t seqNum, uint32_t ackNum);

/**
 * Generate Segment that contain FIN packet
 */
Segment fin(uint32_t seqNum);

/**
 * Generate Segment that contain FIN-ACK packet
 */
Segment finAck(uint32_t seqNum, uint32_t ackNum);

// update return type as needed
uint16_t calculateChecksum(Segment segment);

/**
 * Return a new segment with a calcuated checksum fields
 */
Segment updateChecksum(Segment segment);

/**
 * Check if a TCP Segment has a valid checksum
 */
bool isValidChecksum(Segment segment);

uint16_t calculateCRC(Segment segment);

Segment updateCRC(Segment segment);

bool isValidCRC(Segment segment);

void printSegment(const Segment &segment);

bool operator==(const Segment &lhs, const Segment &rhs);

Segment copySegment(const Segment &source);

// segment pack & unpack
void serializeSegment(const Segment &segment, uint8_t *buffer);
Segment deserializeSegment(const uint8_t *buffer, uint32_t length);

Segment makeSegment(const string &data, uint16_t sport, uint16_t dport);

pair<string, string> extractMetada(const Segment &segment);

// string handler
string combineAsString(const vector<Segment> &segments);
vector<Segment> prepareStringSegments(const string &data, uint32_t seqNum);

#endif