/* Samantha Rack
 * CSE 30264
 * Project 1
 * ftp_server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>

#include "server_lib.h"
#include "ftp_server_lib.h"

#define DEBUG 1
#define BACKLOG 10

int main (int argc, char **argv) {
	char *port;
	if (argc == 2) {
		port = argv[1];
	} else port = "9499";
	
	struct sockaddr_in servInfo;
	struct sockaddr_storage clientInfo;
	//call server_lib function to get a binded socket and to fill in servInfo
	int sockl = getBindedSocket(port, (struct sockaddr*)&servInfo);
	if(sockl < 0) return -1;
	
	//listen on the port
	if (listen(sockl, BACKLOG) < 0) {
		perror("Error: listen\n");
		return -1;
	}

	int sockc;

	while (1) {
		int cliInfSize = sizeof(struct sockaddr_storage);	//reset for each new client

		#if DEBUG
			printf("Waiting for connection...\n");
		#endif
	
		//accept a new client
		if ((sockc = accept(sockl, (struct sockaddr*)&clientInfo, &cliInfSize)) < 0) {
			perror("Error: accept\n");
			return -1;
		}

		#if DEBUG
			printf("Connected!\n");
		#endif

		int clientQuit = 0;
		while (!clientQuit) {
			char req[10] = {'\0'};
			if (readE(sockc, req, sizeof(req)) < 0) return -1;
			#if DEBUG
				printf("REQ RECEIVED: %s\n", req);
			#endif

			if (strcmp(req, "get") == 0) {
				if (getRequest(sockc) < 0) return -1;
			} else if (strcmp(req, "put") == 0) {
				if (putRequest(sockc) < 0) return -1;
			} else if (strcmp(req, "dir") == 0) {
				if (dirRequest(sockc) < 0) return -1;
			} else if (strcmp(req, "xit") == 0) {
				clientQuit = 1;
			}
		}

		if (close(sockc) < 0) {
			perror("Error: close\n");
			return -1;
		}
	} 
	
	if (close(sockl) < 0) {
		perror("Error: close\n");
		return -1;
	}

	return 0;
}
