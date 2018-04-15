// CMPE_207 Assignment_2
// UDP test Client
// Author - Team #2
// This is a UDP test client program. It connects to the server
// and receives total number of processes preforked at the
// server side. It then prints the number on the output screen.
// To compile: gcc udp_test_client.c -o client
// To run: ./client <server_address> <server_port>

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

int createsock(const char *host, const char *service, const char *transport,struct sockaddr_in *remote_server)
{
	struct hostent *phe;
	struct servent *pse;
	struct protoent *ppe;
	int sock, type;

	//reset socket address structure.
	memset(remote_server,0,sizeof(struct sockaddr_in));

	remote_server->sin_family = AF_INET;

	//map service name to port number
	if((pse = getservbyname(service, transport)))
		remote_server->sin_port = pse->s_port;
	else if((remote_server->sin_port = htons((u_short)atoi(service))) == 0)
		//perror("Cant get \"%s\" service entry\n", service);
		perror("Cant get service entry\n");

	//map host name to IP address allowing dotted decimal
	if((phe = gethostbyname(host)))
		memcpy(&((*remote_server).sin_addr), phe->h_addr, phe->h_length);
	else if ((remote_server->sin_addr.s_addr = inet_addr(host) == INADDR_NONE))
		//perror("Cant get \"%s\" host entry\n", host);
		perror("Cant get host entry\n");

	bzero(remote_server->sin_zero, 8);

	//map transport protocol name to protocol number
	ppe = getprotobyname(transport);
	if(ppe == NULL)
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

	return sock;
}


int main(int argc, char **argv)
{
	int sock;
	socklen_t addr_size;
	int len, bytes_read;
	char output[BUFFER];
	char input[BUFFER];
	struct sockaddr_in remote_server;
	
	if(argc < 3)
	{
		printf("Usage: ./client <server IP> <server port> \n");
		exit(0);
	}
	printf("\n************************************************************************\n");
	printf("UDP test client:  Connects to the concurrent preforked UDP server\n");
	printf("receives number of preforked child processes at the server and prints it.\n");
	printf("**************************************************************************\n\n");
		
	//Create UDP socket endpoint
	sock = createsock(argv[1], argv[2], "udp",&remote_server);
	if (sock < 0){
		printf("Socket creation failed\n");
		exit(-1);
	}
	
	// send a datagram
	memset(input, '1', BUFFER);
	if (0 > sendto(sock, input, strlen(input), 0,(struct sockaddr *)&remote_server, sizeof(remote_server)))
	{
		perror("sendto error");
		close(sock);
		exit(-1);
	}

	printf("Request sent......waiting for server's response\n");
	memset(output, 0, BUFFER);
	addr_size = sizeof(remote_server);

	// receive server's response and print.
	len = recvfrom(sock, output, BUFFER, 0,(struct sockaddr *)&remote_server, &addr_size);
	if (len < 0)
	{
		perror("recv error..");
		exit(-1);
	}
	output[len] = '\0';
	printf("%s", output);
	close(sock);
}

