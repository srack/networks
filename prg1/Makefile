all: udpclient

udpclient: udpclient.o
	gcc udpclient.o -o udpclient

udpclient.o: udpclient.c
	gcc -c udpclient.c 

clean:
	rm -f *.o udpclient 
