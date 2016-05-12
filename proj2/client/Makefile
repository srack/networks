all: therm

therm: client.o
	gcc client.o -o therm 

client.o: client.c client_lib.h ../shared.h 
	gcc -c client.c

clean:
	rm -f therm *.o *~
