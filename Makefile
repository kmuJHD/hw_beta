all : server client
	./server &

server : server.c types.h
	gcc server.c -o server

client : client.c types.h
	gcc client.c -o client

clear :
	rm -fr client server
