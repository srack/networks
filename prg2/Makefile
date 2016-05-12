all: udpserver

udpserver: udpserver.o
	gcc udpserver.o -o udpserver

udpserver.o: udpserver.c
	gcc -c udpserver.c 

clean:
	rm -f *.o udpserver 
