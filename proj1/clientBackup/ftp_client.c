/* Samantha Rack
 * CSE 30264
 * Project 1
 * ftp_client.c
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>

#include "client_lib.h"
#include "ftp_client_lib.h"

#define DEBUG 1

int main (int argc, char **argv) {

	//command line parsing	
	char *host;
	int port;	
	if (argc < 2) {
		printf("Usage: %s server_name [port]\n", argv[0]);
		return -1;
	}
	host = argv[1];
	if (argc == 2) port = 9499;
	else port = atoi(argv[2]);

	//socket creation
	int sockfd = getSocket();
	if (sockfd < 0) return -1;

	//fill in server address structure
	struct sockaddr_in serverInfo;
	if (getServerAddr(host, port, &serverInfo) < 0) return -1;

	//try to connect to the server
	#if DEBUG
		printf("Waiting to connect with server...\n");
	#endif
	if (connect(sockfd, (struct sockaddr*)&serverInfo, sizeof(struct sockaddr)) < 0) {
		perror("Error: connect\n");
		return -1;
	}
	#if DEBUG
		printf("Connected!\n");
	#endif

	int quit = 0;
	while (!quit) {
		//get new request from the client
		char request[100];	//plenty of space in the buffer if the user decides to type a very invalid operation
		printf("Enter an operation (get, put, dir, or xit): ");
		scanf("%s", request);

		//if-else structure for each operation
		if (strcmp(request, "get") == 0) {
			//implement get
			if (getRequest(sockfd) < 0) return -1;
		}
		else if (strcmp(request, "put") == 0) {
			//implement put
			if (putRequest(sockfd) < 0) return -1;
		}
		else if (strcmp(request, "dir") == 0) {
			//implement dir
			if (dirRequest(sockfd) < 0) return -1;
		}
		else if (strcmp(request, "xit") == 0) {		//DONE
			//send this request to the server
			if (send(sockfd, request, strlen(request), 0) < 0) {
				perror("Error: send\n");
				return -1;
			}		
			quit = 1;
			printf("Session has been closed.\n");
		}
		else {	//don't send unless it is one of the above instructions
			//unrecognized operation
			#if DEBUG
				printf("Operation unknown.\n");
			#endif
			continue;	//ask for new operation
		}
	}
	
	//close our socket to clean up
	if(close(sockfd) < 0) {
		perror("Error: close\n");
		return -1;
	}

}

