all: server client 

server: util socket
	gcc -c server.c -o server.o
	gcc util.o mysocket.o server.o -o server

client: util socket
	gcc -c client.c -o client.o
	gcc util.o mysocket.o client.o -o client

util: util.c
	gcc -c util.c -o util.o

socket: mysocket.c
	gcc -c mysocket.c -o mysocket.o

clean: 
	rm *.o client server