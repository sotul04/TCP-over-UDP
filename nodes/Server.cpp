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
