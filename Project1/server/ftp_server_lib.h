/* Samantha Rack
 * CSE 30264
 * Project 1
 * ftp_server_lib.h
 */

#ifndef FTP_SERVER_LIB
#define FTP_SERVER_LIB

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h>
#include <sys/stat.h>
#include <mhash.h>
#include <dirent.h>

#include "server_lib.h"

#define DEBUG 0
#define MAXBUFFERSIZE 1024

double getSecSinceEpoch();

/* Function Name: readRequest
 * called after a client is connected, to read its next requestcccccpp
 * request parameter will be update to be used in main
 * request will be null terminated by this function
 * returns the number of bytes read
 */
int readRequest(int sockfd, char *request, int msgSize) {
	int bytesRead;
	bytesRead = read(sockfd, request, msgSize-1);
	//error check
	if(bytesRead < 0) {
		perror("Error: read\n");
		return bytesRead;
	}
	//null terminate the string	
	request[bytesRead] = '\0';
	return strlen(request);
}


int getRequest(int sockfd) {
	//read a short indicating the filename's size
	unsigned short fileNameSize;
	if(readE(sockfd, &fileNameSize, sizeof(fileNameSize)) < 0) return -1;
	fileNameSize = ntohs(fileNameSize);	//convert back to host byte order
	#if DEBUG	
		printf("\tfilename size: %d\n", fileNameSize);
	#endif

	//read the filename from the client
	char fileName[fileNameSize + 1];
	if (readE(sockfd, fileName, sizeof(fileName)) < 0) return -1;
	fileName[fileNameSize] = '\0';
	#if DEBUG
		printf("\tfilename: %s\n", fileName);
	#endif

	//determine and send size of the desired file, if it exists
	int fileSize;
	FILE *f = fopen(fileName, "r");
	struct stat st;
	if (f != NULL && stat(fileName, &st) == 0) fileSize = st.st_size;
	else fileSize = -1;
	#if DEBUG
		printf("\tfile size: %d\n", fileSize);
	#endif
	int fileSize_n = htonl(fileSize);	//fileSize value in network byte order

	if (sendE(sockfd, &fileSize_n, sizeof(fileSize_n), 0) < 0) return -1;
	if (fileSize == -1) {
		#if DEBUG
			printf("\tFile not found.\n");
		#endif
		return 0;
	}	

	//initialize hash as 16 byte string
	char serverHash[16];
	MHASH mh;
	mh = mhash_init(MHASH_MD5);
	if(mh == MHASH_FAILED) {
		perror("Error: hash\n");
		return -1;
	}

	//read N bytes at a time from the file, then send these bytes to the client
	//file already open
	int sentSoFar = 0;
	
	//get time stamp immediately before entering write loop
	double sTime = getSecSinceEpoch();
	//enter loop for reading from file and sending to client
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
	//close the file that we sent
	fclose(f);

	//put hash data into serverHash array
	mhash_deinit(mh, serverHash);
	#if DEBUG
		int j;
		printf("\tHash: ");
		for (j = 0; j < 16; ++j) {
			printf("%hhx", serverHash[j]);
		}
		printf("\n");
	#endif

	//send calculated hash value
	if (sendE(sockfd, serverHash, sizeof(serverHash), 0) < 0) return -1; 
	
	//send timestamp
	char serverTime[64];
	sprintf(serverTime, "%f", sTime);
	#if DEBUG
		printf("\tserver time: %s\n", serverTime);
	#endif
	if (sendE(sockfd, serverTime, strlen(serverTime), 0) < 0) return -1;

	return 0;
}

