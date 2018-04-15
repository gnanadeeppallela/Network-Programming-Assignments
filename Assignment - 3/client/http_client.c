// CMPE_297 Assignment_3
// WEB Client
// Author - Team #2
// This WEB client program connects to the WEB server
// sends a HTTP1.0 GET request for a text file. If the
// requested file exists on the server, Server responds
// with HTTP1.0 response header and the requested file.
// Other wise client receives the error message.
// To compile: gcc http_client.c -o client
// To run: ./client <server_address/file_name> <server_port>


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
#include<time.h>

#define BUF_SIZE 1024 

int connectsock(const char *host, const char *service, const char *transport); 
void split_url(char* url, char** host); 
int get_request(char * url, char * port);
int isValidIP(char * ip);
int parseHeader(char * header);
char* splitKeyValue(char * line, int index);
void openFile();
long GetFileSize(const char* filename);

FILE * fileptr;
//char keys[][25] = {"Date: ", "Hostname: ", "Location: ", "Content-Type: "};
char keys[][25] = {"Content-Type: ", "Content-Length: "};
char status[4] = {0, 0, 0, 0};
char contentFileType[100];
char path[1000];
char getrequest[1024];


// Main Function
int main(int argc, char **argv)
{
	int sock, ret;
	char* host = NULL;
	char * url, * temp;
	char * fileName;
	char status_ok[] = "OK";
	char buffer[BUF_SIZE]; 
	char http_not_found[] = "HTTP/1.0 404 Not Found";
	char http_ok[] = "HTTP/1.0 200 OK";
	char location[] = "Location: ";
	char contentType[] = "Content-Type: ";
	int sPos, ePos;

	clock_t start,stop;
	double result1;

	if(argc < 3)
	{
		printf("Usage: ./client <URL> <server port> \n");
		printf("Example Usage: ./client 127.0.0.1/one_k_file.txt 4000 \n");
		exit(1);
	} else
	{
		printf("\n>> TCP Web Test Client. This program send HTTP1.0 GET request for a\n");
		printf(">> file to the TCP Web server. File is saved in the current directory. \n\n");

		url = argv[1];

		//split argv[1] into server address and filename arguments
		split_url(argv[1], &host);

		printf("CLIENT: Sending getrequest = %s\n", getrequest);

		//create tcp socket end point
		sock = connectsock(host, argv[2], "tcp");

		start=clock();
		// Send HTTP 1.0 GET request over TCP connection
		write(sock, getrequest, strlen(getrequest));

		// handle HTTP response status from the server
		memset(&buffer, 0, sizeof(buffer));
		ret = recv(sock, buffer, BUF_SIZE, 0);  
		if (ret < 0) {  
			printf("Error receiving HTTP status!\n");    
		} else {
			printf("RESPONSE: HTTP STATUS FROM SERVER (STATUS SIZE: %ld bytes.) :\n", (strlen(buffer)-1));
			printf("%s\n", buffer);
			if ((temp = strstr(buffer, http_ok)) != NULL) {
				send(sock, status_ok, strlen(status_ok), 0);
			} else {
				close(sock);
				return 0;
			}
		}

		// handle HTTP response header
		memset(&buffer, 0, sizeof(buffer)); 
		ret = recv(sock, buffer, BUF_SIZE, 0);  
		if (ret < 0) {  
			printf("Error receiving HTTP header!\n");    
		} else {

			printf("RESPONSE: HTTP HEADER FROM SERVER (HEADER SIZE: %ld bytes.) :\n", (strlen(buffer)-1));
			printf("%s\n", buffer);
			if (parseHeader(buffer) == 0) {
				send(sock, status_ok, strlen(status_ok), 0);
			} else {
				printf("Error in HTTP header!\n");
				close(sock);
				return 0;
			}
		} 

		// Open file for writing
		fileptr = fopen(path, "w");
		if (fileptr == NULL) {
			printf("Error opening file!\n");
			close(sock);
			return 0;
		}

		// Handle file transfer from the server
		memset(&buffer, 0, sizeof(buffer));
		while (1) {
			int recvBytes = recv(sock, buffer, BUF_SIZE-1, 0);
			if (recvBytes <= 0)
				break;

			printf("Recieved %d bytes\n", recvBytes);

			if ((strstr(contentFileType, "application/html")) != NULL) {
				printf("Wrote %d bytes\n", fprintf(fileptr, "%s", buffer));
			} else {
				fwrite(&buffer, recvBytes, 1, fileptr);
			}
			memset(&buffer, 0, sizeof(buffer));
		}

		stop=clock();

		printf("File Transfer successful. File stored locally on the disk. \n");
		fclose(fileptr);
		close(sock);

		//openFile();
		printf("RESPONSE: FILE SIZE: %ld bytes.\n", GetFileSize(path));
		result1=((double)(stop-start))/CLOCKS_PER_SEC;
		printf("Time required to fetch the file is %f seconds.\n",result1);
		return 0;
	}
}


