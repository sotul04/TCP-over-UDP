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
            string outputPath = "output_"+metadata.first;
            if (!metadata.second.empty()) outputPath += "."+metadata.second;
            result.first.pop_back();
            receiveFile(result.first, outputPath);
            cout << "Received file saved "<< outputPath << endl;
            
        }
    }
    connection->close();
}
