all: server client

server: util socket executer
	gcc -c server.c -o server.o
	gcc util.o mysocket.o executer.o server.o -o server

executer:
	gcc -c executer.c -o executer.o

client: util socket
	gcc -c client.c -o client.o
	gcc -lreadline util.o mysocket.o client.o -o client

util: util.c
	gcc -c util.c -o util.o

socket: mysocket.c
	gcc -c mysocket.c -o mysocket.o

clean: 
	rm *.o
	rm ./bin/*
