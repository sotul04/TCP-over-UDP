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

        // try to send request to broadcast
        cout << "Trying to contact the sender at " << destIP << ":" << destPort << endl;
        Connection cont =  connection->seekBroadcast(destIP, destPort);

        cout << "STARTING HANDSHAKE" << endl;

        Connection status = connection->reqHandShake(cont.ip, cont.port);
        cout << status.cont << endl;
        connection->close();
    }

    void handleMessage(void *buffer) override {
        cout << "noting" << endl;
    }
};

int main() {
    Client client("127.0.0.1", 9090, "127.0.0.1", 8080);
    client.run();
}