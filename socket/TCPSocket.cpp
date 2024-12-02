#include "TCPSocket.hpp"
#include "../random/random.hpp"

// Request Handshake (Client-side)
Connection TCPSocket::reqHandShake(string destIP, uint16_t destPort)
{
    uint32_t seqNum = randomNumber();

    for (int retries = RETRIES; retries > 0; --retries)
    {
        try
        {
            cout << "Retry #" << (RETRIES - retries + 1) << ": Sending SYN to " << destIP << ":" << destPort << endl;

            // Send SYN
            sendSegment(syn(seqNum), destIP, destPort);
            cout << "SYN sent: Seq=" << seqNum << endl;

            // Wait for SYN-ACK
            uint8_t synAckFlag = SYN_ACK_FLAG;
            MessageFilter filter = MessageFilter::ipNPortNFlagsQuery(destIP, destPort, synAckFlag);
            Message synAckMessage = listen(&filter, HANDSHAKE_TIMEOUT);

            cout << "SYN-ACK received: From " << synAckMessage.ip << ":" << synAckMessage.port
                 << " Seq=" << synAckMessage.segment.seqNum
                 << " Ack=" << synAckMessage.segment.ackNum << endl;

            // Send ACK
            uint32_t ackNum = synAckMessage.segment.seqNum + 1;
            seqNum++;
            sendSegment(ack(seqNum, ackNum), destIP, destPort);
            cout << "ACK sent: Seq=" << seqNum << " Ack=" << ackNum << endl;

            return Connection(true, destIP, destPort, seqNum, ackNum);
        }
        catch (const TimeoutException &te)
        {
            cout << "Timeout waiting for SYN-ACK. Retrying..." << endl;
        }
        catch (const std::exception &e)
        {
            cerr << "Error during handshake: " << e.what() << endl;
        }
    }

    cout << "Handshake failed after " << RETRIES << " retries." << endl;
    return Connection(false, destIP, destPort, 0, 0);
}

// Accept Handshake (Server-side)
Connection TCPSocket::accHandShake(string destIP, uint16_t destPort)
{
    uint32_t seqNum = randomNumber();

    for (int retries = RETRIES; retries > 0; --retries)
    {
        try
        {
            cout << "Listening for SYN..." << endl;

            // Wait for SYN
            uint8_t synFlag = SYN_FLAG;
            MessageFilter filter = (destIP.empty() && destPort == 0)
                                       ? MessageFilter::flagsQuery(synFlag)
                                       : MessageFilter::ipNPortNFlagsQuery(destIP, destPort, synFlag);

            Message synMessage = listen(&filter, HANDSHAKE_TIMEOUT);
            cout << "SYN received: From " << synMessage.ip << ":" << synMessage.port
                 << " Seq=" << synMessage.segment.seqNum << endl;

            // Send SYN-ACK
            uint32_t ackNum = synMessage.segment.seqNum + 1;
            sendSegment(synAck(seqNum, ackNum), synMessage.ip, synMessage.port);
            cout << "SYN-ACK sent: Seq=" << seqNum << " Ack=" << ackNum << endl;

            // Wait for ACK
            uint8_t ackFlag = ACK_FLAG;
            uint32_t seqNumAck = seqNum + 1;
            MessageFilter ackFilter = MessageFilter::ipNPortNAckNumNFlagsQuery(
                synMessage.ip, synMessage.port, seqNumAck, ackFlag);

            Message ackMessage = listen(&ackFilter, HANDSHAKE_TIMEOUT);
            cout << "ACK received: From " << ackMessage.ip << ":" << ackMessage.port
                 << " Seq=" << ackMessage.segment.seqNum
                 << " Ack=" << ackMessage.segment.ackNum << endl;

            return Connection(true, ackMessage.ip, ackMessage.port, ackMessage.segment.ackNum, ackMessage.segment.seqNum + 1);
        }
        catch (const TimeoutException &te)
        {
            cout << "Timeout waiting for handshake completion. Retrying..." << endl;
        }
        catch (const std::exception &e)
        {
            cerr << "Error during handshake: " << e.what() << endl;
        }
    }

    cout << "Handshake failed after " << RETRIES << " retries." << endl;
    return Connection(false, destIP, destPort, 0, 0);
}
