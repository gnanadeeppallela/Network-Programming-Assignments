/*
 * Please do not edit this file.
 * It was generated using rpcgen.
 */

#ifndef _RFOUR1_H_RPCGEN
#define _RFOUR1_H_RPCGEN

#include <rpc/rpc.h>


#ifdef __cplusplus
extern "C" {
#endif

#define MAXBUFLEN 10000

#define FOUR1PROG 0x30090950
#define FOUR1VERS 1

#if defined(__STDC__) || defined(__cplusplus)
#define GETFILE 1
extern  char ** getfile_1(char **, CLIENT *);
extern  char ** getfile_1_svc(char **, struct svc_req *);
extern int four1prog_1_freeresult (SVCXPRT *, xdrproc_t, caddr_t);

#else /* K&R C */
#define GETFILE 1
extern  char ** getfile_1();
extern  char ** getfile_1_svc();
extern int four1prog_1_freeresult ();
#endif /* K&R C */

#ifdef __cplusplus
}
#endif

#endif /* !_RFOUR1_H_RPCGEN */
