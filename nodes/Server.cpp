#include "Server.hpp"

void Server::run()
{
    connection->start();

    // listening to the broad
    cout << OUT << "Listening to the broadcast port for clients." << endl;
    Connection conn = connection->listenBroadcast();
    Connection status = connection->accHandShake(conn.ip, conn.port);

    if (status.cont)
    {
        // data has set using setData method
        updateSeqNum(status.seqNum);
        status = connection->sendData(status.ip, status.port, status.seqNum, data);
        status = connection->reqClosing(status.ip, status.port, status.seqNum);
    }
    connection->close();
    exit(0);
}

void Server::updateSeqNum(uint32_t seqNum)
{
    uint32_t current = seqNum;
    for (Segment &segment: data)
    {
        segment.seqNum = current;
        current += segment.payloadSize;
    }
}

// #include "node.hpp"

// class Server : public Node
// {
// private:
// public:
//     Server(string ip, uint16_t port, vector<Segment> data) : Node(ip, port) {}

//     void run()
//     {
//         connection->start();

//         // listening to broadcast
//         cout << "Listening to the broadcast port for clients." << endl;
//         Connection cont = connection->listenBroadcast();

//         Connection status = connection->accHandShake(cont.ip, cont.port);

//         cout << status.cont << endl;
//         if (status.cont)
//         {
//             string cutt = "Lorem Ipsum is simply dummy text of the printing and typesetting industry. Lorem Ipsum has been the industry's standard dummy text ever since the 1500s, when an unknown printer took a galley of type and scrambled it to make a type specimen book. It has survived not only five centuries, but also the leap into electronic typesetting, remaining essentially unchanged. It was popularised in the 1960s with the release of Letraset sheets containing Lorem Ipsum passages, and more recently with desktop publishing software like Aldus PageMaker including versions of Lorem Ipsum.";
//             vector<Segment> data = prepareStringSegments(cutt, status.seqNum);
//             status = connection->sendData(status.ip, status.port, status.seqNum, data);
//             connection->reqClosing(status.ip, status.port, status.seqNum);
//         }
//         connection->close();
//     }

//     void handleMessage(void *buffer) override
//     {
//         cout << "nothing" << endl;
//     }
// };

// int main()
// {
//     Server server("0.0.0.0", 8080);
//     server.run();
// }
