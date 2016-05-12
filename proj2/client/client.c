/* Adrian Gerbaud and Samantha Rack
 * CSE 30264
 * Project 2
 * client.c
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/socket.h>
#include <netdb.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <time.h>

#include "client_lib.h"
#include "../shared.h"


void readTherm(sensorData *s);
float CtoF(float C);

//path to the error log file
char *errorLog = "/var/log/therm/error/g02_error_log";

int main (int argc, char **argv) {

	/*******************************
	 * COMMAND LINE PARSING
	 *******************************/

	char *host;
	int port;	
	if (argc < 2) {
		printf("Usage: %s server_name [port]\n", argv[0]);
		return -1;
	}
	host = argv[1];
	if (argc == 2) port = PORT;
	else port = atoi(argv[2]);
	

	/********************************
	 * READ FROM CONFIG FILE 
	 *******************************/
	
	//open file, check that it exists
	FILE *configFile = fopen("/etc/t_client/client.conf", "r");
	if (configFile == NULL) {
		perror("Error opening config file.\n");

		//write to the error log
		FILE *logFile = fopen(errorLog, "a");
		if(logFile == NULL) {
			perror("Error opening error log. Nothing recorded.\n");
			return -1;
		}
		char *msg = "Error opening config file.\n";
		fwrite(msg, sizeof(char), strlen(msg)/sizeof(char), logFile);
		fclose(logFile);
		
		return -1;
	}

	//read line one -- number of sensors
	int numSensors = 0;
	fscanf(configFile, "%d\n", &(numSensors));
	#if DEBUG
		printf("%d\n", numSensors);  
	#endif

	//we are done if there are no sensors
	if (numSensors == 0) {
		#if DEBUG
			printf("No sensors. Closing program.\n");
			fclose(configFile);
			return 0;
		#endif
	}

	//allocate dynamic array to store each of the sensor data
	sensorData *sData = malloc(numSensors*sizeof(sensorData));

	//get the client's host name
	char hostStr[32] = {'\0'};
	gethostname(hostStr, sizeof(hostStr));

	//get time
	time_t rawT;
	struct tm *now;
	time(&rawT);
	now = localtime(&rawT);

	//read line two through N -- starts with sensor 0
	int i;
	for (i = 0; i < numSensors; ++i) {
		//zero the structure
		bzero( (void *)&(sData[i]), sizeof(sData[i]));
	
		//fill in the host name -- array copy
		int j;
		for (j = 0; j < 32; ++j) {
			sData[i].hostName[j] = hostStr[j];
		}
		
		//fill in total number of sensors attached to the host
		sData[i].totalNum = numSensors;

		//fill in the id of the sensor
		sData[i].id = i;

		//fill in high and low values
		fscanf(configFile, "%lf %lf", &(sData[i].low), &(sData[i].high));

		//fill in timestamp
		sprintf(sData[i].timestamp, "%04d %02d %02d %02d %02d %02d", now->tm_year + 1900, now->tm_mon + 1, now->tm_mday, now->tm_hour, now->tm_min, now->tm_sec);

		//fill in request status -- zero for all packets
		sData[i].actionReq = 0;
	}

	//finished reading from the config file
	fclose(configFile);
	configFile = NULL;
	

	/*******************************
	 * READ THERMOMETERS
	 ******************************/

	for (i = 0; i < numSensors; ++i) {
		readTherm(&sData[i]);
	}


	/************************************
	 * FILL IN STRUCTURES AND GET SOCKET
	 ***********************************/

	//socket creation
	int sockfd = getSocket();
	if (sockfd < 0) {
		free(sData);
		return -1;
	}

	//fill in server address structure
	struct sockaddr_in serverInfo;
	if (getServerAddr(host, port, &serverInfo) < 0) return -1;


	/*******************************
	 * CONNECT TO SERVER 
	 ******************************/
	#if DEBUG
		printf("Waiting to connect with server...\n");
	#endif
	if (connect(sockfd, (struct sockaddr*)&serverInfo, sizeof(struct sockaddr)) < 0) {
		perror("Error: connect\n");
		free(sData);
		return -1;
	}
	#if DEBUG
		printf("Connected!\n");
	#endif


	/******************************
	 * SEND THE STRUCT AND CLEAN UP
	 *****************************/

	for (i = 0; i < numSensors; ++i) {
		sendStruct(sockfd, &sData[i]);
		#if DEBUG
			printStruct(&sData[i]);
		#endif
	}

	//close our socket to clean up
	if(close(sockfd) < 0) {
		perror("Error: close\n");
		free(sData);
		return -1;
	}

	//free the allocated memory for sData
	free(sData);

}


