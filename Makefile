all : server client
	./server &

server : Server.c types.h
	gcc Server.c -o server

client : Client.c types.h Client_recvAnswer.o Client_recvReNew.o
	gcc $^ -o client -g

Client_recvAnswer.o: Client_recvAnswer.c
Client_recvReNew.o: Client_recvReNew.c

clear :
	rm -fr client server
