all: myftpd

myftpd: ftp_server.o
	gcc ftp_server.o -o myftpd -lmhash

ftp_server.o: ftp_server.c server_lib.h ftp_server_lib.h
	gcc -c ftp_server.c

clean:
	rm -f myftpd *.o *~
