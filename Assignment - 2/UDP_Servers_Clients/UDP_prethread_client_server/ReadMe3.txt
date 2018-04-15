Steps to execute the program:

1. Open the terminal
2. Navigate to the program directory
3. Build server & client:
    make MakeFile3.txt - for server
    make MakeFile3Client.txt - for client
4. Run server and client on two different terminals:
   ./server <port> <Number of threads>
   ex: ./server 10000 4

   ./client <server_IP> <port>
   ex: ./client 127.0.0.1 10000

5. Enter ^C to quit running the server.
