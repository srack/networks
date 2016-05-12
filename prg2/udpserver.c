/* CSE 30264
 * Samantha Rack
 * udpserver.c
 *  This program defines a UDP server on a port specified using a command line argument. 
 * The server is in a loop, waiting to receive data from one client at a time.  When it
 * receives data, it reverses the data it received, adds a timestamp, and sends the 
 * string back. 
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <time.h>

//#define DEBUG
#define BUFFERSIZE 2048 

int sendAll(int socket, char *msg, int msgLength, int flags, struct sockaddr *sa);
void adjustMsg(char msg[], int msgLength);
void reverseMsg(char msg[], int msgLength);
void swapChar(char *a, char *b);

int main (int argc, char **argv) {
	//verify that correct number of command line arguments
	if (argc != 2) {
		printf("Usage: %s port\n", argv[0]);
		return 1;
	}

	//structs holding information about server and client
	struct sockaddr_storage serverInfo, clientInfo; 
	
	int sockfd;
	//get socket descriptor
	if ((sockfd = socket(PF_INET, SOCK_DGRAM, 0)) < 0) {
		printf("Error: socket\n");
		return 1;
	}

	struct addrinfo hints, *results;
	
	bzero((char *)&hints, sizeof(hints));	//zero memory of hints struct
	hints.ai_family = AF_UNSPEC;	//ipv4 or ipv6
	hints.ai_socktype = SOCK_DGRAM;	//udp socket
	hints.ai_flags = AI_PASSIVE;	//os chooses my ip address for me

	//get pointer to addrinfo linked list
	int errStatus;
	if ((errStatus = getaddrinfo(NULL, argv[1], &hints, &results)) < 0) {
		printf("Error: getaddrinfo\n");
		return 1;
	}

	serverInfo = *((struct sockaddr_storage*)(results->ai_addr));
	
	freeaddrinfo(results);
	
	//allow the socket to be reused if the program fails after bind
	int option = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, (char *)&option, sizeof(int));

	int serverInfoSize = sizeof(serverInfo); 
	
	//bind to port specified
	if((errStatus = bind(sockfd, (struct sockaddr *)&serverInfo, sizeof(struct sockaddr))) < 0) {
		printf("Error: bind\n");
		return 1;
	}

	//infinite loop for listening to client requests
	while (1) 
	{
		//pointer to this size passed into recvfrom, reset it for each client 
		// (it will not be too small for clientInfoSize if going from ipv4 to v6)
		int clientInfoSize = sizeof(struct sockaddr_storage);

		#ifdef DEBUG
			printf("Waiting for next message on port %s...\n", argv[1]);
		#endif
	
		//intilize buffer space
		char buffer[BUFFERSIZE];
		int bytesRcvd;
		//subtract 1 from buffer size for adding the null terminator, and ~40 characters for the timestamp
		if ((bytesRcvd = recvfrom(sockfd, buffer, BUFFERSIZE-(1 + 40), 0, (struct sockaddr *)&clientInfo, &clientInfoSize)) < 0) {
			printf("Error: recvfrom\n");
			return 1;
		}
		//null terminate the string received to print it
		buffer[bytesRcvd] = '\0';
		#ifdef DEBUG
			printf("\nRECEIVED: %s\n", buffer);
		#endif

		adjustMsg(buffer, strlen(buffer));	//note: strlen does not include null terminator
		
		//adds null terminator back to the end of the buffer -- will be at location strlen(buffer), there will be enough space 
		// in the buffer because of how recvfrom is called above		
		buffer[strlen(buffer)] = '\0';
		#ifdef DEBUG
			printf("\nREVERSED: %s\n", buffer);
		#endif

		int sentBytes;
		if ((sentBytes = sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&clientInfo, sizeof(struct sockaddr))) < 0) {
			printf("Error: sendto\n");
			return 1;
		}
	}

}

//uses sendto to send all bytes of a message, even if they all do not fit in one package
int sendAll(int socket, char *msg, int msgLength, int flags, struct sockaddr *sa) {
	int totalSent = 0;
	int sentToReturn = 0;
	while (totalSent < msgLength) {
		sentToReturn = sendto(socket, msg, msgLength, flags, sa, sizeof(*sa));
		if(sentToReturn < 0) return sentToReturn;
		totalSent += sentToReturn;
	}
	return totalSent;
}

//adjusts the message to reverse the order of the characters, add a space, then add the timestamp
void adjustMsg(char msg[], int msgLength) {
	reverseMsg(msg, msgLength);
	
	//get timestamp
	struct timeval tv;
	gettimeofday(&tv, NULL);	//second parameter is time zone
	//remainder when divide out seconds taken by previous days since epoch
	// number of seconds elapsed since the day started
	int sToday = (tv.tv_sec % (60 * 60 * 24));	
	int hr = sToday / 3600;
	//number of seconds elapsed since the hour started
	int sHour = (sToday % 3600);
	int min = sHour / 60;
	int sec = sHour % 60;
	int usec = tv.tv_usec;

	//hr is in utc time, so subtract 4 to get our timezone
	hr = ((hr-4 >= 0) ? (hr - 4) : (hr - 4 + 24));
	
	char timeData[50];
	int n = sprintf(timeData, " Timestamp: %02d:%02d:%02d.%06d", hr, min, sec, usec);
	timeData[n] = '\0';
	strcat(msg, timeData);
	//add null terminator to end
	msg[strlen(msg)] = '\0';
}

//reverses the order of the characters stored in the array 
void reverseMsg(char msg[], int msgLength) {
	int i = 0;
	for ( ; i < msgLength/2; ++i) {
		swapChar(&msg[i], &msg[msgLength-1-i]);
	}

}

//swaps the memory location of two characters
void swapChar(char *a, char *b) {
	char temp = *a;
	*a = *b;
	*b = temp; 	
}




