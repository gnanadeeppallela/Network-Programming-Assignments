// CMPE_207 Assignment_2
// Author - Team #2
// UDP server program for threading

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
#include<pthread.h>

#define ERROR 0
#define MAX_DATA 1024


//Structure to create an instance of thread
struct UDP_thread
{
	socklen_t len;
	struct sockaddr_in from_client;
	char buffer[MAX_DATA];
	int socket_new;

};

//Handle request function to handle the requests from the thread creation
//Performs the file transfer of the requested file from server to client
void* handle_req(void *UDP_thread)
{
	struct UDP_thread *UDP_th = UDP_thread;
	char child_line[MAX_DATA];
    char data[MAX_DATA];
	int sock,result,fd;

	//Copy the file name received from the client by the server to child_line buffer
	strncpy(child_line,UDP_th->buffer,strlen(UDP_th->buffer));

	printf("File request from client: %s \n", child_line);
	
	//Compare the file names 
	result = strcmp(child_line, "one_k_file.txt");
	if(!result)	
	{			
		//line matches "test_file_1.txt" 
		printf("File name matched: one_k_file.txt\n");
		fd = open("one_k_file.txt", O_RDONLY);
	} 
	else 
	{

		result = strcmp(child_line, "two_k_file.txt");
		if(!result)
		{ 	
			//line matches "test_file_2.txt"
			printf("File name matched: two_k_file.txt");
			fd = open("two_k_file.txt", O_RDONLY);
		}
		else
		{
			printf("%s ==> File does not exist on the server. \n",child_line);
			strcpy(data, "Error: Requested File does not exist on the server.\n");
			if(0 > sendto(UDP_th->socket_new, data, strlen(data), 0, (struct sockaddr *)&UDP_th->from_client, UDP_th->len))
			{
				perror("sendto error");
				exit(-1);
			}
			printf("Error msg sent. Thread exiting...\n");
			exit(0);
		}
	} 

	//Check error on file descriptor fd
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
		if(result == 0)
		{
			// Ctrl-D to indicate EOF
			data[0]=''; 
			done=1;
			result=1;
		}

		/*send MAX_DATA bytes.*/
		result = sendto(UDP_th->socket_new, data, result, 0, (struct sockaddr *)&UDP_th->from_client, UDP_th->len);
		
		if (0 > result)
		{
			perror("Sendto Error");
			break;
		}
	}//end of while

	//close file descriptor
	close(fd);

	if(done == 1)//if(result == 0)
		printf("File transfer done.Thread Exiting..\n");
			
	exit(0);
}


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
int readline(int fd, char *buf, int maxlen)
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
	int n,result, len;
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
	struct UDP_thread u;//Create an instance to hold data for thread

	//Check for the arguments passed to program
	if(argc != 3)
	{
		printf("Usage: ./server <port>\n");
		exit(0);
	}

	u.len = sizeof(struct sockaddr_in);

	//Create the threads for the argument passed in comman line
	int num_of_threads = atoi(argv[2]);

	result = passivesock(argv[1], "udp", &u.socket_new);

	if (result == -1)
		exit(-1);

	printf("\n**************************************************************\n");
	printf("Concurrent UDP File Server Program: listening on socket %s..\n", argv[1]);
	printf("**************************************************************\n");
	printf("Threads created based on the command line argument: \n\n"); 
	while (1)
	{  

		socklen_t client_t=sizeof(struct sockaddr_in);
		memset(&client, 0, sizeof(client));
		memset(parent_line, 0, MAX_DATA);
		memset(child_line,0, MAX_DATA);

		//parent: wait for incoming client datagram
		bytes_received = recvfrom(u.socket_new, u.buffer, MAX_DATA, 0, (struct sockaddr *)&u.from_client, &u.len);
		if(0 > bytes_received)
		{
			perror("recvfrom error");
			close(u.socket_new);
			exit(-1);
		}
		//Create thread ID for the number of threads specified (num_of_threads)
		pthread_t thread_id[num_of_threads];
		int return_c = 0;
		//Create the threads to handle the function handle_request
		for (int i = 0 ; i < num_of_threads; i++)
		{

			return_c = pthread_create(&thread_id[i],NULL,handle_req,&u);
			if (return_c != 0){
				sleep(0.05);
				perror("Error creating thread\n");
				exit(-1);
			
			}
			else
			{
				// we are in thread
				printf("Thread %d created for the client \n", i+1);
			}
		}
		//Wait for all the threads to complete
		for (int i = 0 ; i < num_of_threads; i++)
		{

			pthread_join(thread_id[i],NULL);
		}
	}

		

	//close the server socket
	close(u.socket_new);
	printf("Main process exiting\n");
}

