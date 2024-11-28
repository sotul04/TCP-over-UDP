#include "segment_handler.hpp"
#include <iostream>
#include <cstring>
using namespace std;

int main() {

    string lorem = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";

    string concatenated;
    for (int i = 0; i < 1000; i++) {
        concatenated += lorem;
    }

    uint32_t dataSize = concatenated.size();
    uint8_t *data = new uint8_t[dataSize];
    memcpy(data, concatenated.c_str(), dataSize);

    cout << "Length: " << dataSize << endl;

    SegmentHandler handler;
    handler.setDataStream(data, dataSize);

    cout << "Window Size: " << static_cast<int>(handler.getWindowSize()) << endl;

    vector<Segment> segments = handler.advanceWindow(39);

    for (int i = 0; i < segments.size(); i++) {
        cout << "Segment " << i + 1 << "\t| SeqNum: " << segments[i].seqNum
             << "\t| PayloadSize: " << static_cast<int>(segments[i].payloadSize) << endl;
        delete[] segments[i].payload;
    }

    segments.clear();

    return 0;
}
