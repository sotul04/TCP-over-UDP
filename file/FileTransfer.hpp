#ifndef FILE_TRANSFER_HPP
#define FILE_TRANSFER_HPP

#include <vector>
#include <string>
#include "../segment/segment.hpp"

// Function prototypes for file operations
vector<Segment> sendFile(const std::string &filePath, uint32_t seqNum);
void receiveFile(const std::vector<Segment> &segments, const std::string &outputPath);

#endif // FILE_TRANSFER_HPP
