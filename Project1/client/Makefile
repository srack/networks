all: myftp

myftp: ftp_client.o
	gcc ftp_client.o -o myftp -lmhash

ftp_client.o: ftp_client.c client_lib.h ftp_client_lib.h
	gcc -c ftp_client.c

clean:
	rm -f myftp *.o *~
