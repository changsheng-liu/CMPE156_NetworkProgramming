all: server client 

server: util dynamicarray
	gcc -c ./src/server.c -o ./bin/server.o
	gcc ./bin/util.o ./bin/server.o ./bin/dynamicarray.o -o ./bin/server -lpthread

client: util protocol client_util
	gcc -c ./src/client.c -o ./bin/client.o
	gcc ./bin/util.o ./bin/myprotocol.o ./bin/client_util.o ./bin/client.o -o ./bin/client -lpthread -lreadline

client_util: 
	gcc -c ./src/client_util.c -o ./bin/client_util.o

util:
	gcc -c ./src/util.c -o ./bin/util.o

dynamicarray:
	gcc -c ./src/dynamicarray.c -o ./bin/dynamicarray.o

protocol:
	gcc -c ./src/myprotocol.c -o ./bin/myprotocol.o

clean: 
	rm ./bin/*.o ./bin/client ./bin/server 
