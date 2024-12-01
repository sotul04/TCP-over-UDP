#ifndef segment_handler_h
#define segment_handler_h

#include "segment.hpp"
#include <vector>
using namespace std;

class SegmentHandler
{
private:
    uint8_t windowSize;
    uint32_t currentSeqNum;
    uint32_t currentAckNum;
    void *dataStream;
    uint32_t dataSize;
    uint32_t dataIndex;
    vector<Segment> segmentBuffer;

    void generateSegments();

public:
    SegmentHandler() : windowSize(0), currentSeqNum(0), currentAckNum(0),
                       dataStream(nullptr), dataSize(0), dataIndex(0) {}

    ~SegmentHandler();

    void setDataStream(uint8_t *dataStream, uint32_t dataSize);
    uint8_t getWindowSize();
    vector<Segment> advanceWindow(uint8_t size);
};

#endif