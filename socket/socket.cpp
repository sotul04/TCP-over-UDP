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

    segmentHandler = new SegmentHandler();
    status = CLOSED;

    cleanerTime = CLEANER_TIME;
    isListening = false;
}

Socket::~Socket()
{
    delete segmentHandler;
    packetBuffer.clear();
    if (socket >= 0)
    {
        ::close(socket);
    }
}

struct sockaddr_in Socket::generateAddress(string ip, uint16_t port)
{
    struct sockaddr_in new_addr;
    memset(&new_addr, 0, sizeof(new_addr));
    new_addr.sin_family = AF_INET;
    new_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    new_addr.sin_port = htons(port);
    return new_addr;
}

void Socket::bindAddress()
{
    struct sockaddr_in newaddress = generateAddress(this->ip, this->port);
    if (bind(socket, (const struct sockaddr *)&newaddress, sizeof(newaddress)) < 0)
    {
        perror("Bind Failed");
        exit(EXIT_FAILURE);
    }
}

void Socket::setBroadcast()
{
    int enable = 1;
    if (setsockopt(socket, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) < 0)
    {
        perror("Setting broadcast option failed");
        ::close(socket);
        exit(EXIT_FAILURE);
    }
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
            std::unique_lock<std::mutex> lock(bufferMutex);

            if (!packetBuffer.empty())
            {
                Message firstMessage = packetBuffer.front();

                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(cleanerTime * 1000)));
                lock.lock();

                if (!packetBuffer.empty() && packetBuffer.front() == firstMessage)
                {
                    packetBuffer.erase(packetBuffer.begin());
                }

                if (packetBuffer.size() > CLEANER_LIMIT)
                {
                    cleanerTime = MIN_CLEANER_TIME;
                }
                else
                {
                    cleanerTime = CLEANER_TIME;
                }
            }
            else
            {
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(cleanerTime * 1000)));
            }
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in cleanerPacketThread: " << e.what() << std::endl;
        }
    }
}

void Socket::listenerPacketThread()
{
    while (isListening)
    {
        try
        {
            uint8_t *buffer = new uint8_t[MAXLINE];
            struct sockaddr_in cliaddr;
            socklen_t len = sizeof(cliaddr);

            int n = recvfrom(socket, buffer, MAXLINE, MSG_WAITALL, (struct sockaddr *)&cliaddr, &len);
            if (n <= 0)
            {
                if (!isListening)
                    break;
                continue;
            }

            Segment received = deserializeSegment(buffer, n);

            if (!isValidChecksum(received))
            {
                continue;
            }

            Message newMessage(inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), received);

            {
                std::lock_guard<std::mutex> lock(bufferMutex);
                packetBuffer.push_back(std::move(newMessage));
                bufferCV.notify_one();
            }
        }
        catch (const std::exception &e)
        {
            if (isListening)
            {
                std::cerr << "Error in listenerPacketThread: " << e.what() << "\n";
            }
        }
    }
}

void Socket::start()
{
    isListening = true;
    listener = std::thread(&Socket::listenerPacketThread, this);
    cleaner = std::thread(&Socket::cleanerPacketThread, this);
}

void Socket::stop()
{
    isListening = false;

    listener.join();
    cleaner.join();

    listener.~thread();
    cleaner.~thread();

    std::lock_guard<std::mutex> lock(bufferMutex);
    packetBuffer.clear();
}

Message Socket::listen(MessageFilter *filter, int timeout)
{
    auto start = std::chrono::steady_clock::now();
    auto limit = (timeout > 0) ? start + std::chrono::seconds(timeout) : start;

    while (isListening)
    {
        try
        {
            std::unique_lock<std::mutex> lock(bufferMutex);

            auto waitResult = bufferCV.wait_for(lock,
                                                std::chrono::milliseconds(100),
                                                [this]()
                                                { return !packetBuffer.empty(); });

            if (waitResult)
            {
                if (filter == nullptr)
                {
                    if (!packetBuffer.empty())
                    {
                        Message message = packetBuffer.front();
                        packetBuffer.erase(packetBuffer.begin());
                        return message;
                    }
                }
                else
                {
                    for (size_t i = 0; i < packetBuffer.size(); ++i)
                    {
                        if (filter->validate(packetBuffer[i]))
                        {
                            Message message = packetBuffer[i];
                            packetBuffer.erase(packetBuffer.begin() + i);
                            return message;
                        }
                    }
                }
            }

            if (timeout > 0 && std::chrono::steady_clock::now() > limit)
            {
                throw TimeoutException("Operation timeout");
            }
        }
        catch (const TimeoutException &te)
        {
            throw TimeoutException("Timeout operation");
        }
        catch (const std::exception &e)
        {
            std::cerr << "Error in listen: " << e.what() << "\n";
        }
    }
    throw exception();
}

void Socket::sendSegment(Segment segment, string ip, uint16_t port)
{
    segment = updateChecksum(segment);

    uint8_t* sending = new uint8_t[segment.payloadSize + 20];
    serializeSegment(segment, sending);

    struct sockaddr_in destaddr = generateAddress(ip, port);

    if (sendto(socket, sending, segment.payloadSize + 20, MSG_CONFIRM, (const struct sockaddr *)&destaddr, sizeof(destaddr)) <= 0)
    {
        perror("Send failed");
    }
}

void Socket::close()
{
    stop();
        ::close(socket);
        socket = -1;
}

string Socket::logStatus()
{
    return "["+StatusMap.at(status)+"] ";
}

void Socket::setStatus(TCPStatusEnum stat)
{
    status = stat;
}