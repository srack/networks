/* Adrian Gerbaud and Samantha Rack
 * CSE 30264
 * Project 2
 * server.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>
#include <pthread.h>

#include "server_lib.h"
#include "../shared.h"

#define BACKLOG 10

char *logPrefix = "/var/log/therm/temp_logs/g02_";

void *handleClient(void * param);
void recvStruct(int sockfd, sensorData *s);
void writeToLog2(sensorData *s0, sensorData *s1);
void writeToLog1(sensorData *s0);


int main (int argc, char **argv) {
	char *port;
	if (argc == 2) {
		port = argv[1];
	} else port = "9763";
	
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
		//reset for each new client
		int cSize = sizeof(struct sockaddr_storage);

		#if DEBUG
			printf("Waiting for new connection...\n");
		#endif
	
		//accept a new client
		if ((sockc = accept(sockl, (struct sockaddr*)&clientInfo, &cSize)) < 0) {
			perror("Error: accept\n");
			return -1;
		}

		#if DEBUG
			printf("Connected with another client!\n");
		#endif

		int *pSock = malloc(sizeof(int));
		*pSock = sockc;

		pthread_t id;
		int rv = pthread_create(&id, NULL, handleClient, (void *)pSock);
	} 
	
	if (close(sockl) < 0) {
		perror("Error: close\n");
		return -1;
	}

	return 0;
}


void *handleClient(void * param) {

        int sockfd = *((int *)param);
	
	//structure that will hold the data we receive
	sensorData sData0, sData1;	

	recvStruct(sockfd, &sData0);
	if (sData0.totalNum == 1) {
		if (sData0.actionReq == 0) {
			writeToLog1(&sData0);
		}	
	} 
	//else, there are two sensor data structures being sent
	else {
		recvStruct(sockfd, &sData1);
		if (sData0.actionReq == 0 && sData1.actionReq == 0) {
			writeToLog2(&sData0, &sData1);
		}
	}
	
	if (close(sockfd) < 0) {
		perror("Error: close\n");
		return;
	}
	free(param);

}

void recvStruct(int sockfd, sensorData *s) {

	//receive hostName
	if (readE(sockfd, s->hostName, sizeof(s->hostName)) < 0) return;
	//receive totalNum
	int totalNum_n;
	if (readE(sockfd, &totalNum_n, sizeof(int)) < 0) return;
	s->totalNum = ntohl(totalNum_n);
	//receive id
	int id_n;
	if (readE(sockfd, &id_n, sizeof(int)) < 0) return;
	s->id = ntohl(id_n);

	//receive doubles, sent in 7 byte character arrays
	char dataStr[7] = {'\0'};
	if (readE(sockfd, dataStr, sizeof(dataStr)) < 0) return;
	sscanf(dataStr, "%lf", &(s->data));
	char lowStr[7] = {'\0'};
	if (readE(sockfd, lowStr, sizeof(lowStr)) < 0) return;
	sscanf(lowStr, "%lf", &(s->low));
	char highStr[7] = {'\0'};
	if (readE(sockfd, highStr, sizeof(highStr)) < 0) return;
	sscanf(highStr, "%lf", &(s->high));
	
	//receive timestamp
	if (readE(sockfd, s->timestamp, sizeof(s->timestamp)) < 0) return;
	//receive actionReq
	int actionReq_n;
	if (readE(sockfd, &actionReq_n, sizeof(int)) < 0) return;
	s->actionReq = ntohl(actionReq_n);	

	#if DEBUG
		printStruct(s);
	#endif
}


void writeToLog2(sensorData *s0, sensorData *s1) {
	#if DEBUG
//		printStruct(s0);
//		printStruct(s1);
	#endif

	//extract time data
	int year, month, day, hour, minute, second;
	sscanf(s0->timestamp, "%04d %02d %02d %02d %02d %02d", &year, &month, &day, &hour, &minute, &second);
	

	//create the filename -- use logPrefix for beginning of path
	char fileName[128] = {'\0'};
	sprintf(fileName, "%s%04d_%02d_%s", logPrefix, year, month, s0->hostName);
	#if DEBUG
		//printf("%s\n", fileName);
	#endif

	//create the line of text that will be written to the log
	char logMsg[128] = {'\0'};
	sprintf(logMsg, "%d %02d %02d %02d %02d %.2lf %.2lf\n", year, month, day, hour, minute, s0->data, s1->data);
	#if DEBUG
		printf("\n%s\n", logMsg);
	#endif		

	FILE *f = fopen(fileName, "a");
	if (f == NULL) {
		printf("Error opening log file %s. Reading not logged.\n", fileName);
		return;
	}
	

	//actually write message to the file	
	fwrite(logMsg, sizeof(char), strlen(logMsg), f);

	fclose(f);

}

//for writing to the log when there is only one sensor data sent
void writeToLog1(sensorData *s0) {
	#if DEBUG
	//	printStruct(s0);
	#endif

	//extract time data
	int year, month, day, hour, minute, second;
	sscanf(s0->timestamp, "%04d %02d %02d %02d %02d %02d", &year, &month, &day, &hour, &minute, &second);
	

	//create the filename -- use logPrefix for beginning of path
	char fileName[128] = {'\0'};
	sprintf(fileName, "%s%04d_%02d_%s", logPrefix, year, month, s0->hostName);
	#if DEBUG
		//printf("%s\n", fileName);
	#endif

	//create the line of text that will be written to the log
	char logMsg[128] = {'\0'};
	sprintf(logMsg, "%d %02d %02d %02d %02d %.2lf\n", year, month, day, hour, minute, s0->data);
	#if DEBUG
		printf("\n%s\n", logMsg);
	#endif		

	FILE *f = fopen(fileName, "a");
	if (f == NULL) {
		printf("Error opening log file %s. Reading not logged.\n", fileName);
		return;
	}
	

	//actually write message to the file	
	fwrite(logMsg, sizeof(char), strlen(logMsg), f);

	fclose(f);


}

