all: proxy 

proxy: util
	gcc -c proxy.c -o proxy.o
	gcc util.o proxy.o -o proxy -lpthread

util: util.c
	gcc -c util.c -o util.o
	
clean: 
	rm *.o proxy 
	# rm ./bin/*
