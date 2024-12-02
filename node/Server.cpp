#include "node.hpp"

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
        connection->close();
    }

    void handleMessage(void *buffer) override {
        cout << "nothing" << endl;
    }
};

int main() {
    Server server("127.0.0.1", 8080);
    server.run();
}