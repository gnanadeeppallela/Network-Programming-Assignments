// CMPE_297 Assignment_3
// WEB Server
// Author - Team #2
// This WEB server program connects with TCP clients
// accepts HTTP 1.0 GET requests from client. processes 
// it and searches for the requested file in the entire
// system. If found HTTP response status + HEADER + file
// is sent to the client. If file is not found then an 
// error is sent back and closes the connection. waits for
// the next connection.
// To compile: gcc http_server.c -o server
// To run: ./server <IP address> <server_port>

#include<stdio.h>
#include<stdlib.h>
#include<sys/types.h>
#include<sys/socket.h>
#include<string.h>
#include<netinet/in.h>
#include<time.h>
#include<dirent.h>
#include<netdb.h>
#include<unistd.h>
#include<arpa/inet.h>
#include <dirent.h> 
#include <errno.h>
#include <sys/stat.h>


#define BUF_SIZE 1024
#define CLADDR_LEN 100

int createSocket(char * host, int port);
int listenForRequest(int sockfd);
char * getFileType(char * file);
int Find(char *** list, char * file_name, char * directory, int * location);

int main(int argc, char **argv) { 
	DIR * dirptr;
	FILE * fileptr;
	time_t timenow;
	struct tm * timeinfo;
	time (&timenow);
	timeinfo = localtime(&timenow);

	char * header, * request, * path, * filename_requested, * host;
	char * dir, * temp;
	int port, sockfd, connfd;
	char get[3], http[9];
	char filepath[BUF_SIZE];
	char http_not_found[] = "HTTP/1.0 404 Not Found\n";
	char http_ok[] = "HTTP/1.0 200 OK\n";
	char buffer[BUF_SIZE];
	char * contentType;

	if (argc != 3) {
		printf("usage: [host] [portnumber]\n");
		exit(1);
	}

	header = (char*)malloc(BUF_SIZE*sizeof(char));
	request = (char*)malloc(BUF_SIZE*sizeof(char));
	path = (char*)malloc(BUF_SIZE*sizeof(char));
	filename_requested = (char*)malloc(BUF_SIZE*sizeof(char));
	char ** found_list = (char**)malloc(sizeof(char*) * 30);
	if (!found_list) return 0;
	int counter_location = 0;

	char * ini_dir = (char*)malloc(sizeof(char) * 500);
	if (!ini_dir) {
		free (found_list);
		return 0;
	}

	host = argv[1];
	port = atoi(argv[2]);
	sockfd = createSocket(host, port);

	for (;;) {

		memset(ini_dir, 0, sizeof(ini_dir));
		printf("--------------------------------------------------------\n");
		printf("Waiting for a connection...\n");
		connfd = listenForRequest(sockfd);
		//gets the request from the connection
		recv(connfd, request, 100, 0);
		printf("Processing request...\n");
		//parses request
		sscanf(request, "%s %s %s", get, path, http);
		filename_requested = path + 1; //ignores the first slash

		//search the filename_requested
		Find(&found_list, filename_requested, ini_dir, &counter_location);
		if (counter_location <= 0) {
			printf("File not found!\n");
			send(connfd, http_not_found, strlen(http_not_found), 0); //sends HTTP 404
			sprintf(header, "Content-Type: application/text\nContent-Length: 0\n\n");
			send(connfd, header, strlen(header), 0); //sends the header

			close(connfd);
			continue;
		}

		strncpy(filepath, found_list[counter_location-1], strlen(found_list[counter_location-1]));
		filepath[strlen(found_list[counter_location-1])] = '\0';
		printf("\nSERVER filepath: %s ", filepath);

//		contentType = getFileType(filename_requested);
		contentType = "application/octet-stream";
		//sprintf(header, "Date: %sHostname: %s:%d\nLocation: %s\nContent-Type: %s\nContent-Length: %d\n\n", asctime(timeinfo), host, port, filename_requested, contentType, 1053);

		struct stat statBuf = {0};
		if (0 > stat(filepath, &statBuf)) {
			printf("Failed to get stat of file\n");
			return 1;
		}

		int file_length = statBuf.st_size;
		sprintf(header, "Content-Type: application/text\nContent-Length: %d\n\n", file_length);

		if ((fileptr = fopen(filepath, "r")) == NULL ) {
		} else {
			printf("Sending the file...\n");
			send(connfd, http_ok, strlen(http_ok), 0); //sends HTTP 200 OK  
			recv(connfd, buffer, BUF_SIZE, 0);
			printf("Received %s\n", buffer);
			if ((temp = strstr(buffer, "OK")) == NULL) {
				printf("Operation aborted by the user!\n");
				//break;
			}
			send(connfd, header, strlen(header), 0); //sends the header
			
			memset(&buffer, 0, sizeof(buffer));
			while (!feof(fileptr)) { //sends the file
				int readBytes = fread(&buffer, 1, sizeof(buffer), fileptr);
				printf("Read %d bytes\n", readBytes);
				send(connfd, buffer, readBytes, 0);
				memset(&buffer, 0, sizeof(buffer));
			}
			printf("File sent...\n");
		}
		printf("Processing completed...\n");
		close(connfd);
	}

	close(sockfd);
	free(header);
	free(request);
	free(path);
	free(filename_requested);
	if (counter_location > 0) {
		while (--counter_location >= 0) {

			free (found_list[counter_location]);
		}
	}

	return 0;
}


