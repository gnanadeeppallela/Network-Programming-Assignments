# Make file targets to build TCP client & TCP server (Assignment 1, problem #3)
# Format
# target: dependencies
#   action

.PHONY: server client

all: server client

server:
	gcc tcp_server_3.c -o server

client:
	gcc tcp_client_3.c -o client

clean:
	rm server client

