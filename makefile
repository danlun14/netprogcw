all: client server

client: client.c
	gcc -o client client.c -Wall -pthread

server: server.c
	gcc -o server server.c -Wall -pthread