/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _SUB_H_RPCGEN
#define _SUB_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif


struct moveStruct {
	int turn;
	int x1;
	int y1;
};
typedef struct moveStruct moveStruct;

#define MOVESUB_PROG 0x31660284
#define MOVESUB_VERSION 1

#if defined(__STDC__) || defined(__cplusplus)
#define MOVESUB_PROC 1
extern  int * movesub_proc_1(moveStruct *, CLIENT *);
extern  int * movesub_proc_1_svc(moveStruct *, struct svc_req *);
extern int movesub_prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define MOVESUB_PROC 1
extern  int * movesub_proc_1();
extern  int * movesub_proc_1_svc();
extern int movesub_prog_1_freeresult ();
#endif /* K&R C */

/* the xdr functions */

#if defined(__STDC__) || defined(__cplusplus)
extern  bool_t xdr_moveStruct (XDR *, moveStruct*);

#else /* K&R C */
extern bool_t xdr_moveStruct ();

#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_SUB_H_RPCGEN */
