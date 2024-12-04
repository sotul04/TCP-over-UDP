#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include "../segment/segment.hpp"

const uint16_t MAX_PAYLOAD_SIZE = 1460;

std::vector<uint8_t> readFile(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary);
    if (!file) {
        throw std::runtime_error("Unable to open file: " + filepath);
    }

    return std::vector<uint8_t>(std::istreambuf_iterator<char>(file),{});
}

std::vector<Segment> createSegmentsFromFile(const std::vector<uint8_t> &fileData, uint16_t sourcePort, uint16_t destPort) {
    std::vector<Segment> segments;
    uint32_t seqNum = 0;
    for (size_t i = 0; i < fileData.size(); i += MAX_PAYLOAD_SIZE) {
        size_t chunkSize = std::min<size_t>(MAX_PAYLOAD_SIZE, fileData.size() - i);
        std::string chunk(reinterpret_cast<const char *>(&fileData[i], chunkSize));

        Segment segment = makeSegment(chunk, sourcePort, destPort);
        segment.seqNum = seqNum++;

        segment = updateChecksum(segment);
        segments.push_back(segment);
    }

    return segments;
}

void cleanupSegments(std::vector<Segment> &segments) {
    for (auto &segment : segments) {
        delete[] segment.payload;
        segment.payload = nullptr;
    }
}

int main() {
    try {
        std::string filePath = "Test.pdf";
        uint16_t sourcePort = 12345;
        uint16_t destPort = 80;

        auto fileData = readFile(filePath);
        auto segments = createSegmentsFromFile(fileData, sourcePort, destPort);

        for (const auto &segment : segments) {
            printSegment(segment);
        }

        cleanupSegments(segments);

        std::cout << "File split into segments successfully!" << std::endl;
    } catch (const std::exception &e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}