#include <iostream>
#include <fstream>
#include <vector>
#include "FileTransfer.hpp"
#include <filesystem>
#include <string>

vector<Segment> sendFile (const std::string &filePath, uint32_t seqNum) {
    std::ifstream file(filePath, std::ios::binary);
    if (!file) {
        std::cerr << "Error: cannot open file " << filePath << std::endl;
        vector<Segment> segment;
        return segment;
    }

    vector<Segment> segments;

    const size_t SEGMENT_SIZE = 1460;
    uint8_t buffer[SEGMENT_SIZE];
    uint32_t currentSeqNum = seqNum;

    while (file.read(reinterpret_cast<char *>(buffer), SEGMENT_SIZE) || file.gcount() > 0) {
        size_t bytesRead = file.gcount();
        Segment segment = {};

        segment.seqNum = currentSeqNum;
        currentSeqNum += SEGMENT_SIZE;
        
        segment.payload = new uint8_t[bytesRead];
        segment.payloadSize = static_cast<uint16_t>(bytesRead);
        segment.data_offset = 5;

        std::memcpy(segment.payload, buffer, bytesRead);

        segments.push_back(segment);

    }

    std::string fileName = std::filesystem::path(filePath).filename().string();
    Segment metadata = {};
    if (fileName.size() > 180) fileName = fileName.substr(fileName.size()-180);
    metadata.payloadSize = fileName.size();
    metadata.payload = new uint8_t[fileName.size()];
    std::memcpy(metadata.payload, fileName.data(), fileName.size());
    metadata.seqNum = currentSeqNum;
    metadata.flags.psh = 1; // EOF
    metadata.flags.fin = 1;
    metadata.ackNum = 0;
    metadata.window = 0;
    metadata.urgentPointer = 0;
    metadata.data_offset = 5;

    segments.push_back(metadata);
    file.close();
    return segments;
}