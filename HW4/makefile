all: client server 

server: util
	gcc -c server.c -o server.o
	gcc util.o server.o -o server -lpthread

client: util datatype
	gcc -c client.c -o client.o
	gcc util.o myds.o client.o -o client -lpthread

util: util.c
	gcc -c util.c -o util.o
	
datatype: mydatastructure.c
	gcc -c mydatastructure.c -o myds.o

clean: 
	rm *.o client server *.bat
	# rm ./bin/*
