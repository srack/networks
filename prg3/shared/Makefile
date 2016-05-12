all: gen

gen: sub.x
	rpcgen -C sub.x

clean: 
	rm -f *.o *~ sub_clnt.c sub_svc.c sub_xdr.c