// Function to Handle TCP connection request
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


// Function to split url into host and filenames
void split_url(char* url, char** host)
{
	char* ptr;
	char* temp_host, *host1;

	if (isValidIP(url)) { //when an IP address is given
		sprintf(getrequest, "GET / HTTP/1.0\nHOST: %s\n\n", url);
	} else { //when a host name is given
		if ((ptr = strstr(url, "/")) == NULL) {
			//when hostname does not contain a slash
			sprintf(getrequest, "GET /  HTTP/1.0\nHOST: %s\n\n", url);

		} else {
			//when hostname contains a slash, it is a path to file
			strcpy(path, ptr);
			*host = strtok(url, "/");
			sprintf(getrequest, "GET %s HTTP/1.0\nHOST: %s\n\n", path, url);
		}
	} 

	ptr = strtok(path, "/");
	strcpy(path, ptr);
}

// Function to validate given IP address
int isValidIP(char * ip) {
	struct sockaddr_in addr;
	int valid = inet_pton(AF_INET, ip, &(addr.sin_addr));
	return valid != 0;
}

// Functio to parse HTTP header
int parseHeader(char * header) {
	//"Date: %sHostname: %s:%d\nLocation: %s\nContent-Type: %s\n\n"
	char * line, * key, * value;
	char temp[100];
	int i = 0;
	line = strtok(header, "\n");
	while (line != NULL) {
		//printf("%s\n", line);
		strcpy(temp, line);
		value = splitKeyValue(line, i);  
		if (i == 3) {   
			strcpy(contentFileType, value);
		}
		//printf("value=%s\n", value);
		line = strtok(NULL, "\n");
		i++; 
	}
	for (i = 0; i < 2; i++) {
		if (status[i] == 0) return 1;
		//printf("status[%d]=%d\n", i, status[i]);
	}
	return 0;
}

// Helper function
char * splitKeyValue(char * line, int index) {
	char * temp;
	if ((temp = strstr(line, keys[index])) != NULL) {
		temp = temp + strlen(keys[index]);
		status[index] = 1;
	}
	return temp;
}


// Function to open the transfered file
void openFile() {
	char * temp;
	char command[100];
	char fileName[1000];
	strcpy(fileName, path);
	//printf("File Name: %s\n", fileName);
	//printf("Content Type: %s\n", contentFileType);
	if ((temp = strstr(contentFileType, "text/html")) != NULL) {
		if ((temp = strstr(fileName, ".txt")) != NULL) {
			sprintf(command, "gedit %s", fileName);
		} else {
			sprintf(command, "firefox %s", fileName);
		}
		system(command);
	} else if ((temp = strstr(contentFileType, "application/pdf")) != NULL) {
		sprintf(command, "acroread %s", fileName);
		system(command);
	} else {
		printf("The filetype %s is not supported. Failed to open %s!\n", contentFileType, fileName);
	}
}


long GetFileSize(const char* filename)
{
	long size;
	FILE *f;
	//printf("getfilesize: filename = %s\n", filename); 
	f = fopen(filename, "rb");
	if (f == NULL){
		return -1;
		printf("error...\n");
	}

	fseek(f, 0, SEEK_END);
	size = ftell(f);
	fclose(f);
	//printf("Size in getfilesize: %ld\n", size);
	return size;

}

