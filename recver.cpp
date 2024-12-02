#include <iostream>
#include <cstring>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

#define BROADCAST_PORT 8080  // Port to listen for broadcast messages
#define MAXLINE 1024        // Maximum size of the received message buffer
#define RECEIVER_IP "172.31.90.136"  // Replace with your specific IP address

int main() {
    int sockfd;
    struct sockaddr_in servaddr, clientAddr;
    char buffer[MAXLINE];
    socklen_t addrLen = sizeof(clientAddr);

    // Create a socket
    sockfd = socket(AF_INET, SOCK_DGRAM, 0);
    if (sockfd < 0) {
        perror("Socket creation failed");
        return 1;
    }

    // Enable broadcast option
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_BROADCAST, &enable, sizeof(enable)) < 0) {
        perror("Failed to set broadcast option");
        return 1;
    }

    // Allow reuse of address and port
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        perror("Failed to set reuse address option");
        return 1;
    }

    // Bind the socket to the specific IP address
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(BROADCAST_PORT);
    servaddr.sin_addr.s_addr = inet_addr(RECEIVER_IP);
    // if (inet_pton(AF_INET, RECEIVER_IP, &servaddr.sin_addr) <= 0) {
    //     perror("Invalid address");
    //     close(sockfd);
    //     return 1;
    // }

    if (bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr)) < 0) {
        perror("Bind failed");
        close(sockfd);
        return 1;
    }

    std::cout << "Listening for broadcast messages on " << RECEIVER_IP 
              << ":" << BROADCAST_PORT << "..." << std::endl;

    // Loop to receive broadcast messages
    while (true) {
        int n = recvfrom(sockfd, buffer, MAXLINE, 0, (struct sockaddr*)&clientAddr, &addrLen);
        if (n < 0) {
            perror("Receive failed");
            break;
        }
        buffer[n] = '\0';  // Null-terminate the received message
        std::cout << "Received broadcast message from "
                  << inet_ntoa(clientAddr.sin_addr) << ": " << buffer << std::endl;
    }

    close(sockfd);
    return 0;
}