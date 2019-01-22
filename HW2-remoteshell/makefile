all: server client 

server: util
	gcc -c server.c -o server.o
	gcc util.o server.o -o server

client: util
	gcc -c client.c -o client.o
	gcc util.o client.o -o client

util: util.c
	gcc -c util.c -o util.o

clean: 
	rm *.o client server