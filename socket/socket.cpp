#include "socket.hpp"

Socket::Socket(const string ip, const int16_t &port)
{
    this->ip = ip;
    this->port = port;

    socket = ::socket(AF_INET, SOCK_DGRAM, 0);
    if (socket < 0)
    {
        perror("Socket creation failed");
        exit(EXIT_FAILURE);
    }

    // binding
    struct sockaddr_in sock_addr = {};
    sock_addr.sin_family = AF_INET;
    sock_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    sock_addr.sin_port = htons(port);

    if (bind(socket, (const struct sockaddr *)&sock_addr, sizeof(sock_addr)) < 0)
    {
        perror("Bind failed");
        exit(EXIT_FAILURE);
    }

    segmentHandler = new SegmentHandler();
    status = CLOSED;

    cout << "Socket created" << endl;

    cleanerTime = CLEANER_TIME;
    isListening = false;
}

Socket::~Socket()
{
    delete segmentHandler;
    packetBuffer.clear();
}

void Socket::setCleanerTime(float time)
{
    this->cleanerTime = time;
}

void Socket::clearPacketBuffer()
{
    packetBuffer.clear();
}

void Socket::cleanerPacketThread()
{
    while (isListening)
    {
        try
        {
            if (!packetBuffer.empty())
            {
                Message firstMessage = packetBuffer.front();

                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(cleanerTime * 1000)));

                if (!packetBuffer.empty() && packetBuffer.front() == firstMessage)
                {
                    packetBuffer.erase(packetBuffer.begin());
                }

                if (packetBuffer.size() > CLEANER_LIMIT)
                {
                    setCleanerTime(MIN_CLEANER_TIME);
                }
                else
                {
                    setCleanerTime(CLEANER_TIME);
                }
            }
            else
            {
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(cleanerTime * 1000)));
            }
        }
        catch (exception &e)
        {
            cout << "Error in cleanerPacketThread: " << e.what() << endl;
        }
    }
}

void Socket::listenerPacketThread()
{
    cout << "Listening" << endl;

    while (isListening)
    {
        std::vector<uint8_t> buffer(MAXLINE);
        struct sockaddr_in cliaddr;
        socklen_t len = sizeof(cliaddr);
        try
        {
            int n = recvfrom(socket, buffer.data(), MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
            if (n <= 0)
            {
                throw runtime_error("Failed to receive data");
            }

            Segment received = deserializeSegment(buffer.data(), n);

            try
            {
                if (!isValidChecksum(received))
                {
                    throw exception();
                }
                Message newMessage(inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), received);
                {
                    // Protecting packetBuffer with a lock
                    std::lock_guard<std::mutex> lock(bufferMutex);
                    packetBuffer.push_back(newMessage);
                }
            }
            catch (const exception &e)
            {
                cout << "Bad packet: " << e.what() << "\n";
            }
        }
        catch (const runtime_error &e)
        {
            cout << "Error in listenerPacketThread: " << e.what() << "\n";
        }
    }
}

void Socket::start()
{
    isListening = true;
    listener = thread(&Socket::listenerPacketThread, this);
    cleaner = thread(&Socket::cleanerPacketThread, this);
}

void Socket::stop()
{
    isListening = false;
    listener.join();
    cleaner.join();
}

Message Socket::listen(MessageFilter *filter, int timeout)
{
    Message *message;
    auto start = std::chrono::steady_clock::now();
    auto limit = (timeout > 0) ? start + std::chrono::seconds(timeout) : start;

    while (true)
    {
        try
        {
            if (filter == nullptr)
            {
                std::lock_guard<std::mutex> lock(bufferMutex);
                if (!packetBuffer.empty())
                {
                    message = &packetBuffer.front();
                    packetBuffer.erase(packetBuffer.begin());
                    return *message;
                }
            }
            else
            {
                std::lock_guard<std::mutex> lock(bufferMutex);
                for (auto it = packetBuffer.begin(); it != packetBuffer.end(); ++it)
                {
                    if (filter->validate(*it))
                    {
                        packetBuffer.erase(it);
                        return *it;
                    }
                }
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in listen: " << e.what() << "\n";
        }

        if (timeout > 0 && std::chrono::steady_clock::now() > limit)
        {
            throw TimeoutException("Operation timeout");
        }
    }
}

void Socket::sendSegment(Segment segment, string ip, uint16_t port)
{
    segment = updateChecksum(segment);
    uint8_t *sending = new uint8_t[segment.payloadSize + 20];
    serializeSegment(segment, sending);
    struct sockaddr_in destaddr;

    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(port);
    destaddr.sin_addr.s_addr = inet_addr(ip.c_str());

    sendto(socket, sending, segment.payloadSize + 20, MSG_CONFIRM, (const struct sockaddr *)&destaddr, sizeof(destaddr));
    cout << "SENDED" << endl;
}

void Socket::close()
{
    ::close(socket);
    stop();
}
