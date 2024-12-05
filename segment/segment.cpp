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

Segment ack(uint32_t ackNum)
{
    Segment segment = {};
    segment.ackNum = ackNum;
    segment.flags.ack = 1;
    segment.data_offset = 5;
    segment.payload = nullptr;
    segment.payloadSize = 0;
    return segment;
}

Segment synAck(uint32_t seqNum, uint32_t ackNum)
{
    Segment segment = {};
    segment.seqNum = seqNum;
    segment.ackNum = ackNum;
    segment.flags.syn = 1;
    segment.flags.ack = 1;
    segment.data_offset = 5;
    segment.payload = nullptr;
    segment.payloadSize = 0;
    return segment;
}

Segment fin(uint32_t seqNum)
{
    Segment segment = {};
    segment.seqNum = seqNum;
    segment.flags.fin = 1;
    segment.data_offset = 5;
    segment.payload = nullptr;
    segment.payloadSize = 0;
    return segment;
}

Segment finAck(uint32_t seqNum, uint32_t ackNum)
{
    Segment segment = {};
    segment.seqNum = seqNum;
    segment.ackNum = ackNum;
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

uint16_t calculateCRC(Segment segment)
{
    uint16_t crcPoly = 0x8005;
    uint16_t crc = 0xFFFF;
    size_t length = sizeof(segment.payload) / sizeof(segment.payload[0]);

    for (size_t i = 0; i < length; i++) {
        crc ^= (uint16_t)(segment.payload[i] << 8);
        for (uint8_t bit = 0; bit < 8; bit++) {
            if (crc & 0x8000) {
                crc = (crc << 1) ^ crcPoly;
            } else {
                crc <<= 1;
            }
        }
    }
    
    return crc;

    return 0;
}

Segment updateCRC(Segment segment)
{
    segment.urgentPointer = calculateCRC(segment);
    return segment;
}

bool isValidCRC(Segment segment)
{
    uint16_t computedCRC = calculateCRC(segment);
    return computedCRC == segment.urgentPointer;
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

bool operator==(const Segment &lhs, const Segment &rhs)
{
    return lhs.sourcePort == rhs.sourcePort &&
           lhs.destPort == rhs.destPort &&
           lhs.seqNum == rhs.seqNum &&
           lhs.ackNum == rhs.ackNum &&
           lhs.data_offset == rhs.data_offset &&
           lhs.reserved == rhs.reserved &&
           lhs.flags.cwr == rhs.flags.cwr &&
           lhs.flags.ece == rhs.flags.ece &&
           lhs.flags.urg == rhs.flags.urg &&
           lhs.flags.ack == rhs.flags.ack &&
           lhs.flags.psh == rhs.flags.psh &&
           lhs.flags.rst == rhs.flags.rst &&
           lhs.flags.syn == rhs.flags.syn &&
           lhs.flags.fin == rhs.flags.fin &&
           lhs.window == rhs.window &&
           lhs.checksum == rhs.checksum &&
           lhs.urgentPointer == rhs.urgentPointer &&
           lhs.payloadSize == rhs.payloadSize &&
           memcmp(lhs.payload, rhs.payload, lhs.payloadSize) == 0;
}

Segment copySegment(const Segment &source)
{
    Segment copy = {};

    copy.sourcePort = source.sourcePort;
    copy.destPort = source.destPort;
    copy.seqNum = source.seqNum;
    copy.ackNum = source.ackNum;
    copy.data_offset = source.data_offset;
    copy.reserved = source.reserved;
    copy.flags.cwr = source.flags.cwr;
    copy.flags.ece = source.flags.ece;
    copy.flags.urg = source.flags.urg;
    copy.flags.ack = source.flags.ack;
    copy.flags.psh = source.flags.psh;
    copy.flags.rst = source.flags.rst;
    copy.flags.syn = source.flags.syn;
    copy.flags.fin = source.flags.fin;
    copy.window = source.window;
    copy.checksum = source.checksum;
    copy.urgentPointer = source.urgentPointer;
    copy.payloadSize = source.payloadSize;

    if (source.payload != nullptr && source.payloadSize > 0)
    {

        copy.payload = new uint8_t[source.payloadSize];
        std::memcpy(copy.payload, source.payload, source.payloadSize);
    }
    else
    {
        copy.payload = nullptr;
    }

    return copy;
}

void serializeSegment(const Segment &segment, uint8_t *buffer)
{
    memcpy(buffer, &segment.sourcePort, sizeof(segment.sourcePort));
    memcpy(buffer + 2, &segment.destPort, sizeof(segment.destPort));
    memcpy(buffer + 4, &segment.seqNum, sizeof(segment.seqNum));
    memcpy(buffer + 8, &segment.ackNum, sizeof(segment.ackNum));

    buffer[12] = (segment.data_offset << 4) | (segment.reserved & 0x0F);

    memcpy(buffer + 13, &segment.flags, sizeof(segment.flags));
    memcpy(buffer + 14, &segment.window, sizeof(segment.window));
    memcpy(buffer + 16, &segment.checksum, sizeof(segment.checksum));
    memcpy(buffer + 18, &segment.urgentPointer, sizeof(segment.urgentPointer));
    memcpy(buffer + 20, segment.payload, segment.payloadSize);
}

Segment deserializeSegment(const uint8_t *buffer, uint32_t length)
{
    Segment segment = {};
    memcpy(&segment.sourcePort, buffer, sizeof(segment.sourcePort));
    memcpy(&segment.destPort, buffer + 2, sizeof(segment.destPort));
    memcpy(&segment.seqNum, buffer + 4, sizeof(segment.seqNum));
    memcpy(&segment.ackNum, buffer + 8, sizeof(segment.ackNum));

    segment.data_offset = (buffer[12] >> 4) & 0x0F;
    segment.reserved = buffer[12] & 0x0F;

    memcpy(&segment.flags, buffer + 13, sizeof(segment.flags));
    memcpy(&segment.window, buffer + 14, sizeof(segment.window));
    memcpy(&segment.checksum, buffer + 16, sizeof(segment.checksum));
    memcpy(&segment.urgentPointer, buffer + 18, sizeof(segment.urgentPointer));

    segment.payloadSize = length - 20;
    segment.payload = new uint8_t[segment.payloadSize];
    memcpy(segment.payload, buffer + 20, segment.payloadSize);

    return segment;
}

Segment makeSegment(const std::string &data, uint16_t sport, uint16_t dport)
{
    Segment segment = {};

    segment.payloadSize = static_cast<uint16_t>(data.length());

    segment.payload = new uint8_t[segment.payloadSize];
    std::memcpy(segment.payload, data.c_str(), segment.payloadSize);

    segment.flags.syn = 0;

    segment.sourcePort = sport;
    segment.destPort = dport;
    segment.seqNum = 0;
    segment.ackNum = 0;
    segment.window = 0;
    segment.urgentPointer = 0;

    segment.data_offset = 5;

    return segment;
}

pair<string, string> extractMetada(const Segment &segment)
{
    if (segment.payload == nullptr || segment.payloadSize == 0)
    {
        return {"", ""};
    }

    string payloadData(reinterpret_cast<char *>(segment.payload), segment.payloadSize);

    size_t dotPos = payloadData.rfind('.');
    if (dotPos == string::npos)
    {
        return {payloadData, ""};
    }

    string fileName = payloadData.substr(0, dotPos);
    string fileExtension = payloadData.substr(dotPos + 1);
    return {fileName, fileExtension};
}

string combineAsString(const vector<Segment> &segments)
{
    string combined;
    cout << "try to combined" << endl;

    for (const auto& segment : segments)
    {
        if (segment.payload && segment.payloadSize > 0)
        {
            string temp(reinterpret_cast<char*>(segment.payload), segment.payloadSize);
            combined.append(temp);
        }
    }

    return combined;
}

vector<Segment> prepareStringSegments(const string& data, uint32_t seqNum)
{
    const size_t maxPayloadSize = 1444;
    vector<Segment> segments;

    size_t totalLength = data.size();
    size_t numSegments = (totalLength + maxPayloadSize - 1) / maxPayloadSize;

    uint32_t seqNumNow = seqNum;

    for (size_t i = 0; i < numSegments; ++i)
    {
        size_t startIdx = i * maxPayloadSize;
        size_t currentPayloadSize = std::min(maxPayloadSize, totalLength - startIdx);

        Segment seg = {};
        seg.payload = new uint8_t[currentPayloadSize];
        seg.payloadSize = currentPayloadSize;
        seg.data_offset = 5;

        memcpy(seg.payload, data.c_str() + startIdx, currentPayloadSize);

        seg.seqNum = seqNumNow;
        seqNumNow += seg.payloadSize;
        segments.push_back(seg);
    }

    Segment metadata = {};
    metadata.payloadSize = 0;
    metadata.payload = nullptr;
    metadata.seqNum = seqNumNow;
    metadata.flags.psh = 1; // EOF
    metadata.flags.fin = 1;
    metadata.ackNum = 0;
    metadata.window = 0;
    metadata.urgentPointer = 0;
    metadata.data_offset = 5;
    
    segments.push_back(metadata);

    return segments;
}