// Client side implementation of UDP client-server model
#include <bits/stdc++.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include "segment/segment.hpp"

#define PORT 8080
#define MAXLINE 1500

// Driver code
int main()
{
	int sockfd;
	char buffer[MAXLINE];
	Segment message = syn(334455);

    uint8_t data[] = "Lorem ipsum dolor sit amet, consectetur adipiscing elit.";
	uint32_t sizeData = sizeof(data) - 1;

	message.payload = data;
	message.payloadSize = sizeData;

	message = updateChecksum(message);
	struct sockaddr_in servaddr;

	// Creating socket file descriptor
	if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
	{
		perror("socket creation failed");
		exit(EXIT_FAILURE);
	}

	memset(&servaddr, 0, sizeof(servaddr));

	// Filling server information
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(PORT);
	servaddr.sin_addr.s_addr = inet_addr("172.31.90.136");

	int n;
	socklen_t len;

	uint8_t *sending = new uint8_t[message.payloadSize + 20];
	serializeSegment(message, sending);

	sendto(sockfd, sending, message.payloadSize + 20,
		   MSG_CONFIRM, (const struct sockaddr *)&servaddr,
		   sizeof(servaddr));
	std::cout << "Sending message sent." << std::endl;

	n = recvfrom(sockfd, (char *)buffer, MAXLINE,
				 MSG_WAITALL, (struct sockaddr *)&servaddr,
				 &len);
	buffer[n] = '\0';
	std::cout << "Server : " << buffer << std::endl;

	close(sockfd);
	return 0;
}
