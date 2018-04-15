#include <rpc/rpc.h>

#define	RPC_SVC
#include "rfour1.h"

static	char *retcode;

extern char *getfile(char *);

extern char	**
getfile_1_svc(char **w, struct svc_req *rqstp)
{
	retcode = getfile(*(char**)w);
	return &retcode;
}


