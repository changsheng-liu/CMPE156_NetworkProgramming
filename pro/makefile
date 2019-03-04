all: util 

server: util
	gcc -c server.c -o server.o
	gcc util.o server.o -o server -lpthread

client: util datatype
	gcc -c client.c -o client.o
	gcc util.o myds.o client.o -o client -lpthread

util: ./src/util.c
	gcc -c ./src/util.c -o ./bin/util.o
	
clean: 
	rm ./bin/*.o /bin/client /bin/server 
