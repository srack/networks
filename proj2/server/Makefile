all: thermd

thermd: server.o
	gcc server.o -o thermd -lpthread

server.o: server.c server_lib.h ../shared.h 
	gcc -c server.c

clean:
	rm -f thermd *.o *~
