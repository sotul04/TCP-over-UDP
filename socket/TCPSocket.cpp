#include "TCPSocket.hpp"
#include "../random/random.hpp"

Connection TCPSocket::seekBroadcast(string destIP, uint16_t destPort)
{
    bindAddress();
    setBroadcast();
    int retries = 3;
    while (retries > 0)
    {
        try
        {
            Segment searchMsg = makeSegment("SEARCHING", port, destPort);
            sendSegment(searchMsg, destIP, destPort);

            const char *payload = "APPROVED";
            MessageFilter filter = MessageFilter().withPayloads((uint8_t *)payload, strlen(payload));
            Message response = listen(&filter, HANDSHAKE_TIMEOUT);

            this->ip = response.ip;
            this->port = response.port;

            return Connection(true, response.ip, response.port, 0, 0);
        }
        catch (const TimeoutException &te)
        {
            cout << "REQUEST TIMEOUT, retrying... " << endl;
            retries--;
        }
    }
    cout << "FAILED to discover SERVER, terminating... " << endl;
    exit(EXIT_SUCCESS);
}

Connection TCPSocket::listenBroadcast()
{
    try
    {
        bindAddress();
        const char *payload = "SEARCHING";
        MessageFilter filter = MessageFilter().withPayloads((uint8_t *)payload, strlen(payload));
        Message searchingMsg = listen(&filter, BROADCAST_LISTEN_TIMEOUT);

        Segment response = makeSegment("APPROVED", port, searchingMsg.port);
        sendSegment(response, searchingMsg.ip, searchingMsg.port);
        return Connection(true, searchingMsg.ip, searchingMsg.port, 0, 0);
    }
    catch (const TimeoutException &te)
    {
        cout << "LISTEN TIMEOUT, terminating..." << endl;
        exit(EXIT_SUCCESS);
    }
}

