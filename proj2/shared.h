/* Adrian Gerbaud and Samantha Rack
 * CSE 30264
 * Project 2
 * shared.h
 */


#ifndef SHARED_H
#define SHARED_H

typedef struct {
	char hostName[32];
	int totalNum;
	int id;
	double data;
	double low;
	double high;
	char timestamp[32];
	int actionReq;
} sensorData;

#define DEBUG 0
#define PORT 9763

void printStruct(sensorData *s) {
	printf("\thostName = %s\n\ttotalNum = %d\n\tid = %d\n\tdata = %lf\n\tlow = %lf\n\thigh = %lf\n\ttimestamp = %s\n\tactionReq = %d\n\n", s->hostName, s->totalNum, s->id, s->data, s->low, s->high, s->timestamp, s->actionReq);


}



#endif
