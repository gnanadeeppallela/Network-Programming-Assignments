Steps to execute the program:

1. Open the terminal
2. Navigate to the program directory
3. Build server & client:
    make all 

   Build server alone:
    make server

   Build client alone:
    make client

   Clean all targets:
    make clean

5. Run server and client on two different terminals:
   ./server <port> <number of child processes>
   ex: ./server 10000 5

   ./client <server_IP> <port>
   ex: ./client 127.0.0.1 10000

6. Enter ^C to quit running the server.


TCP ECHO Server: Server program creates N(given as command line argument) child processes in advance (process pool)to handle the incoming client request. Each child waits for the incoming connection by calling accept() system call. One of the children accepts the connection when a new connection is requested from the client and serves the client. child process returns back to the pool once client connection is served and disconnected.

TCP test client: client connects the server and receives number of preforked children at server side and prints it to the screen.

