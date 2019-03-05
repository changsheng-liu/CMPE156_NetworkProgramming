all: server client 

server: util
	gcc -c ./src/server.c -o ./bin/server.o
	gcc ./bin/util.o ./bin/server.o -o ./bin/server

client: util 
	gcc -c ./src/client.c -o ./bin/client.o
	gcc ./bin/util.o ./bin/client.o -o ./bin/client

util:
	gcc -c ./src/util.c -o ./bin/util.o
	
clean: 
	rm ./bin/*.o ./bin/client ./bin/server 
