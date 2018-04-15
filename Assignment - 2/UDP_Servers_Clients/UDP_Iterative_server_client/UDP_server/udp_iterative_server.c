/*
UDP iterative server program by group-02
Make file, code, output are provided
Reference: Internetworking with TCP/IP Vol. 3, Client-Server programming and applications, Comer and Stevens, Linux/POSIX Sockets version, ISBN: 0-13-032071-4, 2001.
*/
#include <errno.h>
#include <netinet/in.h>
#include <time.h>
#include <unistd.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <fcntl.h>

#define __USE_BSD 1
#define UNIXEPOCH 2208988800 
int passiveUDP(const char *service);
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */
typedef unsigned long u_long;
typedef unsigned short u_short;
extern int errno;
int passivesock(const char *service, const char *transport, int qlen);
int errexit(const char *format, ...);
u_short portbase = 0;
/* port base, for non-root servers
*/
/*------------------------------------------------------------------------
* passivesock - allocate & bind a server socket using TCP or UDP
*------------------------------------------------------------------------
*/
#define LINELEN 128

int passiveTCP(const char *service,int qlen);
int errexit(const char *format, ...);
int passivesock(const char *service, const char *transport,int qlen);
int TCPdaytimed(int fd);
#define QLEN 5
/*------------------------------------------------------------------------
* main - Iterative TCP server for DAYTIME service
*------------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
struct sockaddr_in fsin;
char *service = "echo";
char buffer[10000];
int msock,ssock;
unsigned int alen;
int fil;
size_t rd,wr;

//printf("Before switch \n");
switch (argc) {
case 1:
break;
case 2:
service = argv[1];
break;
default:
errexit("usage: TCPdaytimed [port]\n");
}

//printf("After Switch \n");
//fflush(stdout);
msock = passiveUDP(service);
while (1) {
printf("Waiting for the client \n");
//fflush(stdout);
//ssock = accept(msock, (struct sockaddr *)&fsin, &alen);
//ssock = 2;
//if (ssock < 0)
//errexit("accept failed: %s\n", strerror(errno));

//fflush(stdout);
int file_n=recvfrom(msock,buffer,10000,0,(struct sockaddr *)&fsin, &alen);

printf("File is opened in read only mode\n");
fil=open(buffer,O_RDONLY);
//printf("%s",buffer);
while((rd=read(fil,buffer,2048))>0)
{
wr=sendto(msock,buffer,rd,0,(struct sockaddr *)&fsin,sizeof(fsin));
printf("The data from the file is sent to the client\n\n");
}

(void) close(ssock);
}

}



/* passiveUDP.c - passiveUDP */

/*------------------------------------------------------------------------
* passiveUDP - create a passive socket for use in a UDP server
*------------------------------------------------------------------------
*/
int passiveUDP(const char *service)
/*
* Arguments:
*
service - service associated with the desired port
*/
{
return passivesock(service, "udp", 0);
}


int passivesock(const char *service, const char *transport, int qlen)
/*
* Arguments:
* service
- service associated with the desired port
* transport - transport protocol to use ("tcp" or "udp")
* qlen
- maximum server request queue length
*/
{
struct servent *pse; /* pointer to service information entry */
struct protoent *ppe; /* pointer to protocol information entry*/
struct sockaddr_in sin; /* an Internet endpoint address*/
int s, type;
/* socket descriptor and socket type
*/
memset(&sin, 0, sizeof(sin));
sin.sin_family = AF_INET;
sin.sin_addr.s_addr = INADDR_ANY;
/* Map service name to port number */
if ( pse = getservbyname(service, transport) )
sin.sin_port = htons(10026);
else if ( (sin.sin_port = htons((u_short)atoi(service))) == 0 )
errexit("can't get \"%s\" service entry\n", service);
/* Map protocol name to protocol number */
if ( (ppe = getprotobyname(transport)) == 0)
errexit("can't get \"%s\" protocol entry\n", transport);
/* Use protocol to choose a socket type */
if (strcmp(transport, "udp") == 0)
type = SOCK_DGRAM;
else
type = SOCK_STREAM;
/* Allocate a socket */
s = socket(PF_INET, type, ppe->p_proto);
if (s < 0)
errexit("can't create socket: %s\n", strerror(errno));
/* Bind the socket */
if (bind(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
errexit("can't bind to %s port: %s\n", service,
strerror(errno));
if (type == SOCK_STREAM && listen(s, qlen) < 0)
errexit("can't listen on %s port: %s\n", service,
strerror(errno));
return s;
}


int errexit(const char *format, ...)
{
va_list args;
va_start(args, format);
vfprintf(stderr, format, args);
va_end(args);
exit(1);
}

