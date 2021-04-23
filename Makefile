CC = gcc -Wall -g

all: client server_multithread

client: client.c common.h
	$(CC) -o client client.c

server_multithread: server.c common.h
	$(CC) -o server_multithread server.c -lpthread

.PHONY: clean

clean:
	rm -f client server_multithread
