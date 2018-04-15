// CMPE_207 Assignment_2
// Author - Team #2
// UDP server program
// This is a concurrent UDP server program. Server 
// listens for active clients. It creates new child
// processes to handle each new client connection.
// child exits once client is served. Parent process
// waits for new connections in an infinite loop.
// To compile: gcc udp_server.c -o server
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
#define NUM_CHILDREN 5

int passivesock(const char *service, const char *transport, int *sock)
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
	int bytes_received = 0;
	int n, sock, result, len;
	int clientfd;
	int num;
	pid_t cpid;
	int data_len;
	char data[MAX_DATA];
	char parent_line[MAX_DATA];
	char child_line[MAX_DATA];
	char cpid_s[32];
	char err_msg[32] = "";
	int fd;

	if(argc != 2)
	{
		printf("Usage: ./server <port>\n");
		exit(0);
	}

	result = passivesock(argv[1], "udp", &sock);

	if (result == -1)
		exit(-1);

	printf("\n**************************************************************\n");
	printf("Concurrent UDP File Server Program: listening on socket %s..\n", argv[1]);
	printf("**************************************************************\n");
	printf("Child processes forked on demand.One per client connection: \n\n"); 
	while (1)
	{  
		//socklen_t sockaddr_len = sizeof(struct sockaddr_in);
		socklen_t client_t=sizeof(struct sockaddr_in);
		memset(&client, 0, sizeof(client));
		memset(parent_line, 0, MAX_DATA);
		memset(child_line,0, MAX_DATA);

		//parent: wait for incoming client datagram
		bytes_received = recvfrom(sock, parent_line, MAX_DATA, 0, (struct sockaddr *)&client, &client_t);
		if(0 > bytes_received)
		{
			perror("recvfrom error");
			close(sock);
			exit(-1);
		}
		
		strncpy(child_line,parent_line,strlen(parent_line));

		cpid = fork();
		if (cpid != 0){
			sleep(0.05);
			if(cpid == -1){
				perror("fork error");
				exit(-1);
			}
		}
		else
		{
			// we are in child
			printf("forking new child process %d to handle the client %s:%d \n",getpid(),inet_ntoa(client.sin_addr),client.sin_port);

			printf("File request from client: %s \n", child_line);
			//n = strlen(child_line);
			//child_line[n-1] = '\0';
			result = strcmp(child_line, "one_k_file.txt");
			if(!result)	{			//line matches "test_file_1.txt" 

				printf("File name matched: one_k_file.txt\n");
				fd = open("one_k_file.txt", O_RDONLY);
			} else {

				result = strcmp(child_line, "two_k_file.txt");
				if(!result){ 		//line matches "test_file_2.txt"

					printf("File name matched: two_k_file.txt");
					fd = open("two_k_file.txt", O_RDONLY);
				}else{
					printf("%s ==> File does not exist on the server. \n",child_line);
					strcpy(data, "Error: Requested File does not exist on the server.\n");
					if(0 > sendto(sock, data, strlen(data), 0, (struct sockaddr *)&client, client_t))
					{
						perror("sendto error");
						exit(-1);
					}
					printf("%d: Error msg sent. child exiting...\n", getpid());
					exit(0);
				}
			} 

			if (fd < 0)
			{
				printf("File open Error..\n");
				exit(-1);
			}

			// set pointer to beginning of the file.
			lseek(fd, 0, SEEK_SET);
			int done = 0;
			while(!done)
			{
				//read MAX_DATA data bytes from the file
				result = readline(fd, data, MAX_DATA);
				//printf("File read result = %d bytes\n", result);
				if(result == 0)
				{
					// Ctrl-D to indicate EOF
					data[0]=''; 
					done=1;
					result=1;
				}

				/*send MAX_DATA bytes.*/
				result = sendto(sock, data, result, 0, (struct sockaddr *)&client, client_t);
				//printf("send result = %d bytes\n", result);
				if (0 > result)
				{
					perror("Sendto Error");
					break;
				}
			}//end inner while

			//close file
			close(fd);

			if(done == 1)//if(result == 0)
				printf("%d: File transfer done.Child Exiting..\n",getpid());
			
			exit(0);
		}//end else
	} //end while(1)

	//close the server socket
	close(sock);
	printf("Main process exiting\n");
}
