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
        MessageFilter finfilter = MessageFilter().withIP(destIP).withPort(destPort).withFlags(FIN_FLAG);
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

void TCPSocket::senderThread(const Message &message, std::atomic<int> &lastAck, std::atomic<bool> &abort)
{
    int retries = RETRIES;
    while (retries > 0)
    {
        try
        {
            sendSegment(message.segment, message.ip, message.port);
            cout << "[SENDING] Segment [S=" << message.segment.seqNum << "] sent to " << message.ip << ":" << message.port << endl;

            MessageFilter ackFilter = MessageFilter()
                                          .withIP(message.ip)
                                          .withPort(message.port)
                                          .withFlags(ACK_FLAG)
                                          .withAckNum(message.segment.seqNum + 1);
            Message ackMsg = listen(&ackFilter, RETRANSMIT_TIMEOUT);

            // lastAck.store(message.segment.seqNum + 1);
            cout << "[ACK RECEIVED] ACK for [S=" << message.segment.seqNum << "] from " << message.ip << ":" << message.port << endl;
            while (lastAck.load() + 1 != message.segment.seqNum)
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(50));
            }
            lastAck.store(message.segment.seqNum);
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
    const uint32_t SWS = 5;    // Sender Window Size
    uint32_t LAR = seqNum - 1; // Last Acknowledgment Received
    uint32_t LFS = seqNum - 1; // Last Frame Sent

    cout << "[INFO] Sending data to " << destIP << ":" << destPort << endl;

    std::atomic<int> lastAck(seqNum - 1);
    std::atomic<bool> abort(false);
    vector<std::thread> threads;

    while (LFS < seqNum + data.size() - 1)
    {
        // Slide the window
        while (LFS - LAR < SWS && LFS < seqNum + data.size() - 1)
        {
            // Validate index bounds
            if ((LFS - seqNum + 1) >= 0)
            {
                threads.push_back(thread(&TCPSocket::senderThread, this,
                                         Message(destIP, destPort, data[LFS - seqNum + 1]),
                                         std::ref(lastAck), std::ref(abort)));
                cout << "[WINDOW] Sending frame " << LFS << endl;
                LFS++;
            }
            std::this_thread::sleep_for(std::chrono::milliseconds(5)); // Avoid busy loop
        }

        if (abort.load())
        {
            cerr << "[ABORT] Sending aborted due to errors." << endl;
            break;
        }

        // Update LAR based on last acknowledgment
        if (lastAck.load() > LAR)
        {
            LAR = lastAck.load();
            cout << "[WINDOW MOVED] LAR=" << LAR << endl;
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

    cout << "[SEND COMPLETE] All segments sent to " << destIP << ":" << destPort << endl;
    return Connection(true, destIP, destPort, lastAck.load() + 1, 0);
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
                sendSegment(ack(message.segment.seqNum + 1), message.ip, message.port);
            }
            else if (message.segment.seqNum == targetSeqNum)
            {
                // metadata
                if (message.segment.flags.fin == 1 && message.segment.flags.psh == 1)
                {
                    cout << IN << logStatus() << "Received metada from " << message.ip << ":" << message.port << endl;
                    sendSegment(ack(message.segment.seqNum + 1), message.ip, message.port);
                    pair<string, string> metadata = extractMetada(message.segment);
                    cout << "[METADATA] name: " << metadata.first << " extension: " << metadata.second << endl;
                    cout << OUT << logStatus() << "Sending ACK for metadata to " << message.ip << ":" << message.port << endl;
                    finished = true;
                    continue;
                }
                receivedSegments.push_back(message.segment);
                cout << IN << logStatus() << "[Seq " << receivedSegments.size() << "] [S=" << message.segment.seqNum << "] from " << message.ip << ":" << message.port << " ACKed" << endl;

                sendSegment(ack(targetSeqNum + 1), message.ip, message.port);
                targetSeqNum++;
                cout << OUT <<  logStatus() << "[Seq " << receivedSegments.size() << "] [A=" << targetSeqNum << "] Sent to " << message.ip << ":" << message.port << endl;
            }
        }
        catch (const TimeoutException &)
        {
            timeoutLimit++;
            cout << "[TIMEOUT] Waiting for segment [S=" << targetSeqNum << "] from " << destIP << ":" << destPort << endl;
        }
    }

    return {receivedSegments, Connection(true, destIP, destPort, targetSeqNum+2, 0)};
}
