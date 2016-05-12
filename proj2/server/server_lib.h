/* Adrian Gerbaud and Samantha Rack
 * CSE 30264
 * Project 2
 * server_lib.h
 */

#ifndef SERVER_LIB_H
#define SERVER_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

/* Function Name: getBindedSocket
 * Returns descriptor for server's socket, binded to the specified port number.  The
 * sockaddr structure is updated to reflect the server's information
 */
int getBindedSocket(char* port, struct sockaddr *serverInfo) {
	//create the TCP socket
	int sockfd = socket(PF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {
		perror("Error: socket creation\n");
		return sockfd;
	}
	
	//fill in the addrinfo structure
	struct addrinfo hints, *results;
	bzero((char *)&hints, sizeof(hints));
	hints.ai_family = AF_UNSPEC;		//ipv4 or v6
	hints.ai_socktype = SOCK_STREAM;	//tcp socket
	hints.ai_flags = AI_PASSIVE;	//os chooses my ip address
	
	int errStatus;
	if ((errStatus = getaddrinfo(NULL, port, &hints, &results)) < 0) {
		perror("Error: getaddrinfo\n");
		return errStatus;
	}

	serverInfo = results->ai_addr;

	freeaddrinfo(results);

	//set socket options to allow the port to be reused when the program crashes
	int opt = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&opt, sizeof(int));
	
	//bind the socket to the server's address and port number
	if ((errStatus = bind(sockfd, serverInfo, sizeof(*serverInfo))) < 0) {
		perror("Error: bind\n");
		return -1;
	}

	//on success, return the file descriptor for the binded socket
	return sockfd;
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

//Uses sendto to send all bytes of a message, even if they all do not fit in one package
int sendAll(int socket, char *msg, int msgSize, int flags) {
	int totalSent = 0;
	int msgLength = strlen(msg);
	int sentSoFar = 0;
	while (totalSent < msgLength) {
		sentSoFar = send(socket, msg, msgSize, flags);
		if(sentSoFar < 0) return sentSoFar;
		totalSent += sentSoFar;
	}
	return totalSent;
}

#endif
