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
   ./server <port>
   ex: ./server 10000

   ./client <server_IP> <port>
   ex: ./client 127.0.0.1 10000

6. Enter ^C to quit running the server.


UDP File Server:Server maintains 2 files "one_k_file.txt" and "two_k_file.txt". Server program creates child processes on demand to handle the incoming client request.When client sends a file transfer request, server validates the file name and transfer the file. If File name sent
by the client does not match with server files then an error message is sent back to the client.

UDP test client:A File name is passed to the client as a command line argument. Client sends a datagram to the server requesting this file
from the server. Client then waits for the file transfer. It opens "test_file.txt" file and writes messages transfered by the server into
this file. Then closes the connection.

