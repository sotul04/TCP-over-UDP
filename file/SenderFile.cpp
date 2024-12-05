#include <iostream>
#include <fstream>
#include <vector>
#include <cstring>
#include <algorithm>
#include <stdexcept>
#include "../segment/segment.hpp"
#include <bitset>
const uint16_t MAX_PAYLOAD_SIZE = 1460;

std::string pdfToBinaryString(const std::string &filePath)
{
    std::ifstream file(filePath, std::ios::binary);
    if (!file)
    {
        throw std::runtime_error("Could not open the file.");
    }

    std::ostringstream binaryString;

    char byte;
    while (file.get(byte))
    {
        // Convert each byte to an 8-bit binary representation
        binaryString << std::bitset<8>(static_cast<unsigned char>(byte));
    }

    file.close();
    return binaryString.str();
}

std::vector<uint8_t> readFile(const std::string &filepath) {
    std::ifstream file(filepath, std::ios::binary | std::ios::ate);
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


int main()
{
    try
    {
        std::string filePath = "tost.pdf"; // Replace with your PDF file path
        std::string binaryString = pdfToBinaryString(filePath);

        std::cout << "Binary string representation of the PDF file:" << std::endl;
        std::cout << binaryString << std::endl;

        // Optional: Save the binary string to a file
        std::ofstream outputFile("binary_output.txt");
        outputFile << binaryString;
        outputFile.close();

        std::cout << "Binary string saved to binary_output.txt" << std::endl;
    }
    catch (const std::exception &e)
    {
        std::cerr << "Error: " << e.what() << std::endl;
    }

    return 0;
}