// Function to Create TCP socket end point
int createSocket(char * host, int port) {
	int sockfd;
	struct sockaddr_in addr;
	struct hostent * host_ent;
	char * hostAddr;

	memset(&addr, 0, sizeof(addr));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = INADDR_ANY;
	addr.sin_port = htons((short)port);

	sockfd = socket(AF_INET, SOCK_STREAM, 0);
	if (sockfd < 0) {  
		printf("Error creating socket!\n");  
		exit(1);  
	}  
	printf("Socket created...\n");

	if (bind(sockfd, (struct sockaddr *)&addr, sizeof(addr)) < 0) {
		printf("Error binding socket to port!\n");  
		exit(1);
	}
	printf("Binding done...\n");

	return sockfd;
}


// Function to listen for incoming requests
int listenForRequest(int sockfd) {
	int conn;
	char hostip[32];
	struct sockaddr_in addr;
	struct hostent * host;
	struct in_addr inAddr;
	int len;

	addr.sin_family = AF_INET;

	listen(sockfd, 5); //maximum 5 connections
	len = sizeof(addr); 
	if ((conn = accept(sockfd, (struct sockaddr *)&addr, &len)) < 0) {
		printf("Error accepting connection!\n");
		exit(1);
	}
	printf("Connection accepted...\n");

	inet_ntop(AF_INET, &(addr.sin_addr), hostip, 32);
	inet_pton(AF_INET, hostip, &inAddr);
	host = gethostbyaddr(&inAddr, sizeof(inAddr), AF_INET);

	printf("---Connection received from: %s [IP= %s]---\n", host->h_name, hostip);
	return conn;
}


// Function find type of the file 
// HTTP client is requesting
char * getFileType(char * file) {
	char * temp;
	if ((temp = strstr(file, ".html")) != NULL) {
		return "text/html";
	} else if ((temp = strstr(file, ".pdf")) != NULL) {
		return "application/pdf";
	} else if ((temp = strstr(file, ".txt")) != NULL) {
		return "text/html";
	}
}


// Function to search a file system wide.
int Find(char *** list, char * file_name, char * directory, int * location)
{
	DIR * opened_dir;
	struct dirent * directory_structure = NULL;
	struct dirent entry;

	char * temp_dir = (char*)malloc(sizeof(char) * 300);
	if (!temp_dir) return -1;

	
	strcat(directory, "/"); 
	//printf("Find:directory = %s\n", directory);

	opened_dir = opendir(directory);
	if (opened_dir == NULL) {
		free (temp_dir);
		return -1;
	}

	while (1) {
		directory_structure = readdir(opened_dir);
		if (!directory_structure) {
			closedir(opened_dir);
			free (temp_dir);
			return 0;
		}

		if (!strcmp(directory_structure->d_name, "..") || 
				!strcmp(directory_structure->d_name, ".")) continue; 

		else if (!strcmp(directory_structure->d_name, file_name)) {
			(*list)[*location] = (char*)malloc(sizeof(char) * 300);
			sprintf((*list)[*location], "%s%s", directory, directory_structure->d_name);
			(*location)++;
		}
		else if (directory_structure->d_type == DT_DIR) {
			sprintf(temp_dir, "%s%s", directory, directory_structure->d_name);
			Find(list, file_name, temp_dir, location);
		}
	}

	return 0;
}
