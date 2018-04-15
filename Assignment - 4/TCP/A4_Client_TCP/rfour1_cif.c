#include <rpc/rpc.h>
#include <stdio.h>
#include "rfour1.h"

extern CLIENT *handle;
static char **ret;


char *getfile(char *filename)
{
	char **arg;
	arg=&filename;
	ret=getfile_1(arg,handle);

	return ret==0 ? 0 : *ret;
}

