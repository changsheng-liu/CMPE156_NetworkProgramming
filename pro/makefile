all: server client 

server: util dynamicarray
	gcc -c ./src/server.c -o ./bin/server.o
	gcc ./bin/util.o ./bin/server.o ./bin/dynamicarray.o -o ./bin/server -lpthread

client: util 
	gcc -c ./src/client.c -o ./bin/client.o
	gcc ./bin/util.o ./bin/client.o -o ./bin/client -lpthread

util:
	gcc -c ./src/util.c -o ./bin/util.o

dynamicarray:
	gcc -c ./src/dynamicarray.c -o ./bin/dynamicarray.o

clean: 
	rm ./bin/*.o ./bin/client ./bin/server 
