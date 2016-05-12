/* Adrian Gerbaud and Samantha Rack
 * CSE 30264
 * Project 2
 * client_lib.h
 */
#ifndef CLIENT_LIB_H
#define CLIENT_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* Function Name:  getSocket
 * Returns the file descriptor for a TCP socket, or -1 on failure.
 */
int getSocket() {
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if(sockfd < 0) {
		perror("Error: socket creation\n");
	}
	return sockfd;
}


/* Function Name:  getServerAddr
 * The host name and the port number are passed into this function.  The serverInfo
 * parameter is filled in by the function using getaddrinfo
 */
int getServerAddr(char *host, int port, struct sockaddr_in *serverInfo) {

	struct addrinfo hints, *results;
	bzero((char *)&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	if(getaddrinfo(host, NULL, &hints, &results) < 0) {
		perror("Error: getaddrinfo\n");
		return -1;
	}

	//fill in serverInfo structure
	serverInfo->sin_family = AF_INET;
	serverInfo->sin_port = htons(port);

	serverInfo->sin_addr.s_addr = ((struct sockaddr_in*)(results->ai_addr))->sin_addr.s_addr;	

	//free memory allocated for the linked list of addrinfo structs
	freeaddrinfo(results);
	
	return 0;

}	

//read with error message included
int readE(int sockfd, void *msg, int msgSize) {
	int bytesRead = recv(sockfd, msg, msgSize, 0);
	if (bytesRead < 0) perror("Error: recv\n");
	return bytesRead;
}

//send with error message included
int sendE(int sockfd, void *msg, int msgSize, int flags) {
	int sBytes = send(sockfd, msg, msgSize, flags);
	if (sBytes < 0) perror("Error: send\n");
	return sBytes;
}


/* buffer parameter is updated to contain bytesToRead bytes of data read
 * from the server, this function blocks until that number of bytes is read
 * precondition: bytesToRead <= bufferSize
 * this function does not null terminate the data
 */
int readBytes(int sockfd, char *buffer, int bufferSize, int bytesToRead) {
	//ensure there is enough space in the buffer to hold <bytesToRead> bytes
	if (bytesToRead > bufferSize) return -1;
	int totalRead = 0;
	while (totalRead < bytesToRead) {
		//blocks until next message is received
		int bRead = recv(sockfd, buffer+totalRead, bufferSize-totalRead, 0);
		//error check return value of read
		if (bRead < 0) {
			perror("Error: recv\n");
			return bRead;
		}
		totalRead += bRead;
	}
	return totalRead;	//totalRead should equal bytesToRead parameter
}

#endif
