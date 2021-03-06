// CMPE_207 Assignment 2
// UDP Client
// Author - Team #2
// This is a UDP test client program. Client connects
// the server. Requests a specific file (1K or 2K file)
// from the server. Save the file on the local disk.
// To compile: gcc udp_test_client.c -o client
// To run: ./client <server_address> <server_port> <file_name>


#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<sys/stat.h>
#include<fcntl.h>
#include<netinet/in.h>
#include<string.h>
#include<ctype.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/wait.h>

#define ERROR -1
#define BUFFER 1024

int createsock(const char *host, const char *service, const char *transport, struct sockaddr_in *remote_server)
{
	struct hostent *phe;
	struct servent *pse;
	struct protoent *ppe;
	//struct sockaddr_in remote_server;
	int sock, type;

	//reset socket address structure.
	memset(remote_server,0,sizeof(struct sockaddr_in));

	remote_server->sin_family = AF_INET;

	//map service name to port number
	if(pse = getservbyname(service, transport))
		remote_server->sin_port = pse->s_port;
	else if((remote_server->sin_port = htons((u_short)atoi(service))) == 0)
		//perror("Cant get \"%s\" service entry\n", service);
		perror("Cant get service entry\n");

	//map host name to IP address allowing dotted decimal
	if(phe = gethostbyname(host))
		memcpy(&((*remote_server).sin_addr), phe->h_addr, phe->h_length);
	else if ((remote_server->sin_addr.s_addr = inet_addr(host) == INADDR_NONE))
		//perror("Cant get \"%s\" host entry\n", host);
		perror("Cant get host entry\n");

	bzero(remote_server->sin_zero, 8);

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

	return sock;
}


int main(int argc, char **argv)
{
	int sock;
	int c, fd;
	char input[BUFFER];
	char output[BUFFER];
	int len; 
	struct sockaddr_in remote_server;
	socklen_t addr_len;
	if(argc < 4)
	{
		printf("Usage: ./client <server IP> <server port> <file_name> \n");
		printf("Server hosts \"one_k_file.txt\" & \"two_k_file.txt\". \n");
		printf("User must request one of these two files for <file_name> argument\n");
		exit(0);
	} else
	{
		printf("\n***********************************************************************\n");
		printf("UDP test client program. Requesting %s file from server.\n",argv[3]);
		printf("File will be stored in the client local current directory as \"test_file.txt\"\n");
		printf("\n***********************************************************************\n");
	
		sock = createsock(argv[1], argv[2], "udp", &remote_server);
		if (sock < 0){
			printf("Failed to create socket.\n");
			exit(-1);
		}

		// connection successfuly established
		printf("Sending datagram requesting file transfer..\n");

		memset(input,0,BUFFER);
		strcpy(input, argv[3]);
		//input[strlen(input)] = '\n';

		// send file number
		if (0 > sendto(sock, input, strlen(input), 0, (struct sockaddr *)&remote_server,sizeof(remote_server)))
		{
			perror("send error");
			close(sock);
			exit(1);
		}

		// open file for writing
		fd = open("test_file.txt", O_CREAT|O_TRUNC|O_WRONLY, S_IRUSR|S_IWUSR|S_IRGRP|S_IROTH);
		if(fd < 0){
			printf("Failed to create file\n");
			exit(1);
		}

		addr_len = sizeof(remote_server);
		//receive file and write it to the file 
		while(1)
		{
			
			memset(output, 0, BUFFER);
			len = recvfrom(sock, output, BUFFER, 0, (struct sockaddr *)&remote_server, &addr_len);
			//printf("recvfrom len = %d\n", len);
			if (len < 0)
			{
				perror("recv error");
				close(fd);
				close(sock);
				exit(1);
			}
			if (len == 0)
				break;

			if (output[0] == '') {
				if (len > 1) {
					output[0]=' ';
					output[len]='\0';
					printf("%s\n", output);
					close(sock);
					exit(-1);
				}
				//close(sock);
				//exit(-1);
				break;
			}

			//write to file
			len = write(fd, output, strlen(output));
			//printf("write len = %d\n", len);
			if(len < 0) {
					printf("File write error..exiting..\n");
					close(fd);
					close(sock);
					exit(1);
			}
		}

		close(fd);
		//close client socket
		printf("File transfer successful. Closing the socket.Exiting the child.\n");
		close(sock);
	}
}

