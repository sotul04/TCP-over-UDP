#include "Client.hpp"

void Client::setServerPort(uint16_t port)
{
    serverPort = port;
}

void Client::run()
{
    connection->start();

    // make sure the serverPort has been set
    cout << OUT << "Trying to contact the sender at 255.255.255.255:" << serverPort << endl;
    Connection cont = connection->seekBroadcast("255.255.255.255", serverPort);

    Connection status = connection->reqHandShake(cont.ip, cont.port);
    if (status.cont)
    {
        pair<vector<Segment>, Connection> result = connection->receiveData(status.ip, status.port, status.seqNum);
        connection->accClosing(result.second.ip, result.second.port, result.second.seqNum);
        pair<string, string> metadata = extractMetada(result.first.back());
        // string input
        if (metadata.first.empty() && metadata.second.empty())
        {
            result.first.pop_back();
            string message = combineAsString(result.first);
            cout << "Received message:" << endl;
            cout << message << endl;
        }
        else // file input
        {
            // TODO: file handle
        }
    }
    connection->close();
    exit(0);
}

/*
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

        Connection status = connection->reqHandShake(cont.ip, cont.port);
        cout << status.cont << endl;
        if (status.cont) {
            pair<vector<Segment>, Connection> result = connection->receiveData(status.ip, status.port, status.seqNum);
            cout << "SEQNUM: " << result.second.seqNum << endl;
            connection->accClosing(result.second.ip, result.second.port, result.second.seqNum);
            cout << "ENDED" << endl;
            string combined = combineAsString(result.first);
            cout << "Size: " << combined.size() << endl;
            cout << combined << endl;
        }
        connection->close();
    }

    void handleMessage(void *buffer) override {
        cout << "noting" << endl;
    }
};

int main() {
    Client client("172.31.90.136", 9090, "255.255.255.255", 8080);
    client.run();
}

*/