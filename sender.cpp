#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BROADCAST_PORT 8080 // Port to send the broadcast message
#define BROADCAST_MESSAGE "Hello, this is a broadcast message!"
#define BROADCAST_ADDRESS "172.31.95.255" // Broadcast address for your subnet

int main()
{
    int sockfd;
    struct sockaddr_in broadcastAddr;
    const char *message = BROADCAST_MESSAGE;

    // Create a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0)
    {
        perror("Socket creation failed");
        return 1;
    }

    // Enable broadcast option
    int broadcastEnable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &broadcastEnable, sizeof(broadcastEnable)) < 0)
    {
        perror("Setting broadcast option failed");
        close(sockfd);
        return 1;
    }

    // Set up the broadcast address
    memset(&broadcastAddr, 0, sizeof(broadcastAddr));
    broadcastAddr.sin_family = AF_INET;
    broadcastAddr.sin_port = htons(BROADCAST_PORT);
    broadcastAddr.sin_addr.s_addr = inet_addr(BROADCAST_ADDRESS);

    // Send the broadcast message
    if (sendto(sockfd, message, strlen(message), 0,
               (struct sockaddr *)&broadcastAddr, sizeof(broadcastAddr)) < 0)
    {
        perror("Broadcast send failed");
        close(sockfd);
        return 1;
    }

    std::cout << "Broadcasting to: " << BROADCAST_ADDRESS << "\n";
    std::cout << "Broadcast message sent: " << message << std::endl;

    close(sockfd);
    return 0;
}