/* CSE 30264
 * Samantha Rack
 * Program 1 - Simple UDP Client
 * Usage: ./udpclient host port <msg or txt file>
 * This program implements a UDP client that sends a message to a server, then receives 
 * data back from the server.  The program determines if the 3rd argument to the function
 * is a filename containing the message, or a message in itself.
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>

//#define DEBUG
//#define CHECKENDIANOFSERVER

char *parseMsgInput(char *cmdArg);
int sendAll(int socket, char *msg, int flags, struct sockaddr *sa);

int main (int argc, char **argv) {
	//verify that there are the correct number of command line arguments
	if (argc != 4) {
		printf("Usage: %s host port message\n", argv[0]);
		return 1;
	}

	int socketfd;
	struct addrinfo hints, *results, *p;
	struct sockaddr_in *sockin;	//can hold ipv4

	//create a socket
	socketfd = socket(PF_INET, SOCK_DGRAM, 0);	//returns file descriptor
	//error checking
	if (socketfd < 0) {
		printf("Error: socket\n");
		return(1);
	}

	/*bzero((char *)&sockin, sizeof(struct sockaddr_in));
	sockin.sin_family = AF_INET;
	sockin.sin_port = htons(atoi(argv[2]));
	inet_pton(sockin.sin_family, argv[1], &(sockin.sin_addr));
	*/
	//initialize the hints structure to zeros
	bzero((char *)&hints, sizeof(hints));
	//then fill it in so getaddrinfo can use it
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_DGRAM;

	//get linked list to options for server
	int errStatus;
	if ((errStatus = getaddrinfo(argv[1], argv[2], &hints, &results)) != 0) {
		printf("Error getaddrinfo: %s\n", gai_strerror(errStatus));
		return 1;
	}
	
	//get structure for server to whom we will send the message
	for (p = results; p != NULL; p->ai_next) {
		if(p->ai_family == AF_INET) {	//ipv4
			sockin = (struct sockaddr_in*)p->ai_addr;
			break;
		}
		//ignore any ipv6
		else {
			#ifdef DEBUG
				printf("Found ipv6\n");
			#endif
		}
	}
	freeaddrinfo(results);	//free the memory allocated by getaddrinfo
	//check that a server was found
	if (p == NULL) {
		printf("Unable to find server.\n");
		return 1;
	}

	#ifdef DEBUG
		char ipv4_readable[100];
		inet_ntop(sockin->sin_family, &(sockin->sin_addr), ipv4_readable, INET_ADDRSTRLEN);
		printf("Found server at %s\n", ipv4_readable);
	#endif

	#ifdef CHECKENDIANOFSERVER
		//change the port so it is specified as little-endian, see if the server can be sent to
		sockin->sin_port = ntohs(atoi(argv[2]));
	#endif	
	
	//determine if the msg argument of ./udpclient is a filename or the message to send
	char *message = parseMsgInput(argv[3]); 

	int sentBytes;
	if ((sentBytes = sendAll(socketfd, message, 0, (struct sockaddr*)sockin)) < 0) {
		printf("Error: sendto %d\n", sentBytes);
		return 1;
	} 
	free(message);	//free the memory allocated in parseMsgInput
	message = NULL;

	int receivedBytes;
	char buffer[1024];
	int sockinSize = sizeof(struct sockaddr_in);
	
	if ((receivedBytes = recvfrom(socketfd, buffer, sizeof(buffer), 0, (struct sockaddr*)sockin, &sockinSize)) < 0) {
		printf("Error: recvfrom\n");
		return 1;
	}
	buffer[receivedBytes] = '\0';	//add a null terminator to the end of the msg
	printf("%s", buffer);	//print the received bytes

}

//determines if the message argument for ./udpclient is a file name or text to send
//if the arg is a file, the file is read and a string containing the data is returned
//if the arg is a string itself, then the argument is returned
//NOTE: buffer returned by this function is allocated using malloc and must be freed by the calling function after its use 
char *parseMsgInput(char *cmdArg) {
	int cmdLength = strlen(cmdArg);
	
	//allocate heap space for whatever char array we return
	char *buffer;

	//attempt to open a file based on whatever the cmd argument is	
	FILE *txtFile = fopen(cmdArg, "r");
	
	//if unable to find the file and open it, return the filename as the message to send
	if(txtFile == NULL) {
		buffer = malloc((strlen(cmdArg) + 1) * sizeof(char));
		strcpy(buffer, cmdArg);
	} else {
		int bufferSize = 500;
		buffer = malloc(bufferSize * sizeof(char));
		int charsRead = 0;
		int readThisIteration = 1;
	
		while(readThisIteration > 0) {
			//add charsRead to buffer (pointer arithmetic) so old information is not overwritten
			readThisIteration = fread(buffer + charsRead, sizeof(char), bufferSize-charsRead, txtFile);
			charsRead += readThisIteration;
			if(charsRead == bufferSize) {
				#ifdef DEBUG
					printf("reallocing. buffer = %s\n", buffer);
				#endif
				bufferSize += 100;
				buffer = realloc(buffer, bufferSize * sizeof(char));
			}
		}

		fclose(txtFile);
	}
	return buffer;
}

//Uses sendto to send all bytes of a message, even if they all do not fit in one package
int sendAll(int socket, char *msg, int flags, struct sockaddr *sa) {
	int totalSent = 0;
	int msgLength = strlen(msg);
	int sentToReturn = 0;
	while (totalSent < msgLength) {
		sentToReturn = sendto(socket, msg, strlen(msg), flags, sa, sizeof(*sa));
		if(sentToReturn < 0) return sentToReturn;
		totalSent += sentToReturn;
	}
	return totalSent;
}