// Request Handshake (Client-side)
Connection TCPSocket::reqHandShake(string destIP, uint16_t destPort)
{

    for (int retries = RETRIES; retries > 0; --retries)
    {
        try
        {
            uint32_t seqNum = randomNumber();
            sendSegment(syn(seqNum), destIP, destPort);
            setStatus(SYN_SENT);
            cout << OUT << logStatus() << " [S=" << seqNum << "]" << "Sending SYN request to " << destIP << ":" << destPort << endl;

            MessageFilter filter = MessageFilter().withIP(destIP).withPort(destPort).withFlags(SYN_ACK_FLAG).withAckNum(++seqNum);
            Message synAckMessage = listen(&filter, HANDSHAKE_TIMEOUT);

            setStatus(ESTABLISHED);

            cout << IN << logStatus()
                 << "[S=" << synAckMessage.segment.seqNum
                 << "] [A=" << synAckMessage.segment.ackNum
                 << "] Received SYN-ACK request from " << synAckMessage.ip << ":" << synAckMessage.port << endl;

            uint32_t ackNum = synAckMessage.segment.seqNum + 1;
            sendSegment(ack(ackNum), destIP, destPort);
            cout << OUT << logStatus() << "[A=" << ackNum << "] Sending ACK request to " << destIP << ":" << destPort << endl;
            cout << IN << "Ready to receive input from " << destIP << ":" << destPort << endl;
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
    try
    {
        uint32_t seqNum = randomNumber();
        // Wait for SYN
        MessageFilter filter = (destIP.empty() && destPort == 0)
                                   ? MessageFilter().withFlags(SYN_FLAG)
                                   : MessageFilter().withIP(destIP).withPort(destPort).withFlags(SYN_FLAG);

        Message synMessage = listen(&filter, HANDSHAKE_TIMEOUT);
        setStatus(SYN_RECEIVED);
        cout << IN << logStatus()
             << "[S=" << synMessage.segment.seqNum
             << "] Received SYN request from " << synMessage.ip << ":" << synMessage.port << endl;
        for (int retries = RETRIES; retries > 0; --retries)
        {
            try
            {
                // Send SYN-ACK
                uint32_t ackNum = synMessage.segment.seqNum + 1;
                sendSegment(synAck(seqNum, ackNum), synMessage.ip, synMessage.port);
                cout << OUT << logStatus() << "[S=" << seqNum << "] [A=" << ackNum << "] Sending SYN-ACK request to " << destIP << ":" << destPort << endl;

                MessageFilter ackFilter = MessageFilter().withIP(synMessage.ip).withPort(synMessage.port).withAckNum(++seqNum).withFlags(ACK_FLAG);
                Message ackMessage = listen(&ackFilter, HANDSHAKE_TIMEOUT);
                setStatus(ESTABLISHED);
                cout << IN << logStatus() << "[A=" << ackMessage.segment.ackNum << "] Received ACK request from" << ":"
                     << destIP << ":" << destPort << endl;

                return Connection(true, ackMessage.ip, ackMessage.port, ackNum, ackMessage.segment.seqNum);
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
    catch (const TimeoutException &e)
    {
    }

    cout << "Timeout waiting for handshake request." << endl;
    return Connection(false, destIP, destPort, 0, 0);
}

Connection TCPSocket::reqClosing(string destIP, uint16_t destPort, uint32_t finSeqNum)
{
    for (int i = RETRIES; i > 0; i--)
    {
        try
        {
            cout << OUT << "[CLOSING] " << "Sending FIN request to " << destIP << ":" << destPort << endl;
            sendSegment(fin(finSeqNum), destIP, destPort);

            MessageFilter filter = MessageFilter().withIP(destIP).withPort(destPort).withFlags(FIN_ACK_FLAG).withAckNum(++finSeqNum);
            Message finackmsg = listen(&filter, HANDSHAKE_TIMEOUT);

            cout << IN << "[CLOSING] " << "Received FIN-ACK request from " << destIP << ":" << destPort << endl;

            sendSegment(ack(finackmsg.segment.seqNum + 1), destIP, destPort);
            cout << OUT << "[CLOSING] " << "Sending ACK request to " << destIP << ":" << destPort << endl;
            return Connection(true, destIP, destPort, 0, 0);
        }
        catch (const TimeoutException &te)
        {
            cout << "Timeout waiting for FIN-ACK. Retrying..." << endl;
        }
        catch (const std::exception &e)
        {
            cerr << "Error during closing: " << e.what() << endl;
        }
    }
    cout << "Timeout waiting FIN-ACK request." << endl;
    return Connection(false, destIP, destPort, 0, 0);
}

Connection TCPSocket::accClosing(string destIP, uint16_t destPort, uint32_t finSeqNum)
{
    try
    {
        MessageFilter finfilter = MessageFilter().withIP(destIP).withPort(destPort).withFlags(FIN_FLAG).withSeqNum(finSeqNum);
        Message finmsg = listen(&finfilter, HANDSHAKE_TIMEOUT);
        cout << IN << "[CLOSING] " << "Received FIN request from " << destIP << ":" << destPort << endl;
        uint32_t seqNum = randomNumber();

        for (int i = RETRIES; i > 0; i--)
        {
            try
            {
                sendSegment(finAck(finmsg.segment.seqNum, finmsg.segment.seqNum + 1), destIP, destPort);
                cout << OUT << "[CLOSING] " << "Sending FIN-ACK request to " << destIP << ":" << destPort << endl;

                MessageFilter ackfilter = MessageFilter().withIP(destIP).withPort(destPort).withFlags(ACK_FLAG).withAckNum(finmsg.segment.seqNum + 1);
                Message ackmsg = listen(&ackfilter, HANDSHAKE_TIMEOUT);
                cout << IN << "[CLOSING] " << "Received ACK request from " << ackmsg.ip << ":" << ackmsg.port << endl;
                return Connection(true, destIP, destPort, 0, 0);
            }
            catch (const TimeoutException &te)
            {
                cout << "Timeout waiting for ACK. Retrying..." << endl;
            }
            catch (const std::exception &e)
            {
                cerr << "Error during closing: " << e.what() << endl;
            }
        }
        cout << "Timeout waiting for ACK request.";
        return Connection(false, destIP, destPort, 0, 0);
    }
    catch (const TimeoutException &te)
    {
    }

    cout << "Timeout waiting for FIN request." << endl;
    return Connection(false, destIP, destPort, 0, 0);
}

void TCPSocket::senderThread(const Message &message, uint32_t current, std::atomic<int> &lastAck, std::atomic<bool> &abort)
{
    int retries = RETRIES;
    while (retries > 0)
    {
        try
        {
            sendSegment(message.segment, message.ip, message.port);
            cout << OUT << logStatus() << "[Seg " << current << "] [S=" << message.segment.seqNum << "] Sent" << endl;

            MessageFilter ackFilter = MessageFilter()
                                          .withIP(message.ip)
                                          .withPort(message.port)
                                          .withFlags(ACK_FLAG)
                                          .withAckNum(message.segment.seqNum + message.segment.payloadSize);
            Message ackMsg = listen(&ackFilter, RETRANSMIT_TIMEOUT);

            cout << IN << logStatus() << "[Seg " << current << "] [A=" << ackMsg.segment.ackNum << "] ACKed" << endl;
            while (lastAck.load() + 1 != current)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            lastAck.store(current);
            return;
        }
        catch (const TimeoutException &)
        {
            cout << "[TIMEOUT] No ACK received for [S=" << message.segment.seqNum << "]. Retrying..." << endl;
            retries--;
        }
    }
    cout << "[ERROR] Failed to send segment [S=" << message.segment.seqNum << "] after retries." << endl;
    abort.store(true);
}

Connection TCPSocket::sendData(string destIP, uint16_t destPort, uint32_t seqNum, vector<Segment> data)
{
    const uint32_t SWS = WINDOW_SIZE; // Sender Window Size
    uint32_t LAR = 0;                 // Last Acknowledgment Received
    uint32_t LFS = 0;                 // Last Frame Sent

    cout << OUT << "Sending input to " << destIP << ":" << destPort << endl;

    std::atomic<int> lastAck(0);
    std::atomic<bool> abort(false);
    vector<std::thread> threads;

    while (LFS < data.size())
    {
        // Slide the window
        while (LFS - LAR < SWS && LFS < data.size())
        {
            // Validate index bounds
            Message msg(destIP, destPort, data.at(LFS));
            threads.push_back(thread(&TCPSocket::senderThread, this,
                                     msg,
                                     ++LFS,
                                     std::ref(lastAck), std::ref(abort)));
            std::this_thread::sleep_for(std::chrono::milliseconds(20));
        }

        if (abort.load())
        {
            cerr << "[ABORT] Sending aborted due to errors." << endl;
            break;
        }

        if (lastAck.load() > LAR)
        {
            LAR = lastAck.load();
        }
    }

    // Join all threads
    for (auto &t : threads)
    {
        if (t.joinable())
        {
            t.join();
        }
    }

    cout << OUT << "[SEND COMPLETE] All segments sent to " << destIP << ":" << destPort << endl;
    return Connection(true, destIP, destPort, data.at(data.size()-1).seqNum+data.at(data.size()-1).payloadSize, 0);
}

pair<vector<Segment>, Connection> TCPSocket::receiveData(string destIP, uint16_t destPort, uint32_t seqNum)
{
    vector<Segment> receivedSegments;

    bool finished = false;

    uint32_t targetSeqNum = seqNum;
    int timeoutLimit = 0;

    while (!finished && timeoutLimit < 20)
    {
        try
        {
            MessageFilter filter = MessageFilter().withIP(destIP).withPort(destPort);
            Message message = listen(&filter, RETRANSMIT_TIMEOUT);

            if (message.segment.seqNum < targetSeqNum)
            {
                sendSegment(ack(message.segment.seqNum + message.segment.payloadSize), message.ip, message.port);
            }
            else if (message.segment.seqNum == targetSeqNum)
            {
                // metadata
                if (message.segment.flags.fin == 1 && message.segment.flags.psh == 1)
                {
                    cout << IN << logStatus() << "Received metada from " << message.ip << ":" << message.port << endl;
                    sendSegment(ack(message.segment.seqNum + message.segment.payloadSize), message.ip, message.port);
                    pair<string, string> metadata = extractMetada(message.segment);
                    receivedSegments.push_back(copySegment(message.segment));
                    cout << IN << "[METADATA] name: " << metadata.first << " extension: " << metadata.second << endl;
                    cout << OUT << logStatus() << "Sending ACK for metadata to " << message.ip << ":" << message.port << endl;
                    targetSeqNum += message.segment.payloadSize;
                    finished = true;
                    continue;
                }
                receivedSegments.push_back(copySegment(message.segment));
                cout << IN << logStatus() << "[Seq " << receivedSegments.size() << "] [S=" << message.segment.seqNum << "] ACKed" << endl;

                sendSegment(ack(targetSeqNum + message.segment.payloadSize), message.ip, message.port);
                targetSeqNum += message.segment.payloadSize;
                cout << OUT << logStatus() << "[Seq " << receivedSegments.size() << "] [A=" << targetSeqNum << "] Sent" << endl;
            }
        }
        catch (const TimeoutException &)
        {
            timeoutLimit++;
            cout << "[TIMEOUT] Waiting for segment [S=" << targetSeqNum << "] from " << destIP << ":" << destPort << endl;
        }
    }

    return {receivedSegments, Connection(true, destIP, destPort, targetSeqNum, 0)};
}