int putRequest(int sockfd) {
	//read a short indicating the filename's size
	unsigned short fileNameSize;
	if(readE(sockfd, &fileNameSize, sizeof(fileNameSize)) < 0) return -1;
	fileNameSize = ntohs(fileNameSize);	//convert back to host byte order
	#if DEBUG	
		printf("\tfilename size: %d\n", fileNameSize);
	#endif

	//read the filename from the client
	char fileName[fileNameSize + 1];
	if (readE(sockfd, fileName, sizeof(fileName)) < 0) return -1;
	fileName[fileNameSize] = '\0';
	#if DEBUG
		printf("\tfilename: %s\n", fileName);
	#endif

	//server sends ACK
	if (sendE(sockfd, "ACK", sizeof("ACK"), 0) < 0) return -1;
	#if DEBUG
		printf("\tsent ACK\n");
	#endif

	//receive file size
	int fileSize_n;
	if(readE(sockfd, &fileSize_n, sizeof(fileSize_n)) < 0) return -1;
	int fileSize = (int)ntohl(fileSize_n);	//convert to host byte order
	#if DEBUG
		printf("\tfile size: %d\n", (int)fileSize);
	#endif

	//initialize hash
	char serverHash[16];
	MHASH mh;
	mh = mhash_init(MHASH_MD5);
	if(mh == MHASH_FAILED) {
		perror("Error: hash\n");
		return -1;
	}

	//receive the file in a loop
	//open the file for writing
	FILE *f = fopen(fileName, "w");
	if(f == NULL) {
		perror("Error: opening set file.\n");
		return -1;
	}
	int readSoFar = 0;
	//enter loop for reading bytes and saving to file
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
		mhash(mh, buffer, bytesRead);

		#if DEBUG
			printf("\tWrote %d bytes to %s\n", bytesRead, fileName);
		#endif
	}
	//get timestamp immediately after reading all of file
	double sTime = getSecSinceEpoch();

	fclose(f);
	
	//put hash data into serverHash array
	mhash_deinit(mh, serverHash);
	#if DEBUG
		int j;
		printf("\tServer hash: ");
		for(j = 0; j < 16; ++j) {
			printf("%hhx", serverHash[j]);
		}
		printf("\n");
	#endif

	//receive md5hash from client -- 16 bits
	char clientHash[16];
	if (readE(sockfd, clientHash, sizeof(clientHash)) < 0) return -1;
	#if DEBUG
		printf("\tClient hash: ");
		for (j = 0; j < 16; ++j) {
			printf("%hhx", clientHash[j]);
		}
		printf("\n");
	#endif

	//read the timestamp for throughput calculation
	char clientTime[64] = {'\0'};
	if (readE(sockfd, clientTime, sizeof(clientTime)-1) < 0) return -1;
	double cTime;
	sscanf(clientTime, "%lf", &cTime);
	#if DEBUG
		printf("\tserver time: %f\n\tclient time: %f\n", sTime, cTime);
	#endif

	//compare hashes
	if (memcmp(clientHash, serverHash, sizeof(serverHash)) == 0) {
		#if DEBUG
			printf("\tTransfer successful.\n");
		#endif
		//compute throughput
		double changeInTime = sTime - cTime;
		double throughput = (fileSize/changeInTime)/1000000;
		//send throughput data		
		char throughputS[256];
		sprintf(throughputS, "%d bytes transferred in %.4f seconds: %.4f Megabytes/sec", fileSize, changeInTime, throughput);
		if (sendE(sockfd, throughputS, strlen(throughputS), 0) < 0) return -1;
	} else {
		//hashes did not equal
		//send -1 instead of throughput
		if (sendE(sockfd, "-1", sizeof("-1"), 0) < 0) return -1;

		#if DEBUG
			printf("Transfer failed.\n");
		#endif
		if (remove(fileName) < 0) {
			perror("Error: remove\n");
			return -1;
		}
		#if DEBUG
			printf("%s deleted\n", fileName);
		#endif
	}
}

int dirRequest(int sockfd) {
	//get listing of my directory
	DIR *curDir = opendir(".");
	if (curDir == NULL) return -1;

	char dirList[4096] = "";
	struct dirent *entry = readdir(curDir);

	char *ptrInDirList = dirList;	
	while( entry != NULL) {
		if ((entry->d_name)[0] != '.') {
			strcat(dirList, "\t");
			strcat(dirList, entry->d_name);
			strcat(dirList, "\n");		
		}
		entry = readdir(curDir);
	}
	#if DEBUG
		printf("%s\n", dirList);
	#endif	
	
	//compute size of this listing
	int sizeList = strlen(dirList);
	#if DEBUG
		printf("\tlist size: %d\n", sizeList);
	#endif
	//send size to client
	int sizeList_n = htonl(sizeList);
	if (sendE(sockfd, &sizeList_n, sizeof(sizeList_n), 0) < 0) return -1;	

	int bytesSent = sendAll(sockfd, dirList, sizeList, 0);
	if (bytesSent < sizeList) return -1; 
	
	closedir(curDir);
	return 0;
}

//returns the current time measured in seconds since Unix epoch
double getSecSinceEpoch() {
	struct timeval tv;
	gettimeofday(&tv, NULL);
	return (tv.tv_sec + (tv.tv_usec/1000000.0));
}

#endif
