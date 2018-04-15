/* Client - Multithreaded server
Assignment-02 Group-02
Step 1: Creating a socket
Step 2: Get portnumber and IP address
step 3: Connect with the socket
step 4: Read the values
*/


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
	int c, fd;
	char input[BUFFER];
	char output[BUFFER];
	int len; 

	if(argc < 4)
	{
		printf("Usage: ./client <server IP> <server port> <file_name> \n");
		printf("Server hosts \"one_k_file.txt\" & \"two_k_file.txt\". \n");
		printf("User must request one of these two files for <file_name> argument\n");
		exit(0);
	} else
	{
		printf("\n***********************************************************************\n");
		printf("TCP test client program. Requesting %s file from server.\n",argv[3]);
		printf("File will be stored in the client local current directory as \"test_file.txt\"\n");
		printf("\n***********************************************************************\n");
	
		sock = connectsock(argv[1], argv[2], "tcp");
		if (sock < 0){
			printf("connection Failed.\n");
			exit(-1);
		}

		// connection successfuly established
		printf("Connection successful.Requesting file transfer..\n");

		memset(input,0,BUFFER);
		strcpy(input, argv[3]);
		input[strlen(input)] = '\n';

		// send file number
		if (0 > send(sock, input, strlen(input), 0))
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

		//receive file and write it to the file 
		while(1)
		{
			memset(output, 0, BUFFER);
			len = recv(sock, output, BUFFER, 0);
			if (len < 0)
			{
				perror("recv error");
				close(fd);
				close(sock);
				exit(1);
			}
			if (len == 0)
				break;

			//write to file
			len = write(fd, output, strlen(output));
			if(len < 0) {
					printf("File write error..exiting..\n");
					close(fd);
					close(sock);
					exit(1);
			}
		}

		close(fd);
		//close client socket
		printf("File transfer successful. Closing the socket.\n");
		close(sock);
	}
}

