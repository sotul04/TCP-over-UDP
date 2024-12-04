#include "node.hpp"

vector<Segment> createSegmentsFromString(const string &data, uint32_t seqNum) {
    const size_t maxPayloadSize = 1460;  // Max payload size for each segment
    vector<Segment> segments;

    size_t totalLength = data.size();
    size_t numSegments = (totalLength + maxPayloadSize - 1) / maxPayloadSize;  // Calculate number of segments

    for (size_t i = 0; i < numSegments; ++i) {
        // Get the starting index for the current segment's data
        size_t startIdx = i * maxPayloadSize;
        // Get the length of the current segment's payload (it might be smaller than maxPayloadSize for the last segment)
        size_t currentPayloadSize = std::min(maxPayloadSize, totalLength - startIdx);

        // Create a new Segment for the current chunk of data
        Segment seg;
        seg.payload = new uint8_t[currentPayloadSize];  // Allocate memory for payload
        seg.payloadSize = currentPayloadSize;

        // Copy the data from the string into the segment's payload
        memcpy(seg.payload, data.c_str() + startIdx, currentPayloadSize);

        // Set the segment's sequence number
        seg.seqNum = seqNum + i;

        // Update the checksum for the segment
        seg = updateChecksum(seg);

        // Optionally, you can set other fields in the segment, such as ackNum, flags, etc.
        // For this example, we'll leave the segment's other fields empty, but you can adjust as necessary.
        
        // Push the segment to the segments vector
        segments.push_back(seg);
    }

    return segments;
}

class Server: public Node {
public:
    Server(string ip, uint16_t port): Node(ip, port) {}

    void run() {
        connection->start();

        // listening to broadcast
        cout << "Listening to the broadcast port for clients." << endl;
        Connection cont = connection->listenBroadcast();

        Connection status = connection->accHandShake(cont.ip, cont.port);


        cout << status.cont << endl;
        if (status.cont) {
            string cutt = "One would argue that in such an arrangement the amount of data in transit decreases once a timeout occurs";
            string b;
            for (int i = 0; i < 1000; i ++) {
                b.append(cutt);
            }
            cout << b.size() << endl;
            vector<Segment> data = createSegmentsFromString(b, status.seqNum);
            Segment metadata = makeSegment("string", 0, status.port);
            metadata.seqNum = data.back().seqNum + 1;
            metadata.flags.psh = 1;
            metadata.flags.fin = 1;
            data.push_back(metadata);
            cout << data.at(0).seqNum << " : " << data.size() << endl;
            status = connection->sendData(status.ip, status.port, status.seqNum, data);

            cout << "-----------------FIN " << status.seqNum << endl;

            // std::this_thread::sleep_for(std::chrono::seconds)

            connection->reqClosing(status.ip, status.port, status.seqNum);
        }
        connection->close();
    }

    void handleMessage(void *buffer) override {
        cout << "nothing" << endl;
    }
};

int main() {
    Server server("0.0.0.0", 8080);
    server.run();
}

