// CMPE_207 Assignment_2
// Author - Team #2
// UDP server program
// This is a concurrent UDP server program. Server 
// listens for active clients. server creates few child
// processes in advance to handle incoming connections.
// child process handle client connections and returns 
// to the process pool once done serving the client.
// To compile: gcc udp_server -o server
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

#define ERROR 0
#define MAX_CLIENTS 5
#define MAX_DATA 1024
#define NUM_CHILDREN 7

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
	if((pse = getservbyname(service, transport)))
		server.sin_port = htons(ntohs((u_short)pse->s_port));
	else if((server.sin_port = htons((u_short)atoi(service))) == 0)
		//perror("Cant get \"%s\" service entry\n", service);
		perror("Cant get service entry\n");

	bzero(&server.sin_zero, 8);

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

int main(int argc, char **argv)
{
	struct sockaddr_in client;  //client data structure

	int n, sock, result;
	pid_t cpid;
	int clientfd, bytes_read;
	socklen_t sockaddr_len = sizeof(struct sockaddr_in);
	socklen_t client_t;
	char line[MAX_DATA];

	if(argc != 2)
	{
		printf("Usage: ./server <port>\n");
		exit(0);
	}

	//create server socket and bind it to an address.
	result = passivesock(argv[1], "udp", MAX_CLIENTS, &sock);

	if (result == -1)
		exit(-1);

	printf("\nConcurrent preforking UDP Server Program \n");
	printf("**********************************************\n");
	printf("Number of child processes preforked: %d\n\n", NUM_CHILDREN); 

	/* Fork NUM_CHILDREN child processes to handle client requests. */
	for (int i = 0; i < NUM_CHILDREN; i++) 
	{	
		cpid = fork();
		if (cpid == -1) {
			perror("Couldn't fork");
			exit(-1);
		}

		if (cpid == 0) 
		{ // We're in the child ...
			printf("Child: %d forked....\n", getpid());

			for (;;) 
			{ // Run forever ...

				// wait for datagrams				
				client_t = sizeof(struct sockaddr*);
				bytes_read = recvfrom(sock,line, MAX_DATA, 0, (struct sockaddr *)&client, &client_t);
				if(bytes_read > 0)
				{
					printf("Child %d: Request received from client: %s:%d\n", getpid(),inet_ntoa(client.sin_addr),client.sin_port);
					memset(line, 0, MAX_DATA);
					//send back response to the client
					sprintf(line, "\nChild pid %d: Number of preforked processes at server is %d\n",getpid(), NUM_CHILDREN);
					if(0 > sendto(sock,line, strlen(line), 0, (struct sockaddr *)&client, client_t))
					{
						perror("sendto error");
						exit(-1);
					}
					printf("Child %d: response sent to %s:%d\n",getpid(),inet_ntoa(client.sin_addr),client.sin_port);
				}
				else
					perror("recvfrom error");
			}
		}
	}

	/*parent: Sit back and wait for all child processes to exit */
	while (waitpid(-1, NULL, 0) > 0);

	//close the server socket
	close(sock);
	printf("Main process exiting\n");
}
