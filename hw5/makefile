all: proxy 

proxy: util dynamicarray
	gcc -c proxy.c -o proxy.o
	gcc util.o dynamicarray.o proxy.o -o proxy -lpthread -lm

dynamicarray: dynamicarray.c
	gcc -c dynamicarray.c -o dynamicarray.o

util: util.c
	gcc -c util.c -o util.o
	
clean: 
	rm *.o proxy 
	# rm ./bin/*