int sendStruct(int sockfd, sensorData *s) {

	//send hostName, totalNum, and id	
	if (sendE(sockfd, s->hostName, sizeof(s->hostName), 0) < 0) return -1;
	int totalNum_n = htonl(s->totalNum);
	if (sendE(sockfd, &totalNum_n, sizeof(int), 0) < 0) return -1;
	int id_n = htonl(s->id);	
	if (sendE(sockfd, &id_n, sizeof(int), 0) < 0) return -1;
	
	// to send doubles, print them to a string with a specified number of decimal points so the server can parse them
	// will have maximum of three digits before the decimal (3+1), and then 2 decimal places ( = 6) 
	char dataStr[7] = {'\0'};
	sprintf(dataStr, "%.2lf", s->data);
	if (sendE(sockfd, dataStr, sizeof(dataStr), 0) < 0) return -1;

	char lowStr[7] = {'\0'};
	sprintf(lowStr, "%.2lf", s->low);
	if (sendE(sockfd, lowStr, sizeof(lowStr), 0) < 0) return -1;
	
	char highStr[7] = {'\0'};
	sprintf(highStr, "%.2lf", s->high);
	if (sendE(sockfd, highStr, sizeof(highStr), 0) < 0) return -1;

	//send timestamp and actionReq
	if (sendE(sockfd, s->timestamp, sizeof(s->timestamp), 0) < 0) return -1;
	int actionReq_n = htonl(s->actionReq);
	if (sendE(sockfd, &actionReq_n, sizeof(int), 0) < 0) return -1;
	
	return 0; 	//successfully sent the structure

}




/* Attribution: The code below is based largely on/copied from that written by Jeff
 *	Sadowski <jeff.sadowski@gmail.com> with information gathered from David L.
 *	Vernier and Greg KH. 
 */


/* This is close to the structure I found in Greg's Code */
struct packet {
	unsigned char measurements;
	unsigned char counter;
	int16_t measurement0;
	int16_t measurement1;
	int16_t measurement2; 
};

/* Function to convert Celsius to Fahrenheit*/
float CtoF(float C) {
	return (C*9.0/5.0)+32;
}

/* Function Name: readTherm
 * Purpose:  Reads data from the sensor, and fills in the information in
 * 	the sensorData structure passed in
 */
void readTherm(sensorData *s) {

	//determine which sensor to read
	char *fileName;
	if (s->id == 0)
		fileName = "/dev/gotemp";
	else if (s->id == 1)	
		fileName = "/dev/gotemp2";
	else	//there are 2 sensors max, so do nothing, there must be an error
		return;

	/********************************
	 * READ THE SENSOR
	 ********************************/
	struct stat buf;
	struct packet temp;	
	int fd;


	//his attribution
	/* I got this number from the GoIO_SDK and it matched 
   	what David L. Vernier got from his Engineer */
	float conversion = 0.0078125;
	
	//check that we are root
	if (stat(fileName, &buf)) {
		if (mknod(fileName, S_IFCHR | S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP |S_IROTH | S_IWOTH, makedev(180,176))) {
			//ERROR -- must be root
			//write to the error log
			FILE *logFile = fopen(errorLog, "a");
			if(logFile == NULL) {
				perror("Error opening error log. Nothing recorded.\n");
				return;
			}
			char *msg = "Error reading sensor, must be root.\n";
			fwrite(msg, sizeof(char), strlen(msg)/sizeof(char), logFile);
			fclose(logFile);

			return;	//do nothing else		
		}
	}
	
	//file descriptor for the open device file
	fd = open(fileName, O_RDONLY);

	//if can't open, check permssions
	if (fd == -1) {
		//ERROR -- couldn't read the file	
		//write to the error log
		FILE *logFile = fopen(errorLog, "a");
		if(logFile == NULL) {
			perror("Error opening error log. Nothing recorded.\n");
			return;
		}
		char *msg = "Error reading sensor, couldn't read the file.\n";
		fwrite(msg, sizeof(char), strlen(msg)/sizeof(char), logFile);
		fclose(logFile);
		close(fd);		
		return;	//do nothing else
	}

	//if error on read, it may not be plugged in
	if ( read(fd, &temp, sizeof(temp)) != 8) {
		//ERROR
		//write to the error log
		FILE *logFile = fopen(errorLog, "a");
		if(logFile == NULL) {
			perror("Error opening error log. Nothing recorded.\n");
			return;
		}
		char *msg = "Error reading sensor, may not be plugged in.\n";
		fwrite(msg, sizeof(char), strlen(msg)/sizeof(char), logFile);
		fclose(logFile);
		close(fd);		
		return;
	}

	close(fd);

	//fill in the structure
	s->data = CtoF( ( (float)temp.measurement0)*conversion);
	return;

}



























