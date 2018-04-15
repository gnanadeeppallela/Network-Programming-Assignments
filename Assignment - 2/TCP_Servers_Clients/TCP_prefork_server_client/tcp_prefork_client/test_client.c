// CMPE_297 Assignment_2
// TCP Client
// Author - Team #2
// This is a TCP test client program. It connects to the server
// and receives total number of processes preforked at the
// server side. It then prints the number on the output screen.
// To compile: gcc test_client.c -o client
// To run: ./client server_address server_port

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/wait.h>

#define ERROR -1
#define BUFFER 1024

int connectsock(const char *host, const char *service, const char *transport)
{
	struct hostent *phe;
	struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in remote_server;
	int sock, type;

	//reset socket address structure.
	memset(&remote_server,0,sizeof(remote_server));

	remote_server.sin_family = AF_INET;

	//map service name to port number
	if(pse = getservbyname(service, transport))
		remote_server.sin_port = pse->s_port;
	else if((remote_server.sin_port = htons((u_short)atoi(service))) == 0)
		//perror("Cant get \"%s\" service entry\n", service);
		perror("Cant get service entry\n");

	//map host name to IP address allowing dotted decimal
	if(phe = gethostbyname(host))
		memcpy(&remote_server.sin_addr, phe->h_addr, phe->h_length);
	else if ((remote_server.sin_addr.s_addr = inet_addr(host) == INADDR_NONE))
		//perror("Cant get \"%s\" host entry\n", host);
		perror("Cant get host entry\n");

	bzero(&remote_server.sin_zero, 8);

	//map transport protocol name to protocol number
	ppe = getprotobyname(transport);
	if((ppe == NULL))
		//perror("cant get \"%s\" protocol entry\n", transport);
		perror("Cant get protocol entry\n");

	//use protocol to chose a socket type
	if(strcmp(transport, "udp") == 0)
		type = SOCK_DGRAM;
	else
		type = SOCK_STREAM;

	//allocate a socket
	sock = socket(PF_INET, type, ppe->p_proto);
	if(sock < 0)
		//perror("Cant create socket: %s\n", strerror(errno));
		perror("Cant create socket\n");

	//connect the socket
	if(connect(sock, (struct sockaddr *)&remote_server, sizeof(remote_server)) < 0)
		//perror("Cant connect to %s %s: %s\n", host, service, sererror(errno));
		perror("Cant connect to the remote host\n");

	return sock;
}


int main(int argc, char **argv)
{
	int sock;
	int len;
	char output[BUFFER];

	if(argc < 3)
	{
		printf("Usage: ./client <server IP> <server port> \n");
		exit(0);
	}
	printf("\n************************************************************************\n");
	printf("TCP test client: This client connects to the concurrent preforked TCP server\n");
	printf("receives number of preforked child processes at the server and prints it.\n");
	printf("**************************************************************************\n\n");
		
	printf("Initiating a connection to TCP server ===>  ");
	sock = connectsock(argv[1], argv[2], "tcp");
	if (sock < 0){
		printf("Connection Failed\n");
		exit(-1);
	}
	
	// connection successfuly established
	printf("Connection established."); 
	memset(output, 0, BUFFER);
	
	// receive server's response and print.
	len = recv(sock, output, BUFFER, 0);
	if (len < 0)
	{
		perror("recv error..");
		exit(-1);
	}
	output[len] = '\0';
	printf("%s", output);
	close(sock);
}

