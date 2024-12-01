#include "serialize.hpp"

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