#include "node.hpp"

class Client: public Node {
private:
    string destIP;
    uint16_t destPort;
public:
    Client(string ip, uint16_t port, string destIP, uint16_t destPort): Node(ip, port) {
        this->destIP = destIP;
        this->destPort = destPort;
    }

    void run() {
        connection->start();
        uint8_t hello[] = "Hello from me";

        Connection status = connection->reqHandShake(destIP, destPort);
        cout << status.cont << endl;
        connection->stop();
    }

    void handleMessage(void *buffer) override {
        cout << "noting" << endl;
    }
};

int main() {
    Client client("172.31.90.136", 9090, "172.31.90.136", 8080);
    client.run();
}