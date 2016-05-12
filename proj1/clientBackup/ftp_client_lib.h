/* Samantha Rack
 * CSE 30264
 * Project 1
 * ftp_client_lib.h
 */
#ifndef FTP_CLIENT_LIB_H
#define FTP_CLIENT_LIB_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <mhash.h>

#include "client_lib.h"

#define DEBUG 1
#define MAXBUFFERSIZE 1024

double getSecSinceEpoch();

int getRequest(int sockfd) {
	//write the initial request to the server	
	if (sendE(sockfd, "get", strlen("get"), 0) < 0) return -1;
	
	//what file do you want?
	char fileName[100] = {'\0'};	//max fileName size = 100 characters
	printf("\tEnter name of file to get: ");
	scanf("%s", fileName);

	//send length of file name as short -- convert to network byte order 
	unsigned short lengthFileName = htons(strlen(fileName));	
	if (sendE(sockfd, &lengthFileName, sizeof(lengthFileName), 0) < 0) return -1;

	//send file name to server
	if (sendE(sockfd, fileName, strlen(fileName), 0) < 0) return -1;

	//receive file size
	int fileSize;
	if(readE(sockfd, &fileSize, sizeof(fileSize)) < 0) return -1;
	fileSize = (int)ntohl(fileSize);	//convert to host byte order
	#if DEBUG
		printf("\tfile size: %d\n", (int)fileSize);
	#endif
	//check if server sent -1 (ie. file does not exist)
	if (fileSize == -1) {
		//file does not exist, so tell user and return
		printf("File does not exist on the server.\n");
		return 0;
	}

	//initialize hash 
	char clientHash[16];
	MHASH mh;
	mh = mhash_init(MHASH_MD5);
	if(mh == MHASH_FAILED) {
		perror("Error: hash\n");
		return -1;
	}

	//open the file for writing
	FILE *f = fopen(fileName, "w");
	if(f == NULL) {
		perror("Error: opening get file.\n");
		return -1;
	}
	int readSoFar = 0;
	//enter loop for reading btyes and saving to file
	while (readSoFar < fileSize) {
		//read up to 1024 bytes at a time	
		int bytesReceiving = (fileSize-readSoFar > MAXBUFFERSIZE ? MAXBUFFERSIZE : fileSize-readSoFar);
		char buffer[bytesReceiving];
		int bytesRead = readE(sockfd, buffer, sizeof(buffer));
		if (bytesRead < 0) return -1;
		readSoFar += bytesRead;
		//write the bytes to the file
		if (fwrite(buffer, sizeof(char), bytesRead, f) < 0) {
			perror("Error: fwrite\n");
			return -1;
		}

		//update hash to reflect these bytes
		mhash(mh, buffer, sizeof(buffer));	

		#if DEBUG
			printf("\tWrote %d bytes to %s\n", bytesRead, fileName);
		#endif
	}
	//get timestamp immediately after reading all of file
	double cTime = getSecSinceEpoch();

	fclose(f);

	//put hash data into clientHash array
	mhash_deinit(mh, clientHash);
	#if DEBUG
		int j;
		printf("\tClient hash: ");
		for (j = 0; j < 16; ++j) {
			printf("%hhx", clientHash[j]);
		}
		printf("\n");
	#endif

	//receive md5hash
	char serverHash[16];
	if (readE(sockfd, serverHash, sizeof(serverHash)) < 0) return -1;
	#if DEBUG
		printf("\tServer hash: ");
		for (j = 0; j < 16; ++j) {
			printf("%hhx", serverHash[j]);
		}
		printf("\n");
	#endif
	
	//read the timestamp for throughput calculation
	char serverTime[64] = {'\0'};
	if (readE(sockfd, serverTime, sizeof(serverTime)-1) < 0) return -1;
	double sTime;
	sscanf(serverTime, "%lf", &sTime);
	#if DEBUG
		printf("\tclient time: %f\n\tserver time: %f\n", cTime, sTime);
	#endif


	//compare the hashes
	if (memcmp(clientHash, serverHash, sizeof(clientHash)) == 0) {
		//compute throughput
		double changeInTime = cTime - sTime;
		double throughput = (fileSize/changeInTime)/1000000;
		//display throughput
		printf("\t%d bytes transferred in %.4f seconds: %.4f Megabytes/sec\n", fileSize, changeInTime, throughput);
		//display hash data
		printf("\tFile MD5sum: ");
		int i;
		for (i = 0; i < 16; ++i) {
			printf("%hhx", serverHash[i]);
		}
		printf("\n");
		
	} else {
		//hashes did not equal, data was messed up
		printf("Transfer unsuccessful\n");
		//remove file we wrote
		if (remove(fileName) < 0) {
			perror("Error: unsuccessful transfer clean up\n");
			return -1;
		}
		#if DEBUG
			printf("%s deleted due to unsuccessful transfer\n", fileName);
		#endif
	}
	return 0;
}



