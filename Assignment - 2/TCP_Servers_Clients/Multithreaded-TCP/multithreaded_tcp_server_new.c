// CMPE_207 Assignment_2
// Author - Team #2
// TCP server program
// This is a concurrent TCP server program. Server 
// listens for active clients. 
// To compile: gcc tcp_server -o server
// To run: ./server port_number

#include<stdio.h>
#include<stdlib.h>
#include<unistd.h>
#include<sys/socket.h>
#include<sys/types.h>
#include<netinet/in.h>
#include<string.h>
#include<arpa/inet.h>
#include<netdb.h>
#include<sys/wait.h>
#include<sys/stat.h>
#include<fcntl.h>

#define ERROR 0
#define MAX_DATA 1024


//Handler function by the threads created
void* file_transfer(void* new_sock)
{
	int n,result;
	int data_len;
	char data[MAX_DATA];
	char line[MAX_DATA];
	char err_msg[32] = "";
	int fd;
	int clientfd=*(int*)new_sock;

	memset(line, 0, MAX_DATA);
	/* Read a line from the client socket ... */
	n = readline(clientfd, line, MAX_DATA);
	if (n <= 0) 
	{
		printf("Error reading from client");
		exit(-1);
	}

	printf("File request from client: %s\n", line);
	line[n-1] = '\0';
	result = strcmp(line, "one_k_file.txt");
	if(!result)	
	{	
		//line matches "test_file_1.txt" 
		fd = open("one_k_file.txt", O_RDONLY);
	} 
	else 
	{

		result = strcmp(line, "two_k_file.txt");
		if(!result)
		{ 		
			//line matches "test_file_2.txt"
			fd = open("two_k_file.txt", O_RDONLY);
		}
		else
		{
			printf("File does not exist. Error msg sent back...\n");
			strcpy(data, "Error: Requested File does not exist on the server.\n");
			if(0 > send(clientfd, data, strlen(data), 0))
			{
				perror("send error");
				exit(-1);
			}
			printf("Closing client connection. Thread exits.\n");
			close(clientfd);
			exit(0);
		}
	} 

	if (fd < 0)
	{
		printf("File open Error..\n");
		close(clientfd);
		exit(-1);
	}

	// set pointer to beginning of the file.
	lseek(fd, 0, SEEK_SET);
	while(1)
	{
		//read MAX_DATA data bytes from the file
		result = readline(fd, data, MAX_DATA);
		//printf("File read result = %d bytes\n", result);
		if(result == 0)
			break;

		/*send MAX_DATA bytes.*/
		result = send(clientfd, data, result, 0);
		//printf("send result = %d bytes\n", result);
		if (0 > result)
		{
			perror("Send Error");
			break;
		}
	}//end inner while

	//close file descriptor
	close(fd);

	if(result == 0)
	{
		printf("File transfer done.\n");
		printf("\n%d ==>  Closing client connection. Child exiting..\n\n", getpid());
		close(clientfd);
		exit(0);
	}//end else

	


}

int passivesock(const char *service, const char *transport, int qlen, int *sock)
{
	struct servent *pse;
	struct protoent *ppe;
	struct sockaddr_in server;
	int res = 1;
	int type;

	//reset socket address structure.
	memset(&server,0,sizeof(server));

	server.sin_family = AF_INET;
	server.sin_addr.s_addr = INADDR_ANY;

	//map service name to port number
	if(pse = getservbyname(service, transport))
		server.sin_port = htons(ntohs((u_short)pse->s_port));
	else if((server.sin_port = htons((u_short)atoi(service))) == 0)
		//perror("Cant get \"%s\" service entry\n", service);
		perror("Cant get service entry\n");

	bzero(&server.sin_zero, 8);

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
	*sock = socket(PF_INET, type, ppe->p_proto);
	if (*sock < 0){
		res = -1;
		perror("Cant create socket\n");
	}

	//Bind the socket
	if(bind(*sock, (struct sockaddr *)&server, sizeof(server)) < 0)
	{
		res = -1;
		perror("Cant bind to specified port\n");
	}

	//listen for incoming client connections
	if(type == SOCK_STREAM && (listen(*sock, qlen) < 0))
	{	
		res = -1;
		perror("Listen error\n");
	}

	return res;
}

/**
 * Simple utility function that reads a line from a file descriptor fd, 
 * up to maxlen bytes -- ripped from Unix Network Programming, Stevens.
 */
	int
readline(int fd, char *buf, int maxlen)
{
	int n, rc;
	char c;

	for (n = 1; n < maxlen; n++) {
		rc = read(fd, &c, 1);

		if (rc == 1) {
			*buf++ = c;
			if (c == '\n')
				break;
			
		} else if (rc == 0) {
			if (n == 1)
				return 0; // EOF, no data read
			else
				break; // EOF, read some data
		} else
			return -1; // error
	}

	*buf = '\0'; // null-terminate
	return n;
}


int main(int argc, char **argv)
{
	struct sockaddr_in client;  //client data structure

	int n, sock, result, len,*new_sock;
	int clientfd;
	int num;
	int data_len;
	char data[MAX_DATA];
	char line[MAX_DATA];
	char err_msg[32] = "";
	int fd;

	if(argc != 2)
	{
		printf("Usage: ./server <port>\n");
		exit(0);
	}

	result = passivesock(argv[1], "tcp", MAX_DATA, &sock);

	if (result == -1)
		exit(-1);

	printf("\n**************************************************************\n");
	printf("Concurrent TCP File Server Program: listening on socket %s..\n", argv[1]);
	printf("**************************************************************\n");
	printf("Threads created based on the number of connections recieved: \n\n"); 
	while (1)
	{  
		socklen_t sockaddr_len = sizeof(struct sockaddr_in);
		socklen_t client_t=sizeof(struct sockaddr_in);
		memset(&client, 0, sizeof(client));
		clientfd = accept(sock, (struct sockaddr*)&client, &client_t);
		if ( clientfd < 0 ) {
			perror("Accept Error");
			exit(-1);
		}
		printf("New connection request from %s:%d  ==> \n",inet_ntoa(client.sin_addr), ntohs(client.sin_port));
		//Create a Pthread with a thread_id
		pthread_t thread_id;
		new_sock = malloc(1);
		*new_sock = (int)clientfd;

		//Check whether the created thread contains error
		if (pthread_create(&thread_id,NULL,file_transfer,new_sock) < 0){
			sleep(0.05);
			close(clientfd);
			perror("thread error");
			exit(-1);
			}
		
		else
		{
			// we are in thread
			printf("Child thread created\n");
		}
	}

		//close the server socket
		close(sock);
		printf("Main process exiting\n");
}


