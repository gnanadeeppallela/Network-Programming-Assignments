/* UDPDaytime.c - main 
   Group-02 Assignment-01
   Make file, Output file included
*/

#define __USE_BSD 1
#include <stdarg.h>
#include <errno.h>
#include <netinet/in.h>
#include <time.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#define BUFSIZE 64
#define UNIXEPOCH 2208988800 /* UNIX epoch, in UCT secs : Number of seconds from1st January 1900 */
#define MSG "what time is it?\n"

#ifndef INADDR_NONE
#define INADDR_NONE 0xffffffff
#endif
 extern int errno;
typedef unsigned short u_short;
extern int errno;
int errexit(const char *format, ...);


typedef unsigned long u_long;
extern int errno;

/*connectUDP - connect to a specified TCP service on a specified host */
/*
* Arguments:
* host - name of host to which connection is desired
* service - service associated with the desired port
*/

int connectsock(const char *host, const char *service, const char *transport);
int connectUDP(const char *host, const char *service);

int errexit(const char *format, ...);

/*------------------------------------------------------------------------
* main - UDP client for TIME service that prints the resulting time
*------------------------------------------------------------------------
*/
int main(int argc, char *argv[])
{
	char *host = "localhost"; /* host to use if none supplied */
	char *service = "time"; /* default service name */
	time_t now; /* 32-bit integer to hold time */
	int s, n; /* socket descriptor, read count*/
	switch (argc) 
	{
		case 1:
			host = "localhost";
			break;
		case 3:
			service = argv[2];
		case 2:
			host = argv[1];
			break;
		default:
			fprintf(stderr, "usage: UDPtime [host [port]]\n");
			exit(1);
	}
s = connectUDP(host, service);
(void) write(s, MSG, strlen(MSG));

/* Read the time */
n = read(s, (char *)&now, sizeof(now));
if (n < 0)
	errexit("read failed: %s\n", strerror(errno));
now = ntohl((u_long)now); /* put in host byte order */
now -= UNIXEPOCH; /* convert UCT to UNIX epoch */
printf("%s", ctime(&now));
exit(0);

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
* transport- name of transport protocol to use ("tcp" or "udp")
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
	if ( pse = getservbyname(service, transport))
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

int connectUDP(const char *host, const char *service )
/*
* Arguments:
* host - name of host to which connection is desired
* service - service associated with the desired port
*/
{
	return connectsock(host, service, "udp");
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
