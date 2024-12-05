#include <iostream>
#include <fstream>
#include <vector>
#include "FileTransfer.hpp"

void sendFile (const std::string &filePath, std::vector<Segment> &segments, uint16_t sourcePort, uint16_t destPort) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error: cannot open file " << filePath << std::endl;
        return;
    }

    const size_t SEGMENT_SIZE = 1460;
    uint8_t buffer[SEGMENT_SIZE];
    uint32_t seqNum = 0;

    while (file.read(reinterpret_cast<char *>(buffer), SEGMENT_SIZE) || file.gcount() > 0) {
        size_t bytesRead = file.gcount();
        Segment segment = {};
        segment.sourcePort = sourcePort;
        segment.destPort = destPort;
        segment.seqNum = seqNum++;
        segment.payload = new uint8_t[bytesRead];
        segment.payloadSize = static_cast<uint16_t>(bytesRead);
        std::memcpy(segment.payload, buffer, bytesRead);
        segment = updateChecksum(segment);
        segments.push_back(segment);
    }

    file.close();
}