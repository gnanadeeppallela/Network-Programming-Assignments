/*
TCP Client program by group-02
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

#define __USE_BSD 1
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <stdlib.h>
#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif /* INADDR_NONE */


/* TCPdaytime.c - TCPdaytime, main */
#include <unistd.h>

typedef unsigned short u_short;
extern int errno;
extern int errno;
#define LINELEN 128

int TCPdaytime(const char *host, const char *service);
int errexit(const char *format, ...);
int connectTCP(const char *host, const char *service);



int errexit(const char *format, ...);


/* connectTCP.c - connectTCP */
int connectsock(const char *host, const char *service,const char *transport);




//main - TCP client for DAYTIME service

int main(int argc, char *argv[])
{
char *host1 = "localhost"; /* host to use if none supplied */
char *host2 = "localhost"; /* host to use if none supplied */
char *service = "daytime"; /* default service port*/
switch (argc) {
case 1:
host1 = "localhost";
host2 = "localhost";
break;
case 4:
service = argv[3];
/* FALL THROUGH */
case 3:
host1 = argv[1];
host2 = argv[2];
break;
default:
fprintf(stderr, "usage: TCPdaytime [host [port]]\n");
exit(1);
}
int s1=TCPdaytime(host1, service);

int s2=TCPdaytime(host2, service);

printf("%d seconds is the difference between both the Time's",s2-s1);

exit(0);
}



/* TCPdaytime - invoke Daytime on specified host and print results*/
int TCPdaytime(const char *host, const char *service)
{
char buf[LINELEN+1];
int s, n,a,b,c,sum;
/* buffer for one line of text socket, read count*/
s = connectTCP(host, service);
while( (n = read(s, buf, LINELEN)) > 0) {
buf[n] = '\0';
}
//printf("%s\n",buf);

printf("Time from server: ");
for(int i=16;i<=23;i++)
printf("%c",buf[i]);
printf("\n");

a=buf[22]-'0';
a=a*10+(buf[23]-'0'); 
  
b=buf[19]-'0';
b=b*10+(buf[20]-'0');
b=b*60; //Convert minutes to seconds

c=buf[16]-'0';
c=c*10+(buf[17]-'0');
c=c*3600; //convert hours to seconds

sum=a+b+c; //Total number of seconds
return sum;
}






/*connectTCP - connect to a specified TCP service on a specified host*/
int connectTCP(const char *host, const char *service )
/*
* Arguments:
* host - name of host to which connection is desired
* service - service associated with the desired port
*/
{
return connectsock( host, service, "tcp");
}




/*connectsock - allocate & connect a socket using TCP or UDP*/
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
sin.sin_port = pse->s_port;
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



/*errexit - print an error message and exit*/
int errexit(const char *format, ...)
{
va_list args;
va_start(args, format);
vfprintf(stderr, format, args);
va_end(args);
exit(1);
}

