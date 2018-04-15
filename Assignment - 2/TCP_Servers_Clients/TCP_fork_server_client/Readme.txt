Steps to execute the program:

1. Open the terminal
2. Navigate to the program directory
3. Build server & client:
    make all 

   Build server alone:
    make server

   Build client alone:
    make client

5. Run server and client on two different terminals:
   ./server <port>
   ex: ./server 10000

   ./client <server_IP> <port> <file_name>
   ex: ./client 127.0.0.1 10000 one_k_file.txt
   ex: ./client 127.0.0.1 10000 two_k_file.txt

6. Enter ^C to quit running the server.


TCP File Server: Server program creates child processes to handle the incoming client
request on demand. Child exits once client is served. server sends specified file to client.
Server can transfer 1. one_k_file.txt (1K size)
		    2. two_k_file.txt (2K size)

TCP test client: Client takes a file name input from the user and sends it to the TCP server.
Server transfer the specifies file back to the client. Client saves it on local disk with name:
test_file.txt

If specified file does not exist on the server then server sends back an error message which
is saved into file "test_file.txt". However file transfer is  printed "successful" on the
client display. User must verify the copied file on the disk.

