// CMPE_207 Assignment_1 Program #3
// Author - Team #2
// TCP server program
// This server program listens for active clients.
// Receives an integer from the client. Decrements
// the int by one and sends it back to the client.
// To compile: gcc tcp_server_3 -o server
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

#define ERROR 0
#define MAX_CLIENTS 5
#define MAX_DATA 1024

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

int validate_data(char data[], int len)
{
	int result = 1;
	int in_value;

	if(data[0] == '-')
		in_value = 1;
	else
		in_value = 0;

	for(int i = in_value; i < len;i++)
	{
		if(data[i] >= 48 && data[i] <=57)
			continue;
		else{
			result = 0;
			break;
		}
	}
	return result;		
}


int main(int argc, char **argv)
{
	struct sockaddr_in client;  //client data structure

	int sock, result;
    	int new_cli_sock;
    	socklen_t sockaddr_len = sizeof(struct sockaddr_in);
    	int data_len;
    	char data[MAX_DATA];
    	int num;

	if(argc != 2)
    	{
        	printf("Usage: ./server <port>\n");
        	exit(0);
    	}

	result = passivesock(argv[1], "tcp", MAX_CLIENTS, &sock);

	if (result == -1)
		exit(-1);
	
    	printf("\n\nTCP Server listening for active connections...\n");

	//Accept client connection
	while(1)
	{
        	if((new_cli_sock = accept(sock, (struct sockaddr *)&client, &sockaddr_len)) == ERROR)
	        {
        	    perror("Accept Error");
	            exit(-1);
        	}

	        printf("New client connected from port no: %d & IP: %s\n", ntohs(client.sin_port), inet_ntoa(client.sin_addr));
        	data_len = 1;

	        while(data_len)
        	{
	            memset(data, 0, MAX_DATA);
        	    data_len = recv(new_cli_sock, data, MAX_DATA, 0);
	            if(data_len)
        	    {
                	// validate the received data
	                printf("\nReceived: %s\n", data);
			result = validate_data(data, strlen(data));
			if(result)
			{
				num = atoi(data);

                    		// decrement the integer data by 1.
                    		num = num - 1;
                    		snprintf(data,MAX_DATA, "%d",num);
			} else
			{
                    		strcpy(data, "Sorry, no valid integer sent.");
                    		data_len = strlen(data);
			}

                	// send response to the client
                	printf("Sent: %s\n",data); 
                	send(new_cli_sock, data, strlen(data), 0);
           	    }
        	}

        	printf("Client disconnected\n");

        	// close client connection
        	close(new_cli_sock);
	}

    	//close the server socket
    	close(sock);
}
