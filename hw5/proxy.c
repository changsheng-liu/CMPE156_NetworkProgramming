#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include "util.h"

void checkParam(int argc, char* argv[]);

int main(int argc, char *argv[])
{
    checkParam(argc, argv);

    int port = atoi(argv[1]);

    int proxy_sock;
    if((proxy_sock = socket(AF_INET, SOCK_STREAM, 0)) < 0) {
        failHandler("create socket error!");
    }
    int optval=1;
    if (setsockopt(proxy_sock, SOL_SOCKET, SO_REUSEADDR, (const void *)&optval , sizeof(int)) < 0) {
        failHandler("create socket error!");
    }
    struct sockaddr_in serveraddr;
    bzero((char *) &serveraddr, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET; 
    serveraddr.sin_addr.s_addr = htonl(INADDR_ANY); 
    serveraddr.sin_port = htons(port); 
    if (bind(proxy_sock, (struct sockaddr*)&serveraddr, sizeof(serveraddr)) < 0) {
        failHandler("create socket error!");
    }

    while(1) {
        if(listen(proxy_sock, 1) < 0){
            failHandler("listen error!");
        }
        int addr_len = sizeof(serveraddr);
        int server_sock = accept(proxy_sock, (struct sockaddr*)&serveraddr, (socklen_t *) &addr_len);
        if(server_sock < 0) {
            continue;
        }
        char buf[1024];
        bzero(buf, 1024);
        write(server_sock, buf, 1024 );
        fputs(buf, stdout);
    }

    return 0;
}

void checkParam(int argc, char* argv[]) {

	if(argc != 3) {
        failHandler("Please use client by correct input! usage: ./proxy <listen-port> <forbidden-sites-file>");
    }

    if(!isNumber(argv[1])) {
        failHandler("Please input correct port number!");
    }

    if(!hasFile(argv[2])) {
        failHandler("forbidden-sites-file does not exist!");
    }
}