all: submarine

submarine: sub_client.o sub_clnt.o sub_xdr.o
	gcc sub_client.o ../shared/sub_clnt.o ../shared/sub_xdr.o -o submarine

sub_client.o: sub_client.c ../shared/sub.h ../shared/shared.h
	gcc -c sub_client.c

sub_clnt.o: ../shared/sub_clnt.c ../shared/sub.h
	gcc -c ../shared/sub_clnt.c; \
	mv sub_clnt.o ../shared

sub_xdr.o: ../shared/sub_xdr.c ../shared/sub.h
	gcc -c ../shared/sub_xdr.c; \
	mv sub_xdr.o ../shared

clean:
	rm -f *.o ../shared/*.o *~ submarine
