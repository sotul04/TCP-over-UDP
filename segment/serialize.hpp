#ifndef serialize_h
#define serialize_h

#include "segment.hpp"
#include <cstring>

void serializeSegment(const Segment &segment, uint8_t *buffer);
Segment deserializeSegment(const uint8_t *buffer, uint32_t length);

#endif