int putRequest(int sockfd) {
	//write the initial request to the server	
	if (sendE(sockfd, "put", strlen("put"), 0) < 0) return -1;
	
	//what file do you want?
	char fileName[100] = {'\0'};	//max fileName size = 100 characters
	printf("\tEnter name of file to put: ");
	scanf("%s", fileName);

	FILE *f = fopen(fileName, "r");
	while (f == NULL) {
		printf("\tFile does not exist. Enter name of file to put: ");
		scanf("%s", fileName);
		f = fopen(fileName, "r");
	}

	//send length of file name as short -- convert to network byte order 
	unsigned short lengthFileName = htons(strlen(fileName));	
	if (sendE(sockfd, &lengthFileName, sizeof(lengthFileName), 0) < 0) return -1;

	//send file name to server
	if (sendE(sockfd, fileName, strlen(fileName), 0) < 0) return -1;

	//wait for server's ACK
	char awkBuffer[4] = {'\0'};	
	if (readE(sockfd, awkBuffer, sizeof(awkBuffer)) < 0) return -1;
	if (strcmp(awkBuffer, "ACK") != 0) {
		//received unexpected message back from server, so return
		printf("Received unexpected message from server.\n");		
		return -1;
	}
	#if DEBUG
		printf("\treceived ACK\n");
	#endif

	//determine and send size of the file
	int fileSize;
	struct stat st;
	stat(fileName, &st);
	fileSize = st.st_size;
	#if DEBUG
		printf("\tfile size: %d\n", fileSize);
	#endif
	int fileSize_n = htonl(fileSize);	//fileSize value in network byte order

	if (sendE(sockfd, &fileSize_n, sizeof(fileSize_n), 0) < 0) return -1;

	//intitialize hash
	char clientHash[16];
	MHASH mh;
	mh = mhash_init(MHASH_MD5);
	if(mh == MHASH_FAILED) {
		perror("Error: hash\n");
		return -1;
	}

	//write the file to the server in chunks of MAXBUFFERSIZE
	//file already open
	int sentSoFar = 0;

	//get time stamp immediately before entering write loop
	double cTime = getSecSinceEpoch();
	//enter loop for reading from file and sending to server
	while (sentSoFar < fileSize) {
		//determine how many bytes to send this time through
		int bytesSending = (fileSize-sentSoFar > MAXBUFFERSIZE ? MAXBUFFERSIZE : fileSize-sentSoFar);
		char buffer[bytesSending];
		//read <bytesSending> bytes from the file
		if (fread(buffer, sizeof(char), sizeof(buffer), f) < 0) {
			perror("Error: fread\n");
			return -1;
		}
		
		//update hash to reflect these bytes
		mhash(mh, buffer, sizeof(buffer));
	
		//send these bytes to the client
		int bytesSent = sendE(sockfd, buffer, sizeof(buffer), 0);
		if (bytesSent < 0) return -1;
		sentSoFar += bytesSent;		
	}
	fclose(f);	

	//put hash data in clientHash array
	mhash_deinit(mh, clientHash);
	#if DEBUG
		int j;
		printf("\tHash: ");
		for (j = 0; j < 16; ++j) {
			printf("%hhx", clientHash[j]);
		}
		printf("\n");
	#endif	

	//send md5 hash -- 16 bits
	if (sendE(sockfd, clientHash, sizeof(clientHash), 0) < 0) return -1;

	//send timestamp
	char clientTime[64];
	sprintf(clientTime, "%f", cTime);
	#if DEBUG
		printf("\tclient time: %s\n", clientTime);
	#endif
	if (sendE(sockfd, clientTime, strlen(clientTime), 0) < 0) return -1;

	//receive either: throughput results or transfer failure notice
	// if throughput results, tell user transfer successful and give throughput results
	// else tell user transfer failed
	char throughput[248];
	int bytesRead;
	if ((bytesRead = readE(sockfd, throughput, sizeof(throughput)-1)) < 0) return -1;
	throughput[bytesRead] = '\0';
	
	if (strcmp(throughput, "-1") == 0) {
		printf("Transfer unsuccessful.\n");
	} else {
		//print throughput data
		printf("\t%s\n", throughput);
		//print hash
		printf("\tFile MD5Sum: ");
		int i;
		for (i = 0; i < 16; ++i) {
			printf("%hhx", clientHash[i]);
		}
		printf("\n");
	}
	
	return 0;
}

int dirRequest(int sockfd) {
	//write the initial request to the server	
	if (sendE(sockfd, "dir", strlen("dir"), 0) < 0) return -1;

	//receive size of the listing
	int sizeList_n;
	if (readE(sockfd, &sizeList_n, sizeof(sizeList_n)) < 0) return -1;
	int sizeList = ntohl(sizeList_n);
	#if DEBUG
		printf("\tlisting size: %d\n", sizeList);
	#endif

	char buffer[sizeList + 1];	

	int bytesRead = readBytes(sockfd, buffer, sizeof(buffer), sizeList);
	buffer[bytesRead] = '\0'; 
	printf("%s", buffer);
	return 0;
}

//returns the current time measured in seconds since Unix epoch
double getSecSinceEpoch() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec + (tv.tv_usec/1000000.0));
}

#endif
