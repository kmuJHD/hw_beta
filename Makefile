all : server client
	./server &

server : server.c types.h
	gcc server.c -o server

client : client.c types.h Client_recvAnswer.o
	gcc client.c -o client

Client_recvAnswer.o: Client_recvAnswer.c

clear :
	rm -fr client server
