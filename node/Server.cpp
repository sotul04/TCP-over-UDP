#include "node.hpp"

class Server: public Node {
public:
    Server(string ip, uint16_t port): Node(ip, port) {}

    void run() {
        connection->start();
        Connection status = connection->accHandShake("", 0);
        cout << status.cont << endl;
        connection->stop();
    }

    void handleMessage(void *buffer) override {
        cout << "nothing" << endl;
    }
};

int main() {
    Server server("172.31.90.136", 8080);
    server.run();
}