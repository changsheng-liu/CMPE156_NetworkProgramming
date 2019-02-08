all: server client

server: util socket file
	gcc -c server.c -o server.o
	gcc util.o mysocket.o myfile.o server.o -o server

client: util socket file
	gcc -c client.c -o client.o
	gcc util.o mysocket.o myfile.o client.o -o client

file: myfile.c
	gcc -c myfile.c -o myfile.o

util: util.c
	gcc -c util.c -o util.o

socket: mysocket.c
	gcc -c mysocket.c -o mysocket.o

clean: 
	rm *.o client server
	# rm ./bin/*
