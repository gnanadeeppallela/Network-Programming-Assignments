/*
UDP Client program by Group-02
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

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */
typedef unsigned long u_long;
typedef unsigned short u_short;
extern int errno;

#define LINELEN 128

//int passiveTCP(const char *service,int qlen);
int errexit(const char *format, ...);
//int passivesock(const char *service, const char *transport,int qlen);

int connectUDP(const char *host, const char *service);

int connectsock(const char *host, const char *service,const char *transport);


#define QLEN 5
/*------------------------------------------------------------------------
* main
*------------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
int n=0,count=1;
struct sockaddr_in fsin;
char *service = "echo";
char buffer[10000];
int msock,ssock;
unsigned int alen;
char *host1;
char *file_n;
int port_n;
ssize_t s_file_n,recv_data;
//printf("Before switch");
int new_file;

switch (argc)
{
case 1:
host1 = "localhost";
break;
case 4:
host1 = argv[1];
file_n=argv[2];
port_n=atoi(argv[3]);
break;

default:
fprintf(stderr, "usage: UDP [host [port]]\n");
exit(1);
}

//printf("After Switch");
msock = connectUDP(host1,service);

send(msock,file_n,2048,0);

printf("%s is the file requested from the server\n",file_n);
if((recv_data=recv(msock,buffer,10000,0))>0)
{

if(1)
{
printf("File is created\n");
new_file=open(file_n,O_WRONLY|O_CREAT,0644);
count++;
}

if(1)
{

write(new_file,buffer,recv_data);

printf("%s \nis the data received from the server\n",buffer);
}
close(msock);
}
printf("Done\n");

(void) close(msock);

}



/*------------------------------------------------------------------------
* connectUDP - connect to a specified UDP service on a specified host
*------------------------------------------------------------------------
*/
int connectUDP(const char *host, const char *service )
/*
* Arguments:
* host - name of host to which connection is desired
* service - service associated with the desired port
*/
{
return connectsock(host,service,"udp");
}



/*------------------------------------------------------------------------
* connectsock - allocate & connect a socket using TCP or UDP
*------------------------------------------------------------------------
*/
int connectsock(const char *host, const char *service, const char *transport )
/*
* Arguments:
* host - name of host to which connection is desired
* service - service associated with the desired port
* transport - name of transport protocol to use ("tcp" or "udp")
*/
{
struct hostent *phe; /* pointer to host information entry */
struct servent *pse; /* pointer to service information entry */
struct protoent *ppe; /* pointer to protocol information entry*/
struct sockaddr_in sin; /* an Internet endpoint address */
int s, type; /* socket descriptor and socket type */
memset(&sin, 0, sizeof(sin));
sin.sin_family = AF_INET;
/* Map service name to port number */
if ( pse = getservbyname(service, transport) )
sin.sin_port = htons(10026);
else if ( (sin.sin_port = htons((u_short)atoi(service))) == 0 )
errexit("can't get \"%s\" service entry\n", service);
/* Map host name to IP address, allowing for dotted decimal */
if ( phe = gethostbyname(host) )
memcpy(&sin.sin_addr, phe->h_addr, phe->h_length);
else if ( (sin.sin_addr.s_addr = inet_addr(host)) == INADDR_NONE )
errexit("can't get \"%s\" host entry\n", host);
/* Map transport protocol name to protocol number */
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
/* Connect the socket */
if (connect(s, (struct sockaddr *)&sin, sizeof(sin)) < 0)
errexit("can't connect to %s.%s: %s\n", host, service,strerror(errno));
return s;
}



/*------------------------------------------------------------------------
* errexit - print an error message and exit
*------------------------------------------------------------------------
*/
/*VARARGS1*/
int errexit(const char *format, ...)
{
va_list args;
va_start(args, format);
vfprintf(stderr, format, args);
va_end(args);
exit(1);
}

