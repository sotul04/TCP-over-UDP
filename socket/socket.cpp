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
    struct sockaddr_in this_addr;
    memset(&this_addr, 0, sizeof(this_addr));
    this_addr.sin_family = AF_INET;
    this_addr.sin_addr.s_addr = inet_addr(ip.c_str());
    this_addr.sin_port = htons(port);

    // const char *message = "Hello, this is a broadcast message!";
    // sendto(socket, message, strlen(message), 0, (struct sockaddr *)&this_addr, sizeof(this_addr));

    // cout << "Sending first message" << endl;


    if (bind(socket, (const struct sockaddr *)&this_addr, sizeof(this_addr)) < 0)
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
    if (socket >= 0)
    {
        ::close(socket); // Ensure socket is closed properly
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
    cout << "CLEANING" << endl;
    while (isListening)
    {
        try
        {
            std::unique_lock<std::mutex> lock(bufferMutex);

            if (!packetBuffer.empty())
            {
                // Make a copy of the first message
                Message firstMessage = packetBuffer.front();

                // Wait for specified time while allowing other threads to access buffer
                lock.unlock();
                std::this_thread::sleep_for(std::chrono::milliseconds(static_cast<int>(cleanerTime * 1000)));
                lock.lock();

                // Check if the message is still there and matches
                if (!packetBuffer.empty() && packetBuffer.front() == firstMessage)
                {
                    cout << "Erasing packet" << endl;
                    packetBuffer.erase(packetBuffer.begin());
                }

                // Adjust cleaner time
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
    cout << "Listening" << endl;

    while (isListening)
    {
        try
        {
            std::vector<uint8_t> buffer(MAXLINE);
            struct sockaddr_in cliaddr;
            socklen_t len = sizeof(cliaddr);

            int n = recvfrom(socket, buffer.data(), MAXLINE, 0, (struct sockaddr *)&cliaddr, &len);
            if (n <= 0)
            {
                if (!isListening)
                    break; // Clean exit if we're stopping
                continue;  // Skip this iteration if error
            }

            Segment received = deserializeSegment(buffer.data(), n);

            if (!isValidChecksum(received))
            {
                continue; // Skip invalid packets
            }

            Message newMessage(inet_ntoa(cliaddr.sin_addr), ntohs(cliaddr.sin_port), received);

            {
                cout << "Get new segment" << endl;
                std::lock_guard<std::mutex> lock(bufferMutex);
                packetBuffer.push_back(std::move(newMessage));
                bufferCV.notify_one(); // Notify waiting threads
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

    // Close socket to interrupt recvfrom
    if (socket >= 0)
    {
        ::close(socket);
        socket = -1;
    }

    // Wait for threads to finish
    if (listener.joinable())
        listener.join();
    if (cleaner.joinable())
        cleaner.join();

    // Clear the buffer
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

            // Wait for data with timeout
            auto waitResult = bufferCV.wait_for(lock,
                                                std::chrono::milliseconds(100),
                                                [this]()
                                                { return !packetBuffer.empty(); });

            if (waitResult) // If we have data
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

            // Check timeout
            if (timeout > 0 && std::chrono::steady_clock::now() > limit)
            {
                cout << "Timeout" << endl;
                throw TimeoutException("Operation timeout");
            }
        }
        catch (const TimeoutException & te)
        {
            cout << "Throwing Timeout" << endl;
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

    // Use a vector to handle memory more safely
    std::vector<uint8_t> sending(segment.payloadSize + 20);
    serializeSegment(segment, sending.data());

    struct sockaddr_in destaddr;
    destaddr.sin_family = AF_INET;
    destaddr.sin_port = htons(port);
    destaddr.sin_addr.s_addr = inet_addr(ip.c_str());

    if (sendto(socket, sending.data(), sending.size(), 0, (const struct sockaddr *)&destaddr, sizeof(destaddr)) <= 0)
    {
        perror("Send failed");
    }
    else
    {
        cout << "SENDED" << endl;
    }
}

void Socket::close()
{
    if (socket >= 0)
    {
        ::close(socket);
        socket = -1; // Mark the socket as closed
    }
    stop();
}
