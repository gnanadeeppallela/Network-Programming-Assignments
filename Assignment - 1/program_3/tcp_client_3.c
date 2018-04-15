// CMPE_297 Assignment_1 Program #3
// TCP Client
// Author - Team #2
// This TCP client program connects to the server
// sends an integer value. Receives an integer value
// back from the server which is one decremented from
// the value sent by the client. Client then prints 
// the value to its output screen.
// To compile: gcc tcp_client_3.c -o client
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

#define ERROR -1
#define BUFFER 1024

// function to convert the given
// string to lower case.
char *conv_to_lower(char str[],char *res)
{
    int i = 0;
    while(str[i])
    {
        res[i] = tolower(str[i]);
        i++;
    }
    return res;
}

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
    char input[BUFFER];
    char output[BUFFER];
    char *res_str;
    int len;
    int res;

    if(argc < 3)
    {
        printf("Usage: ./client <server IP> <server port> \n");
        exit(0);
    } else
    {

        printf("\n>> Send an integer. Server responds with a decremented value.\n");
        printf(">> Enter quit when you wish to exit the client.\n\n");

	sock = connectsock(argv[1], argv[2], "tcp");

        strcpy(input, "1");
        while(1)
        {
            // Take user input.
            memset(input, 0, BUFFER);
            printf("User input:");
            fgets(input, BUFFER, stdin);
            len = strlen(input);
            input[len-1] = '\0';

            res_str = (char *) malloc(len);
            // Check if user wishes to exit the client
            if((res =  strcmp(conv_to_lower(input,res_str), "quit")) == 0)
            {
                free(res_str);
                break;
            }

            free(res_str);
            // Send user input to the server.
            send(sock, input, strlen(input), 0);

            memset(output, 0, BUFFER);

            // receive server's response and print.
            len = recv(sock, output, BUFFER, 0);
            output[len] = '\0';
            printf("Server output:%s\n\n", output);
        }

        //close client socket.
        close(sock);
    }
}



