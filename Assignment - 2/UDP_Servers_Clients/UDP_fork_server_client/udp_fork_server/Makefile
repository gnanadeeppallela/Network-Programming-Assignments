# Make file targets to build server and clients
# Format
# target: dependencies
#   action

.PHONY: server

all: server

server:
	gcc udp_server.c -o server

clean:
	rm server
