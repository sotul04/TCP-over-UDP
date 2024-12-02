#include "segment_handler.hpp"
#include <cstring>
#include <iostream>
#include <algorithm>

using namespace std;

void SegmentHandler::generateSegments()
{
    segmentBuffer.clear();
    const uint16_t maxPayloadSize = 1460;

    uint32_t seqNum = currentSeqNum;
    uint32_t index = 0;

    uint8_t *byteStream = static_cast<uint8_t *>(dataStream);

    while (index < dataSize)
    {
        Segment segment = {};
        segment.seqNum = seqNum;
        segment.payloadSize = min(static_cast<uint32_t>(maxPayloadSize), dataSize - index);

        segment.payload = new uint8_t[segment.payloadSize];
        memcpy(segment.payload, byteStream + index, segment.payloadSize);

        segment = updateChecksum(segment);

        segmentBuffer.push_back(segment);

        seqNum += segment.payloadSize;
        index += segment.payloadSize;
    }
}

void SegmentHandler::setDataStream(uint8_t *dataStream, uint32_t dataSize)
{
    this->dataStream = dataStream;
    this->dataSize = dataSize;
    this->dataIndex = 0;
    this->currentSeqNum = 0;
    generateSegments();
}

uint8_t SegmentHandler::getWindowSize()
{
    return windowSize;
}

vector<Segment> SegmentHandler::advanceWindow(uint8_t size)
{
    if (size > segmentBuffer.size())
    {
        size = segmentBuffer.size();
    }

    vector<Segment> segments;
    for (uint8_t i = 0; i < size; i++)
    {
        segments.push_back(segmentBuffer[i]);
    }

    segmentBuffer.erase(segmentBuffer.begin(), segmentBuffer.begin() + size);

    return segments;
}

SegmentHandler::~SegmentHandler()
{
    for (Segment &segment : segmentBuffer)
    {
        cout << "Deleting payload" << endl;
        delete[] segment.payload;
    }
}