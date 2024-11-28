#include "segment.hpp"

Segment syn(uint32_t seqNum)
{
    Segment segment = {};
    segment.seqNum = seqNum;
    segment.flags.syn = 1;
    segment.data_offset = 5; // Minimum TCP Header (5*4 = 20 bytes)
    segment.payload = nullptr;
    segment.payloadSize = 0;
    return segment;
}

Segment ack(uint32_t seqNum, uint32_t ackNum)
{
    Segment segment = {};
    segment.seqNum = seqNum;
    segment.ackNum = ackNum;
    segment.flags.ack = 1;
    segment.data_offset = 5;
    segment.payload = nullptr;
    segment.payloadSize = 0;
    return segment;
}

Segment synAck(uint32_t seqNum)
{
    Segment segment = {};
    segment.seqNum = seqNum;
    segment.flags.syn = 1;
    segment.flags.ack = 1;
    segment.data_offset = 5;
    segment.payload = nullptr;
    segment.payloadSize = 0;
    return segment;
}

Segment fin()
{
    Segment segment = {};
    segment.flags.fin = 1;
    segment.data_offset = 5;
    segment.payload = nullptr;
    segment.payloadSize = 0;
    return segment;
}

Segment finAck()
{
    Segment segment = {};
    segment.flags.fin = 1;
    segment.flags.ack = 1;
    segment.data_offset = 5;
    segment.payload = nullptr;
    segment.payloadSize = 0;
    return segment;
}

uint16_t calculateChecksum(Segment segment)
{
    uint32_t sum = 0;

    sum += segment.sourcePort;
    sum += segment.destPort;
    sum += (segment.seqNum >> 16) & 0xFFFF;
    sum += segment.seqNum & 0xFFFF;
    sum += (segment.ackNum >> 16) & 0xFFFF;
    sum += segment.ackNum & 0xFFFF;

    uint16_t flags_and_offset = (segment.data_offset << 12) |
                                (segment.flags.cwr << 7) |
                                (segment.flags.ece << 6) |
                                (segment.flags.urg << 5) |
                                (segment.flags.ack << 4) |
                                (segment.flags.psh << 3) |
                                (segment.flags.rst << 2) |
                                (segment.flags.syn << 1) |
                                segment.flags.fin;
    sum += flags_and_offset;

    sum += segment.window;
    sum += segment.urgentPointer;

    if (segment.payload != nullptr && segment.payloadSize > 0)
    {
        for (uint16_t i = 0; i < segment.payloadSize; i++)
        {
            sum += segment.payload[i];
        }
    }

    while (sum >> 16)
    {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return ~static_cast<uint16_t>(sum);
}

Segment updateChecksum(Segment segment)
{
    segment.checksum = calculateChecksum(segment);
    return segment;
}

bool isValidChecksum(Segment segment)
{
    uint16_t computedChecksum = calculateChecksum(segment);
    return computedChecksum == segment.checksum;
}

void printSegment(const Segment &segment)
{
    using namespace std;

    cout << "Segment Details:" << endl;
    cout << "Source Port: " << segment.sourcePort << endl;
    cout << "Destination Port: " << segment.destPort << endl;
    cout << "Sequence Number: " << segment.seqNum << endl;
    cout << "Acknowledgment Number: " << segment.ackNum << endl;
    cout << "Data Offset: " << static_cast<int>(segment.data_offset) << endl;
    cout << "Reserved: " << static_cast<int>(segment.reserved) << endl;

    cout << "Flags:" << endl;
    cout << "  CWR: " << static_cast<int>(segment.flags.cwr) << endl;
    cout << "  ECE: " << static_cast<int>(segment.flags.ece) << endl;
    cout << "  URG: " << static_cast<int>(segment.flags.urg) << endl;
    cout << "  ACK: " << static_cast<int>(segment.flags.ack) << endl;
    cout << "  PSH: " << static_cast<int>(segment.flags.psh) << endl;
    cout << "  RST: " << static_cast<int>(segment.flags.rst) << endl;
    cout << "  SYN: " << static_cast<int>(segment.flags.syn) << endl;
    cout << "  FIN: " << static_cast<int>(segment.flags.fin) << endl;

    cout << "Window Size: " << segment.window << endl;
    cout << "Checksum: 0x" << hex << setw(4) << setfill('0') << segment.checksum << dec << endl;
    cout << "Urgent Pointer: " << segment.urgentPointer << endl;

    if (segment.payloadSize > 0 && segment.payload != nullptr)
    {
        cout << "Payload (" << segment.payloadSize << " bytes): ";
        for (uint16_t i = 0; i < segment.payloadSize; ++i)
        {
            cout << segment.payload[i];
        }
        cout << endl;

        cout << "Payload (Hex): ";
        for (uint16_t i = 0; i < segment.payloadSize; ++i)
        {
            cout << hex << setw(2) << setfill('0') << static_cast<int>(segment.payload[i]) << " ";
        }
        cout << dec << endl;
    }
    else
    {
        cout << "Payload: (none)" << endl;
    }
}