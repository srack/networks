all: rpc_subd

rpc_subd: sub_server.o sub_svc.o sub_xdr.o
	gcc sub_server.o ../shared/sub_svc.o ../shared/sub_xdr.o -o rpc_subd

sub_server.o: sub_server.c ../shared/sub.h ../shared/shared.h
	gcc -c sub_server.c

sub_svc.o: ../shared/sub_svc.c ../shared/sub.h
	gcc -c ../shared/sub_svc.c; \
	mv sub_svc.o ../shared

sub_xdr.o: ../shared/sub_xdr.c ../shared/sub.h
	gcc -c ../shared/sub_xdr.c; \
	mv sub_xdr.o ../shared

clean:
	rm -f *.o ../shared/*.o *~ rpc_subd
