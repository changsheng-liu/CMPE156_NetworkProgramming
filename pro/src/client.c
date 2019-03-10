#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <pthread.h>
#include <time.h>
#include "util.h"
#include "myprotocol.h"


void checkParam(int argc, char* argv[]) {
	if(argc != 4) {
        failHandler("Please use client by correct input! usage: ./client <server-ip> <server-port> <client-ip>");
    }
    if(!isValidIP(argv[1])) {
        failHandler("Please input correct server ip address!");
    }
    if(!isNumber(argv[2])) {
        failHandler("Please input correct port number!");
    }
}

int main(int argc, char* argv[]) {
	checkParam(argc, argv);
    int chunk_num = atoi(argv[2]);

    int client_fd;
    if((client_fd = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        failHandler("create socket error!");
    }
	struct timeval tv;
    tv.tv_sec = 2;
    tv.tv_usec = 0;
    setsockopt(client_fd, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv, sizeof tv);
	
	struct sockaddr_in addr;
    memset(&addr, 0, sizeof(addr));
    addr.sin_family = AF_INET;
    if(inet_aton(argv[1], &addr.sin_addr)<=0) { 
        failHandler("connect socket error!");
    } 
    addr.sin_port = htons(atoi(argv[2]));

	if(connect(client_fd, (struct sockaddr*)&addr, sizeof(addr)) < 0){
        failHandler("connect socket error!");
    }
		
    //business logic
    char buf[100];
    write(client_fd, argv[3], 100);
    read(client_fd, buf, 100);
    fputs(buf, stdout);

    close(client_fd);
    return 0;
}

