#include <stdlib.h>
#include <stdio.h>
#include <ctype.h>
#include <string.h>
#define MAXBUFLEN 10000

char *getfile(char *filename);

int main(int argc, char *argv[])
{
	FILE *f1;
	char *c1;

	char *filename=argv[1];
	char *location=argv[2];
	char *total=strcat(location,"/");
	printf("Your have entered the filename: %s and location: %s\n",filename,location);
	total=strcat(total,filename);
	//printf("%s", total);
	c1=getfile(filename);

	f1=fopen(total,"w");
	printf("%s\n",c1);
	fputs(c1,f1);
	printf("File is successfully copied to the specified location.\n");
	fclose(f1);
}

