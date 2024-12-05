#include <iostream>
#include <vector>
#include "FileTransfer.hpp"
#include "../segment/segment.hpp"

int main() {
    std::string inputFilePath = "c++.png"; 
    std::string outputFilePath = "output_C++.PNG"; 

    std::vector<Segment> segments;
    sendFile(inputFilePath, segments, 12345, 54321);
    std::cout << "File has been split into " << segments.size() << " segments.\n";

    receiveFile(segments, outputFilePath);
    std::cout << "File has been reassembled and written to " << outputFilePath << "\n";

    for (auto &segment : segments) {
        delete[] segment.payload;
        segment.payload = nullptr;
    }

    return 0;
}
// g++ -o main file/FileSender.cpp file/FileReceiver.cpp file/DriverFile.cpp segment/segment.cpp