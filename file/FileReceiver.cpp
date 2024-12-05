#include <iostream>
#include <fstream>
#include <vector>
#include "FileTransfer.hpp"

void receiveFile(const std::vector<Segment> &segments, const std::string &outputPath) {
    std::ofstream outFile(outputPath, std::ios::binary);
    if (!outFile) {
        std::cerr << "Error: Cannot open output file " << outputPath << std::endl;
        return;
    }

    for (const auto &segment : segments) {
        outFile.write(reinterpret_cast<const char *>(segment.payload), segment.payloadSize);
    }

    outFile.close();
}