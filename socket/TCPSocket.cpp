#include "TCPSocket.hpp"
#include "../random/random.hpp"

Connection TCPSocket::reqHandShake(string destIP, uint16_t destPort)
{
    uint32_t seqNum = randomNumber();

    int retries = RETRIES;
    while (retries > 0)
    {
        try
        {
            sendSegment(syn(seqNum), ip, port);
            cout << "Sending SYN request to " << destIP << ":" << destPort << " Seq=" << seqNum << endl;

            uint8_t finackflag = FIN_ACK_FLAG;
            MessageFilter filter = MessageFilter::ipNPortNFlagsQuery(&destIP, &destPort, &finackflag);
            Message message = listen(&filter, HANDSHAKE_TIMEOUT);

            cout << "Accepted SYN-ACK from " << message.ip << ":" << message.port << " Seq=" << message.segment.seqNum << " Ack=" << message.segment.ackNum << endl;

            uint32_t ackNum = message.segment.seqNum + 1;
            seqNum++;

            sendSegment(ack(seqNum, ackNum), message.ip, message.port);
            cout << "Sending ACK to " << message.ip << ":" << message.port << " Seq=" << seqNum << " Ack=" << ackNum << endl;

            return Connection(true, message.ip, message.port, seqNum, ackNum);
        }
        catch (const TimeoutException &te)
        {
            cout << "Handshake timeout" << endl;
            retries--;
        }
    }

    return Connection(false, destIP, destPort, 0, 0);
}

Connection TCPSocket::accHandShake(string destIP = "", uint16_t destPort = 0)
{
    Message *synMessage = nullptr;
    uint32_t seqNum = randomNumber();
    uint8_t synflag = SYN_FLAG;
    if (destIP == "" && destPort == 0)
    {
        MessageFilter filter = MessageFilter::flagsQuery(&synflag);
        Message listened = listen(&filter, HANDSHAKE_TIMEOUT); 
        synMessage = &listened;
    }
    else
    {
        MessageFilter filter = MessageFilter::ipNPortNFlagsQuery(&destIP, &destPort, &synflag);
        Message listened = listen(&filter, HANDSHAKE_TIMEOUT); 
        synMessage = &listened;
    }

    cout << "Receive SYN from " << synMessage->ip << ":" << synMessage->port << " Seq=" << synMessage->segment.seqNum << endl;

    uint32_t ackNum = synMessage->segment.seqNum + 1;

    sendSegment(synAck(seqNum, ackNum), synMessage->ip, synMessage->port);

    int retries = RETRIES;
    while (retries > 0)
    {
        try {
            uint8_t ackflag = ACK_FLAG;
            uint32_t seqNumAck = seqNum + 1;
            MessageFilter filter = MessageFilter::ipNPortNAckNumNFlagsQuery(&synMessage->ip, &synMessage->port, &seqNumAck, &ackflag);
            Message ackMessage = listen(&filter, HANDSHAKE_TIMEOUT);
            cout << "Receive ACK from " << ackMessage.ip << ":" << ackMessage.port << " Ack=" << ackMessage.segment.ackNum << endl;

            return Connection(true, ackMessage.ip, ackMessage.port, ackMessage.segment.ackNum, ackMessage.segment.seqNum + 1);
        } catch (const TimeoutException &te) {
            cout << "ACK timeout from " << synMessage->ip << ":" << synMessage->port << endl;
            retries--;
        }
    }

    delete synMessage;
    return Connection(false, destIP, destPort, 0, 0);
}