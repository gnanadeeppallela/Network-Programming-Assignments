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
   ./server <port> <number of child process>
   ex: ./server 10000 7

   ./client <server_IP> <port>
   ex: ./client 127.0.0.1 10000

6. Enter ^C to quit running the server.


UDP prefork Server: Server program creates N(given as command line argument) child processes in advance (process pool)to handle the incoming client request.When a datagram is received from the client, active child process at the server sends back the response (total number of child processes preforked at server side).

UDP test client: client connects the server and receives number of preforked children at server side and prints it to the screen.

