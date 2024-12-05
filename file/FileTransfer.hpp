#ifndef FILE_TRANSFER_HPP
#define FILE_TRANSFER_HPP

#include <vector>
#include <string>
#include "../segment/segment.hpp"

// Function prototypes for file operations
void sendFile(const std::string &filePath, std::vector<Segment> &segments, uint16_t sourcePort, uint16_t destPort);
void receiveFile(const std::vector<Segment> &segments, const std::string &outputPath);

#endif // FILE_TRANSFER_HPP
